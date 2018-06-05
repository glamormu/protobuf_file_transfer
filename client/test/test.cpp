// test/test.c
/*
 * bool secft_startup();//start up sdk
 * void shut_down(); //shut down the sdk
 * int secft_handle_event(const char* event_path, sec_funt_ptr handler, 
 *			void* user_data);
 * 
 */
#include <iostream>
#include "secftclient.h"
#include "plog/Log.h"
#include <variant>

int user_system_callback(void* user_data, sec_variant data){
    std::cout << "In system\n";
    if(holds_alternative<string>(data)){
        std::cerr << get<string>(data);
    }
    return 0;
}
int user_stream_callback(void* user_data, sec_variant data){

    if(holds_alternative<int>(data)){
      std::cout << "uploading: " << get<int>(data) << std::endl;
    }
    return 0;
}


int main(int argc, char **argv)
{
	char addr[] = {"cvzhang.cn\0"};
    char username[] = {"vergil\0"};
    char token[] = {"token\0"};

    char user_data_system[] = {"hello\0"};
    secft_handle_event(SEC_EVENT_SYSTEM, user_system_callback, user_data_system);
    char user_data_stream[] = {"world\0"};
    secft_handle_event(SEC_EVENT_STREAM, user_stream_callback, user_data_stream);

	if(!secft_start_up(addr, 8080, username, token)) {
		LOG_DEBUG << "secft client startup error";
		return 1;
	}
#ifdef DEBUG
	LOG_DEBUG << "Connected the file server!";
#endif
    map<string, sec_variant> test;
    secft_start_stream("test", test);
    secft_start_stream("/home/zhang/Downloads/zImage", test);
	//上传文件测试(单文件)

    //1. 上传全量
    /*
     * path 是文件路径
     * params是文件的相关的配置, 有权限位, 所有者的信息,均为字符串配置,目前采用map来传递信息,以后用sec_variant来传递信息.
     * user_callback是文件上传成功后的回调,用来替换全局回调. 全局回调是用来获得上传信息的. 默认回调只会写入LOG文件.
     */
//    secft_start_stream(path, params, user_callback);
//    secft_start_stream(path, params);
//    //2. 断点上传
//    secft_start_stream(path, params, user_callback);
//    secft_start_stream(path, params);
//    //3. 重新上传
//    secft_start_stream(path, params, user_callback);
//    secft_start_stream(path, params);
//    //4. 取消上传
//    secft_stop_stream(path, params, user_callback);
//    secft_stop_stream(path, params);
//
//    //下载文件测试(单文件)
//    //1. 下载全量
//    secft_start_stream(path, params,user_callback);
//    secft_start_stream(path, params);
//    //2. 断点下载
//    secft_start_stream(path, params,user_callback);
//    secft_start_stream(path, params);
//    //3. 取消下载
//    secft_stop_stream(path, params);
//    secft_stop_stream(path, params, user_callback);

    //todo 文件夹的上传下载
    while(1){
    }
	return 0;
}
