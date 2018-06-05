#ifndef _so1_h_
#define _so1_h_
#include "config.h"

#define SEC_EVENT_SYSTEM                     "event.system"
#define SEC_EVENT_STREAM                     "event.stream"

#define SEC_PROP_LOGPATH                     "prop.log_path"
#define SEC_PROP_DOWNPATH                    "prop.down_path"

#define START_UPLOAD 0
#define START_DOWNLOAD 1
//#include "sec_variant.hpp"
#include <variant>
#include <string>
#include <map>

using namespace std;
typedef variant<bool,uint32_t, int, double, string, void*> sec_variant;

typedef int (*secft_func_ptr)(void* user_data, sec_variant data);

bool secft_start_up(const char* addr, int port,const char* user_name,const char* token);
void secft_shut_down();
const char* secft_version();
int secft_handle_event(const char* event, secft_func_ptr handler, void* user_data);
int sceft_set_property(const char* prop, const char* value);
const char* secft_get_property(const char* prop);
const char *secft_err_msg(void);
int secft_start_stream(const char* path, map<std::__cxx11::string, sec_variant> params);
void secft_stop_stream(const char* path, sec_variant params);


#endif//_so1_h_
