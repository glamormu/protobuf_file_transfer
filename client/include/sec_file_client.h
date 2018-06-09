#ifndef _sec_file_client_H
#define _sec_file_client_H
#include "config.h"
#include "file_transfer.pb.h"
#include "sec_asio_common.h"
using namespace secft::proto::file_transfer_packet;
class sec_file_client : public boost::enable_shared_from_this<sec_file_client>
                  , boost::noncopyable {
    typedef sec_file_client self_type;
    sec_file_client();
    bool _start(ip::tcp::endpoint ep);
public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<sec_file_client> ptr;

    static ptr start(ip::tcp::endpoint ep) {
        ptr new_(new sec_file_client());
        if(!new_->_start(ep)) {
            return nullptr;
        }
        return new_;
    }
    void stop();
    bool send_request(Request request);

private:
    ip::tcp::socket sock_;
    ip::tcp::endpoint ep_;
};
#endif
