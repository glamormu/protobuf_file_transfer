#include "sec_async_client.h"

void sec_async_client::do_write(const std::string &msg) {
    if ( !started() ) return;
    std::copy(msg.begin(), msg.end(), write_buffer_);
    sock_.async_write_some( buffer(write_buffer_, msg.size()),
                            MEM_FN2(on_write,_1,_2));
}

size_t sec_async_client::read_complete(const boost::system::error_code &err, size_t bytes) {
    if ( err) return 0;
    bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
    // we read one-by-one until we get to enter, no buffering
    return found ? 0 : 1;
}

void sec_async_client::do_read() {
    async_read(sock_, buffer(read_buffer_),
               MEM_FN2(read_complete,_1,_2), MEM_FN2(on_read,_1,_2));
}

void sec_async_client::on_read(const sec_async_client::error_code &err, size_t bytes) {
    if ( !err) {
        std::string copy(read_buffer_, bytes - 1);
        std::cout << "server echoed our " << message_ << ": "
                  << (copy == message_ ? "OK" : "FAIL") << std::endl;
    }
    stop();
}

void sec_async_client::on_write(const sec_async_client::error_code &err, size_t bytes) {
    do_read();
}

sec_async_client::sec_async_client()
    : sock_(service), started_(true){}

void sec_async_client::_start(ip::tcp::endpoint ep) {
    sock_.async_connect(ep, MEM_FN1(on_connect,_1));
}

void sec_async_client::stop() {
    if ( !started_) return;
    started_ = false;
    sock_.close();
}

void sec_async_client::on_connect(const sec_async_client::error_code &err) {
    if ( !err)      do_write(message_ + "\n");
    else            stop();
}
