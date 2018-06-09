// test/test.c

#include <iostream>
#include "secftclient.h"
#include "plog/Log.h"


int user_system_callback(void* user_data, std::map<string,sec_variant> data){
    string msg_str;
    std::map<string,sec_variant>::iterator iter = data.find("system.callback.msg");
    if(iter != data.end()){
        if(holds_alternative<string>(iter->second)){
            msg_str = get<string>(iter->second);
        }
    }
    if(msg_str.size() != 0){
        std::cout << msg_str <<std::endl;
    }
    return 0;
}
//stream added or removed
/*
 * data:
 *  class MSG
 *  class ADD
 *  class REMOVE
 *  class SUCESS
 *  class FAILED
 */
int user_stream_callback(void* user_data, map<string,sec_variant> data){
    std::map<string,sec_variant>::iterator iter = data.find("class");
    if(iter == data.end()) {
        return 0;
    }
    string msg_str;
    iter = data.find("stream.callback.msg");
    if(iter != data.end()){
        if(holds_alternative<string>(iter->second)){
            msg_str = get<string>(iter->second);
        }
    }
    if(msg_str.size() != 0){
        std::cout << msg_str <<std::endl;
    }
    return 0;
}
//stream process
/*
 * data:
 *  class MSG
 *  class UPLOAD_PROCESS
 *  class DOWNLOAD_PROCESS
 *  class PAUSED
 *  class CANLSED
 *  class RESUME
 */
int user_process_callback(void* user_data, map<string,sec_variant> data){
    std::map<string,sec_variant>::iterator iter = data.find("class");
    if(iter == data.end()) {
        return 0;
    }
    int clz = get<int>(data["class"]);
    int stream_id;
    double process;
    string path;
    switch (clz) {
    case sec_process_upload:
        stream_id = get<int>(data["stream_id"]);
        process = get<double>(data["process"]);
        path = get<string>(data["path"]);
        std::cout  << "path:" << path << std::endl
                  << "stream id :" <<stream_id << std::endl
                  << "process:" << process << std::endl;
        break;
    case sec_process_uploaded:
        stream_id = get<int>(data["stream_id"]);
        path = get<string>(data["path"]);
        std::cout  << "path:" << path << std::endl
                  << "stream id :" <<stream_id << std::endl
                  << "is uploaded!"<< std::endl;
        break;

    default:
        std::cerr << "in defaulet\n";
        break;
    };
    return 0;
}

int main(int argc, char **argv)
{
	string addr = "127.0.0.1";
    string username = "test";
    string token = "token";
    int port = 10086;

    //register callbacks
    //1. System callback. Notify system events
    secft_handle_event(SEC_EVENT_SYSTEM, user_system_callback, nullptr);
    //2. Stream callbacks. Notify stream events
    secft_handle_event(SEC_EVENT_STREAM, user_stream_callback, nullptr);
    //3. Process callbacks. Notify process events
    secft_handle_event(SEC_EVENT_PROCESS, user_process_callback, nullptr);

	if(!secft_start_up(addr.c_str(), port,
                username.c_str(), token.c_str())) {
        //TODO: get error message
        std::cerr << "client startup error";
		return 1;
	}
#ifdef DEBUG
    std::cerr << "client startup ok!\n";
#endif

    //Test cases: Start upload without parameters
    map<string, sec_variant> test;
    test[SEC_STREAM_TYPE] = START_UPLOAD;
    //Test case one: No such file
    int id = secft_start_stream("test", test);
    if(id == -1) {
        std::cerr << "add task failed\n";
    }
    //Test cases: normal file
    id = secft_start_stream("/home/zhang/Downloads/zImage", test);
    if(id == -1) {
        std::cerr << "add task failed\n";
    }
    usleep(10000);
    secft_cancel_stream(id, test);

    id = secft_start_stream("/home/zhang/Downloads/u-boot.bin", test);
    if(id == -1) {
        std::cerr << "add task failed\n";
    }

    id = secft_start_stream("/home/zhang/Downloads/vfb2-master.zip", test);
    if(id == -1) {
        std::cerr << "add task failed\n";
    }
    //Test case: cancel upload without parameters


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
    sleep(5);
	return 0;
}
