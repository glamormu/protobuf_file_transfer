#include "sec_client_task.h"
#include <filesystem>
int stream_id = 0;

void sec_scheduler::scheduler()
{
    while(1) {
        if(task_count < max_task_num) {
            task_mutex.lock();
            for(auto iter = sec_task_map.begin(); iter != sec_task_map.end(); ++iter) {
                switch (iter->second.item.stream_status) {
                case START_UPLOAD:
                    iter->second.item.stream_status = UPLOADING;
                    iter->second.upload_file();
                    increase_task_count();
                    break;
                case START_DOWNLOAD:
                    iter->second.item.stream_status = DOWNLOADING;
                    iter->second.download_file();
                    increase_task_count();
                    break;
                default:
                    break;
                }
            }
            task_mutex.unlock();
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
    sec_file_client client;
    if(!client.connect(ep)) {
        msg = "connect error";
        system_callback_data["system.callback.msg"] = msg;
        if(callback_map->at(SEC_EVENT_SYSTEM).callback){
            callback_map->at(SEC_EVENT_SYSTEM).callback(callback_map->at(SEC_EVENT_SYSTEM).user_data, system_callback_data);
        }
        return;
    }
#ifdef DEBUG
    LOG_DEBUG << "stream id:"<< this->item.stream_id << this->item.path << " START UPLOAD";
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

        if(!client.send_request(request)) {
            LOG_ERROR << "send_request failed\n";
            this->item.stream_status = UPLOAD_FAILED;
            process_callback_data["class"] = sec_process_failed;
            process_callback_data["path"] = this->item.path;
            process_callback_data["stream_id"] = this->item.stream_id;
            if(callback_map->at(SEC_EVENT_PROCESS).callback) {
                callback_map->at(SEC_EVENT_PROCESS).callback(callback_map->at(SEC_EVENT_PROCESS).user_data, process_callback_data);
            }
            break;
        }

        Reply reply;
        if(client.read_reply(reply) == 0) {
            int reply_status = reply.status();
            LOG_DEBUG << "reply_status:" << reply_status;
            if(reply_status == Status::STATUS_SUCCESS){
                process_callback_data["class"] = sec_process_upload;
                process_callback_data["process"] = (double)((double)this->item.offset / (double)this->item.size);
                process_callback_data["path"] = this->item.path;
                process_callback_data["stream_id"] = this->item.stream_id;
                if(callback_map->at(SEC_EVENT_PROCESS).callback) {
                    callback_map->at(SEC_EVENT_PROCESS).callback(callback_map->at(SEC_EVENT_PROCESS).user_data, process_callback_data);
                }
                continue;
            }
        }
        else {
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
    sec_variant msg = "start download " + this->item.remote_path;
    LOG_DEBUG << get<string>(msg);
    map<string, sec_variant> system_callback_data;
    system_callback_data["system.callback.msg"] = msg;
    if(callback_map->at(SEC_EVENT_SYSTEM).callback){
        callback_map->at(SEC_EVENT_SYSTEM).callback(callback_map->at(SEC_EVENT_SYSTEM).user_data, system_callback_data);
    }
    //0. init connection
    ip::tcp::endpoint ep(ip::address::from_string(_server_addr), _server_port);
    sec_file_client client;
    if(!client.connect(ep)) {
        msg = "connect error";
        system_callback_data["system.callback.msg"] = msg;
        if(callback_map->at(SEC_EVENT_SYSTEM).callback){
            callback_map->at(SEC_EVENT_SYSTEM).callback(callback_map->at(SEC_EVENT_SYSTEM).user_data, system_callback_data);
        }
        return;
    }
#ifdef DEBUG
    LOG_DEBUG << "stream id:"<< this->item.stream_id << this->item.path << " CONNECT OK";
#endif
    //1. send request
    if(check_remote_file(client) == -1) {
        string msg_str = "Remote file error";
        this->item.stream_status = DOWNLOAD_FAILED;
        notify_download_failed(msg_str);
        return;
    }
    //2. create file
    create_download_file();
    //3. recv data
    bool continue_download = true;
    while(item.offset < item.size && continue_download) {
        switch (item.stream_status) {
        case DOWNLOAD_CANCELED:
            continue_download = false;
            break;
        case DOWNLOAD_FAILED:
            continue_download = false;
            notify_download_failed("DOWNLOAF FAILED");
        case DOWNLOAD_PAUSED:
            continue_download = false;
            break;
        default:
            break;
        }
        continue_download = request_packet(client);
    }
}
bool secft_cell::request_packet(sec_file_client& client) {
    //send request
    char buff[max_msg_] = {0};
    Request request;
    request.mutable_packet()->set_user_name(_user_name);
    request.mutable_packet()->set_token(_token);
    request.mutable_packet()->set_data(buff);
    request.mutable_packet()->set_file_size(item.size);
    request.mutable_packet()->set_flags(Packet::Flags::Packet_Flags_FLAG_PACKET);
    request.mutable_download_request()->set_path(this->item.remote_path);
    request.mutable_upload_request()->set_path("");
    request.mutable_upload_request()->set_overwrite(false);
    request.mutable_packet_request()->set_dummy(1);
    request.mutable_packet_request()->set_offset(item.offset);
    client.send_request(request);
    //read packet
    Reply reply;
    client.read_reply(reply);
    int packet_flag = reply.packet().flags();
    std::ofstream outfile;
    switch (packet_flag) {
    case Packet::Flags::Packet_Flags_FLAG_FIRST_PACKET:
        outfile.open(item.path, std::ios::out|std::ios::binary);
        break;
    case Packet::Flags::Packet_Flags_FLAG_LAST_PACKET:
    case Packet::Flags::Packet_Flags_FLAG_PACKET:
        outfile.open(item.path, std::ios::out|std::ios::binary|std::ios::app);
        break;
    default:
        return false;
    }

    if(!outfile.is_open()){
        LOG_ERROR << item.path <<"open failed";
        return false;
    }
    outfile.write(reply.packet().data().c_str(), reply.packet().data().size());
    outfile.close();
    item.offset += reply.packet().data().size();

    //write file
    return true;
}
void secft_cell::notify_download_failed(string msg) {
    map<string, sec_variant> process_callback_data;
    process_callback_data["class"] = sec_process_download;
    process_callback_data["path"] = this->item.path;
    process_callback_data["stream_id"] = this->item.stream_id;
    process_callback_data["msg"] = msg;
    if(callback_map->at(SEC_EVENT_PROCESS).callback) {
        callback_map->at(SEC_EVENT_PROCESS).callback(callback_map->at(SEC_EVENT_PROCESS).user_data, process_callback_data);
    }
}
void secft_cell::create_download_file() {
    string root_path = "./";
    string file_path = root_path + this->item.path;
    std::ofstream outfile;
    if(std::filesystem::exists(file_path)){
        //TODO notice user and rename it
        LOG_ERROR << "File exests!";
        this->item.stream_status = DOWNLOAD_FAILED;
        return;
    }
    outfile.open(file_path, std::ios::out|std::ios::binary);
    if(!outfile.is_open()){
        LOG_ERROR << file_path <<"open failed";
        this->item.stream_status = DOWNLOAD_FAILED;
        return;
    }
    item.path = file_path;
    outfile.close();
}
int secft_cell::check_remote_file(sec_file_client & client){
    char buff[max_msg_] = {0};
    Request request;
    request.mutable_packet()->set_user_name(_user_name);
    request.mutable_packet()->set_token(_token);
    request.mutable_packet()->set_data(buff);
    request.mutable_packet()->set_file_size(0);
    request.mutable_packet()->set_flags(Packet::Flags::Packet_Flags_FLAG_FIRST_PACKET);
    request.mutable_download_request()->set_path(this->item.remote_path);
    request.mutable_upload_request()->set_path("");
    request.mutable_upload_request()->set_overwrite(false);
    request.mutable_packet_request()->set_dummy(1);
    client.send_request(request);
    Reply reply;
    if(client.read_reply(reply) == 0) {
        if(reply.status() == Status::STATUS_PATH_ALREADY_EXISTS){

            LOG_DEBUG << "reply.file_list().item_size():"
                      << reply.file_list().item_size();
            for(int i = 0; i < reply.file_list().item_size(); ++i) {
                FileList::Item item =  reply.file_list().item(i);
                this->item.size = item.size();
                this->item.offset = 0;
                this->item.path = item.name();
                //TODO MD5 needed
            }
            return 0;
        }
        else if(reply.status() == Status::STATUS_PATH_NOT_FOUND) {
            this->item.stream_status = DOWNLOAD_FAILED;
            return -1;
        }
    }

    return -1;

}
Request secft_cell::get_packet()
{
    Request request;
    ifstream infile;
    request.mutable_packet()->set_user_name(_user_name);
    request.mutable_packet()->set_token(_token);

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
    ssize_t data_size = BUFFER_SIZE;
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
    infile.close();
    return request;
}

sec_scheduler* sec_scheduler::p = new sec_scheduler;
sec_scheduler *sec_scheduler::initance()
{
    return p;
}
