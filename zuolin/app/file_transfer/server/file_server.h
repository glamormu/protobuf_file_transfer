#ifndef _FILE_SERVER_H
#define _FILE_SERVER_H

#include <sys/types.h>
int init_server();
/**
* int upload_file(char* username, size_t username_len, 
*                    char* token, size_t token_len, 
*                    char* filename, size_t filename_len);
* 启动文件服务器,传入相关信息作为以后的验证
*/
int upload_file(char* username, size_t username_len, 
                    char* token, size_t token_len, 
                    char* filename, size_t filename_len);
int download_file(char* username, size_t username_len, 
                    char* token, size_t token_len, 
                    char* filename, size_t filename_len);
/**
 * void regist_callback();
 * 注册进度回调函数
 */
void regist_callback(void (*handle)(int progress));

#endif