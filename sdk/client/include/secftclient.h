#ifndef _so1_h_
#define _so1_h_
#include "config.h"

#define SEC_EVENT_SYSTEM                     "event.system"
#define SEC_EVENT_STREAM                     "event.stream"
#define SEC_EVENT_PROCESS                     "event.process"

#define SEC_PROP_LOGPATH                     "prop.log_path"
#define SEC_PROP_DOWNPATH                    "prop.down_path"

#define SEC_STREAM_TYPE                      "stream.type"


//#include "sec_variant.hpp"
#include <variant>
#include <string>
#include <map>
typedef enum sec_stream_status_e {
    START_UPLOAD = 0,
    START_DOWNLOAD,
    UPLOADED,
    DOWNLOADED,
    UPLOAD_CANCELED,
    DOWNLOAD_CANCELED,
    UPLOAD_PAUSED,
    DOWNLOAD_PAUSED,
    UPLOADING,
    DOWNLOADING,
    UPLOAD_FAILED,
    DOWNLOAD_FAILED,
}sec_stream_status;

typedef enum sec_process_event_e {
    sec_process_upload = 1,
    sec_process_download,
    sec_process_paused,
    sec_process_canceled,
    sec_process_resume,
    sec_process_failed,
    sec_process_uploaded,
    sec_process_downloaded,
} sec_process_event;

using namespace std;
typedef variant<bool,uint32_t, int, double, string, void*> sec_variant;

typedef int (*secft_func_ptr)(void* user_data, map<string,sec_variant> data);

bool secft_start_up(const char* addr, int port,const char* user_name,const char* token);
void secft_shut_down();
const char* secft_version();
int secft_handle_event(const char* event, secft_func_ptr handler, void* user_data);
int sceft_set_property(const char* prop, const char* value);
const char* secft_get_property(const char* prop);
const char *secft_err_msg(void);
int secft_start_stream(const char* path, map<std::__cxx11::string, sec_variant> params);
void secft_cancel_stream(int stream_id, map<string, sec_variant> params);


#endif//_so1_h_
