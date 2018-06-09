#include <stdio.h>
#include "secftclient.h"
#include "secftclient_internal.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include "plog/Log.h"
#include <string>
#include <map>
#include <openssl/md5.h>
#include <vector>
#include <mutex>
#include <sys/mman.h>
#include "sec_client_task.h"

using namespace std;
using namespace secft::proto::file_transfer_packet;


map<string, sec_callback>* callback_map;
map<string, string>* property_map;
map<int, stream_item> stream_map;

string errmsg = "";
//vector<stream_item> task_queue;

std::mutex task_mutex;

string _server_addr;
int _server_port;
string _user_name;
string _token;

static void __attribute__((constructor)) sec_client_init()
{
    callback_map = new map<string, sec_callback>();
    property_map = new map<string, string>();
    sec_callback* system_callback = new sec_callback();
    sec_callback* stream_callback = new sec_callback();
    callback_map->insert(pair<string, sec_callback>(SEC_EVENT_SYSTEM, *system_callback));
    callback_map->insert(pair<string, sec_callback>(SEC_EVENT_STREAM, *stream_callback));
    property_map->insert(pair<string, string>(SEC_PROP_LOGPATH,"sec_client.log"));
}
void  __attribute__((destructor)) sec_client_fini()
{

}

bool secft_start_up(const char* addr, int port,const char* user_name,const char* token){
    if(!(addr&&user_name&&token)) {
        return false;
    }
    if(port <= 0) {
        return false;
    }
    _server_addr = addr;
    _server_port = port;
    _user_name = user_name;
    _token = token;
    //init log
    plog::init(plog::debug, property_map->at(SEC_PROP_LOGPATH).c_str());
    LOG_DEBUG << "secft client start up";
    //todo connect to server
    //https://www.zhihu.com/question/27908489
    //start thread pool
    return true;

}
void secft_shut_down()
{
    LOG_DEBUG << "secft client start up";
}
const char* secft_version(){
    return "V1.0";
}
int secft_handle_event(const char* event, secft_func_ptr handler, void* user_data){

    if(!strcmp(SEC_EVENT_SYSTEM, event)){
        callback_map->at(SEC_EVENT_SYSTEM).callback = handler;
        callback_map->at(SEC_EVENT_SYSTEM).user_data = user_data;
    }
    else if(!strcmp(SEC_EVENT_STREAM, event)) {
        callback_map->at(SEC_EVENT_STREAM).callback = handler;
        callback_map->at(SEC_EVENT_STREAM).user_data = user_data;
    }
    else if(!strcmp(SEC_EVENT_PROCESS, event)) {
        sec_callback process_callback;
        process_callback.callback = handler;
        process_callback.user_data = user_data;
        callback_map->insert(pair<string,sec_callback>(SEC_EVENT_PROCESS, process_callback));
    }
    return 0;
}
int sceft_set_property(const char* prop, const char* value)
{
    if(!prop || !value)
        return -1;
    if(!strcmp(prop, SEC_PROP_LOGPATH)){
        property_map->at(SEC_PROP_LOGPATH) = value;
    }
    return 0;
}
const char* secft_get_property(const char* prop){
    if(!strcmp(prop, SEC_PROP_LOGPATH))
        return property_map->at(SEC_PROP_LOGPATH).c_str();
    return "";
}
const char *secft_err_msg(void)
{
    return errmsg.c_str();
}

//todo return task id
int secft_start_stream(const char *path, map<string, sec_variant> params)
{
    //generate stream item
    int file_descript = open(path, O_RDONLY);
    if(file_descript < 0) {
        std::cerr << "no such file\n";
        return -1;
    }
    struct stat statbuf;
    if(fstat(file_descript, &statbuf) < 0)
    {
        std::cerr << "no file";
        close(file_descript);
        return -1;
    }
    //check params
    auto it_find = params.find(SEC_STREAM_TYPE);
    if(it_find != params.end()) {
        LOG_DEBUG << "get " << SEC_STREAM_TYPE;
    }
    else {
        std::cerr << "no stream type";
        return -1;
    }
    stream_item item;
    item.path = path;
    if(S_ISDIR(statbuf.st_mode)) {
        LOG_DEBUG << path << " is a Dir";
        //todo upload or download dir
        return -1;
    }
    else {
        size_t file_size =  statbuf.st_size;
        char* file_buffer = (char*)mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0);
        unsigned char result[MD5_DIGEST_LENGTH];
        MD5((unsigned char*) file_buffer, file_size, result);
        munmap(file_buffer, file_size);
        item.size = file_size;
        memcpy(item.md5, result, MD5_DIGEST_LENGTH);
        item.offset = 0;
        item.stream_status = std::get<int>(params[SEC_STREAM_TYPE]);
    }
    map<string, sec_variant> config;
    item.config = config;
    close(file_descript);
    return sec_scheduler::initance()->add_task(item);
}

void secft_cancel_stream(int stream_id, map<string,sec_variant> params)
{
    //1. stop the loop
    sec_scheduler::initance()->stop_task(stream_id);
    //2. send cancel request
}
