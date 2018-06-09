#include "sec_client_task.h"

int stream_id = 0;

void sec_scheduler::scheduler()
{
    while(1) {
        if(task_count < max_task_num) {
            for(auto iter = sec_task_map.begin(); iter != sec_task_map.end(); ++iter) {
                switch (iter->second.item.stream_status) {
                case START_UPLOAD:
                    iter->second.upload_file();
                    increase_task_count();
                    break;
                case START_DOWNLOAD:
                    iter->second.download_file();
                    increase_task_count();
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void sec_scheduler::increase_task_count()
{
    task_count_mutex.lock();
    task_count ++;
    task_count_mutex.unlock();
}

void sec_scheduler::decrease_task_count()
{
    task_count_mutex.lock();
    task_count --;
    task_count_mutex.unlock();
}

int sec_scheduler::add_task(stream_item item)
{
    int res = -1;
    /*
    for(map<int, secft_cell>::iterator iter = sec_task_map.begin();
        iter != sec_task_map.end(); ++iter) {
        int i = 0;
        for(;i < MD5_DIGEST_LEGTH; ++i) {
            if(iter->second.item.md5[i] != item.md5[i]) {
                break;
            }
        }
        if(i == MD5_DIGEST_LENGTH) {
            return -2; //
        }
    }
    */
    item.stream_status = START_UPLOAD;
    task_mutex.lock();
    do {
        stream_id ++;
    } while(sec_task_map.find(stream_id) != sec_task_map.end());
    item.stream_id = stream_id;
    secft_cell cell;
    cell.item = item;
    sec_task_map[stream_id] = cell;
    res = stream_id;
    task_mutex.unlock();
#ifdef DEBUG
    LOG_DEBUG << "add task and stream id is " << stream_id;
#endif
    return res;
}

void sec_scheduler::stop_task(int stream_id)
{
    if(sec_task_map.find(stream_id) == sec_task_map.end()) {
        return;
    }
    sec_task_map[stream_id].stop();
}

void sec_scheduler::set_task_num(int num)
{
    if(num <= 0) {
        return;
    }
    max_task_num = num;
    //TODO change running tasks
}

void secft_cell::upload_file()
{
    thread upload_thread(&secft_cell::_upload, this);
    upload_thread.detach();
}

void secft_cell::download_file()
{
    thread upload_thread(&secft_cell::_download, this);
    upload_thread.detach();
}

void secft_cell::stop()
{
    this->item.stream_status = UPLOAD_CANCELED;
    std::cout << this->item.path << ":canceled\n";
}

void secft_cell::_upload()
{
    sec_variant msg = "start upload " + this->item.path;
    map<string, sec_variant> system_callback_data;
    system_callback_data["system.callback.msg"] = msg;
    if(callback_map->at(SEC_EVENT_SYSTEM).callback){
        callback_map->at(SEC_EVENT_SYSTEM).callback(callback_map->at(SEC_EVENT_SYSTEM).user_data, system_callback_data);
    }
    //read file and generate packet
    //new connect
    ip::tcp::endpoint ep(ip::address::from_string(_server_addr), _server_port);
    boost::shared_ptr<sec_file_client> client_ptr = sec_file_client::start(ep);
    if(!client_ptr) {
        msg = "connect error";
        system_callback_data["system.callback.msg"] = msg;
        if(callback_map->at(SEC_EVENT_SYSTEM).callback){
            callback_map->at(SEC_EVENT_SYSTEM).callback(callback_map->at(SEC_EVENT_SYSTEM).user_data, system_callback_data);
        }
        return;
    }
#ifdef DEBUG
        LOG_DEBUG << this->item.path << " START UPLOAD";
#endif
    while (this->item.offset < this->item.size) {
        if(this->item.stream_status == UPLOAD_CANCELED ||
            this->item.stream_status == UPLOAD_PAUSED ||
            this->item.stream_status == UPLOADED){
#ifdef DEBUG
            if(this->item.stream_status == UPLOAD_CANCELED){
            LOG_DEBUG << this->item.path << " Exit loop:"
                      << "stream_status == UPLOAD_CANCELED";
            }
#endif
            break;
        }
        else {
            this->item.stream_status = UPLOADING;
        }

        map<string, sec_variant> process_callback_data;
        Request request =  get_packet();
        if(!client_ptr->send_request(request)) {
            std::cerr << "send failed\n";
            this->item.stream_status = UPLOAD_FAILED;
            process_callback_data["class"] = sec_process_failed;
            process_callback_data["path"] = this->item.path;
            process_callback_data["stream_id"] = this->item.stream_id;
            if(callback_map->at(SEC_EVENT_PROCESS).callback) {
                callback_map->at(SEC_EVENT_PROCESS).callback(callback_map->at(SEC_EVENT_PROCESS).user_data, process_callback_data);
            }
            break;
        }

        process_callback_data["class"] = sec_process_upload;
        process_callback_data["process"] = (double)((double)this->item.offset / (double)this->item.size);
        process_callback_data["path"] = this->item.path;
        process_callback_data["stream_id"] = this->item.stream_id;
        if(callback_map->at(SEC_EVENT_PROCESS).callback) {
            callback_map->at(SEC_EVENT_PROCESS).callback(callback_map->at(SEC_EVENT_PROCESS).user_data, process_callback_data);
        }
    }
    if(this->item.offset == this->item.size){
        this->item.stream_status = UPLOADED;
#ifdef DEBUG
        LOG_DEBUG << this->item.path << " UPLOAD_SUCESS";
#endif
        map<string, sec_variant> process_callback_data;
        process_callback_data["class"] = sec_process_uploaded;
        process_callback_data["path"] = this->item.path;
        process_callback_data["stream_id"] = this->item.stream_id;
        if(callback_map->at(SEC_EVENT_PROCESS).callback) {
            callback_map->at(SEC_EVENT_PROCESS).callback(callback_map->at(SEC_EVENT_PROCESS).user_data, process_callback_data);
        }
    }
}

void secft_cell::_download()
{

}
Request secft_cell::get_packet()
{
    Request request;
    ifstream infile;
    infile.open(this->item.path, ios::in|ios::binary);
    if(!infile.is_open()){
        std::cerr << "open failed";
        return request;
    }

    char memblock[BUFFER_SIZE] = {0};
    //todo
    request.mutable_upload_request()->set_path(this->item.path);
    request.mutable_upload_request()->set_overwrite(true);
    infile.seekg(this->item.offset,ios_base::beg);
    infile.read (memblock, BUFFER_SIZE);
    size_t data_size = BUFFER_SIZE;
    if (!infile){
        data_size = infile.gcount();
    }
    this->item.offset += data_size;
    if(this->item.offset == this->item.size) {
        std::cout << "send over\n";
        request.mutable_packet()->set_flags(Packet::Flags::Packet_Flags_FLAG_LAST_PACKET);
    }
    else if(this->item.offset == data_size){
        request.mutable_packet()->set_flags(Packet::Flags::Packet_Flags_FLAG_FIRST_PACKET);
    }
    else if(this->item.offset < this->item.size) {
        request.mutable_packet()->set_flags(Packet::Flags::Packet_Flags_FLAG_PACKET);
    }
    request.mutable_packet()->set_file_size(this->item.size);
    request.mutable_packet()->set_data(memblock, data_size);

    request.mutable_download_request()->set_path("");
    request.mutable_packet_request()->set_dummy(1);
    return request;
}

sec_scheduler* sec_scheduler::p = new sec_scheduler;
sec_scheduler *sec_scheduler::initance()
{
    return p;
}
