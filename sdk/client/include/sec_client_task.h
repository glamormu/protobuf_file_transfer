#ifndef _sec_client_task_h
#define _sec_client_task_h
//https://github.com/goodfella/libtq
#include <vector>
#include <map>
#include "secftclient.h"
#include <openssl/md5.h>
#include <thread>
#include <mutex>
#include "secftclient_internal.h"
using std::vector;
using std::map;
using std::thread;

extern int stream_id;
struct stream_item
{
  int stream_id;
  string path;
  string remote_path;
  unsigned char md5[MD5_DIGEST_LENGTH];
  ssize_t offset;
  ssize_t size;
//  uint8_t stream_type;
  uint8_t stream_status;
  vector < stream_item > children;
  map < string, sec_variant > config;
};

struct secft_cell
{
    stream_item item;
    void upload_file();
    void download_file();
    void stop();
private:
    void _upload();
    void _download();
    void create_download_file();
    Request get_packet();
    int check_remote_file(sec_file_client& client);
    void notify_download_failed(string msg);
    bool request_packet();
    //void pause();
};

class sec_scheduler
{
protected:
    sec_scheduler():max_task_num(4), task_count(0)
    {
        //schelduler thread
        std::thread scheduler_thread(&sec_scheduler::scheduler, this);
        scheduler_thread.detach();
    }
private:
    static sec_scheduler* p;
    map<int, secft_cell> sec_task_map;//map is a RB tree
    int max_task_num;
    int task_count;
    std::mutex task_count_mutex;
    void scheduler();
    void increase_task_count();
    void decrease_task_count();
public:
    static sec_scheduler* initance();
    int add_task(stream_item item);
    void stop_task(int stream_id);
    void set_task_num(int num);
};


#endif
