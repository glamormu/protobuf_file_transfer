#ifndef _SEC_ASYNC_CLIENT_H
#define _SEC_ASYNC_CLIENT_H
#include "config.h"
#include "file_transfer.pb.h"
#include "sec_asio_common.h"


#define MEM_FN(x)       boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y)    boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&self_type::x, shared_from_this(),y,z)

class sec_async_client : public boost::enable_shared_from_this<sec_async_client>
                  , boost::noncopyable {
    typedef sec_async_client self_type;
    sec_async_client();
    void _start(ip::tcp::endpoint ep);
public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<sec_async_client> ptr;

    static ptr start(ip::tcp::endpoint ep) {
        ptr new_(new sec_async_client());
        new_->_start(ep);
        return new_;
    }
    void stop();
    bool started() { return started_; }
private:
    void on_connect(const error_code & err);
    void on_read(const error_code & err, size_t bytes);

    void on_write(const error_code & err, size_t bytes);
    void do_read();
    void do_write(const std::string & msg);
    size_t read_complete(const boost::system::error_code & err, size_t bytes);

private:
    ip::tcp::socket sock_;
    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;
    std::string message_;
};



#endif
