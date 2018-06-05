#include <stdio.h>
#include "file_transfer.pb.h"
#include "secftclient.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include "plog/Log.h"
#include <string>
#include <map>
#include <fstream>
#include <openssl/md5.h>

#include <sys/mman.h>
using namespace std;



struct sec_callback{
public:
    sec_callback():user_data(nullptr),callback(nullptr) {
    }
    void* user_data;
    secft_func_ptr callback;
};

struct stream_item{
    string path;
    string md5;
    size_t offset;
    size_t size;
    uint8_t stream_type;
    vector<stream_item> children;
};

map<string, sec_callback>* callback_map;
map<string, string>* property_map;
string errmsg = "";

void upload_file(stream_item item, map<string, sec_variant> params){
    //check stream
    //open file
    //generate packet
    //send packet
}

void download_file(stream_item item, map<string, sec_variant> params){

}
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
static void client_thread(){
    sec_variant msg = "In system\n";
    if(callback_map->at(SEC_EVENT_SYSTEM).callback){
        callback_map->at(SEC_EVENT_SYSTEM).callback(callback_map->at(SEC_EVENT_SYSTEM).user_data, msg);
    }

    sec_variant process_v = 0;
    int process = 0;
    while(1){
        sleep(1);
        process += 10;
        if(process == 100){
            break;
        }
        process_v = process;
        if(callback_map->at(SEC_EVENT_STREAM).callback) {
            callback_map->at(SEC_EVENT_STREAM).callback(callback_map->at(SEC_EVENT_STREAM).user_data, process_v);
        }
    }

}

bool secft_start_up(const char* addr, int port,const char* user_name,const char* token){
    //init log
    plog::init(plog::debug, property_map->at(SEC_PROP_LOGPATH).c_str());
    LOG_DEBUG << "secft client start up";
    //todo connect to server

    //start thread pool
    std::thread client(client_thread);
    client.detach();
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


int secft_start_stream(const char *path, map<string, sec_variant> params)
{
    //generate stream item

    int file_descript = open(path, O_RDONLY);
    if(file_descript < 0) {
        std::cerr << "no file";
        return -1;
    }
    struct stat statbuf;
    if(fstat(file_descript, &statbuf) < 0)
    {
        std::cerr << "no file";
        close(file_descript);
        return -1;
    }
    size_t file_size =  statbuf.st_size;
    char* file_buffer = (char*)mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0);
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5((unsigned char*) file_buffer, file_size, result);
    munmap(file_buffer, file_size);

    stream_item item;
    item.path = path;
    item.size = file_size;
    item.md5 = result;
    item.offset = 0;
    item.stream_type = START_UPLOAD;

    std::cout << item.path << ": " << item.md5 << std::endl;

    close(file_descript);
    return 0;
}

void secft_stop_stream(const char *path, sec_variant params)
{

}
