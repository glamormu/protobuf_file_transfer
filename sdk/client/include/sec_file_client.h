#ifndef _sec_file_client_H
#define _sec_file_client_H
#include "config.h"
#include "file_transfer.pb.h"
#include "sec_asio_common.h"

const int max_msg_ = 4096*2;
using namespace secft::proto::file_transfer_packet;
struct sec_file_client{
    typedef boost::system::error_code error_code;
    sec_file_client();
    bool connect(ip::tcp::endpoint ep) {
        error_code err;
        sock_.connect(ep, err);
        if(err) {
            return false;
        }
        return true;
    }
    void stop();
    bool send_request(Request request);
    int read_reply(Reply & reply);
private:
    ip::tcp::socket sock_;
    ip::tcp::endpoint epi_;
    char buffer_[max_msg_];
    int  alread_read_;
    bool started_;
};
#endif
