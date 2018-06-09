#ifndef _secftclient_internal_h
#define _secftclient_internal_h
#include <fstream>
#include "sec_asio_common.h"
#include "sec_file_client.h"
#include "file_transfer.pb.h"
struct sec_callback{
public:
    sec_callback():user_data(nullptr),callback(nullptr) {
    }
    void* user_data;
    secft_func_ptr callback;
};

extern map<string, sec_callback>* callback_map;
extern map<string, string>* property_map;


extern string errmsg;
//vector<stream_item> task_queue;

extern std::mutex task_mutex;

extern string _server_addr;
extern int _server_port;
extern string _user_name;
extern string _token;

#endif
