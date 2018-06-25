#include <stdio.h>
#include "file_transfer.pb.h"
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <thread>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "secft_asio_common.h"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "plog/Log.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "file_transfer.pb.h"
using namespace secft::proto::file_transfer_packet;
using namespace boost::asio;
using namespace boost::posix_time;

static int port_;


struct talk_to_client;
typedef boost::shared_ptr<talk_to_client> client_ptr;
typedef std::vector<client_ptr> array;
array clients;

boost::recursive_mutex cs;

const int max_msg = 4096*2;
/*
 * 1. Get auth packet; verify token
 * 2. Get data packet: write data
 * 3. if time out, close server
 * 4. if not match, close server
 *
 */
struct talk_to_client : public boost::enable_shared_from_this<talk_to_client>
, boost::noncopyable{
    typedef talk_to_client self_type;
    talk_to_client() : sock_(service) {
        memset(data_buffer, 0, max_msg);
    }
    ip::tcp::socket & sock() {
        return sock_;
    }
    void set_clients_logged_in() {
        logged_in_ = true;
    }
    std::string username() const { return username_; }
    void stop() {
        // close client connection
        boost::system::error_code err;
        sock_.close(err);
    }
    void answer_to_client() {
        try {
            read_request();
            process_request();
        } catch ( boost::system::system_error&) {
            stop();
        }
        if ( timed_out()) {
            stop();
            std::cout << "stopping " << username_ << " - no ping in time" << std::endl;
        }
    }
    bool timed_out() const {
        //ptime now = microsec_clock::local_time();
        //long long ms = (now - last_ping).total_milliseconds();
        long long ms  = 0;
        return ms > 5000 ;
    }
private:
    void read_request() {
        char buff[4] = {0};
        if ( sock_.available()){
            int acc_read = sock_.receive(buffer(buff, 4), ip::tcp::socket::message_peek );
            LOG_DEBUG << "acc_read is " << acc_read << std::endl;
            google::protobuf::uint32 size;
            google::protobuf::io::ArrayInputStream ais_hdr(buff,4);
            google::protobuf::io::CodedInputStream coded_input_hdr(&ais_hdr);
            coded_input_hdr.ReadVarint32(&size);//Decode the HDR and get the size
            LOG_DEBUG << "size of payload is " << size << std::endl;

            already_read_ = sock_.read_some(
                        buffer(data_buffer, 4+size));
            if(already_read_ <= 0) {
                LOG_DEBUG << "already_read_ is " << already_read_;
                return;
            }
            google::protobuf::io::ArrayInputStream ais(data_buffer,size+4);
            google::protobuf::io::CodedInputStream coded_input(&ais);
            //Read an unsigned integer with Varint encoding, truncating to 32 bits.
            coded_input.ReadVarint32(&size);
            //After the message's length is read, PushLimit() is used to prevent the CodedInputStream
            //from reading beyond that length.Limits are used when parsing length-delimited
            //embedded messages
            google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(size);
            //De-Serialize

            Request request;
            request.ParseFromCodedStream(&coded_input);
            //Once the embedded message has been parsed, PopLimit() is called to undo the limit
            coded_input.PopLimit(msgLimit);
            //Print the message
        }


    }
    void process_request() {
        if(already_read_ <= 0) {
            return;
        }
        //Step 0. generate packet


        //Step 1. handle auth

        //Step 2. handle packet
    }
private:
    ip::tcp::socket sock_;
    std::string username_;
    std::string token_;
    bool logged_in_;
    ptime last_packet;
    char data_buffer[max_msg];
    int already_read_;
};
void update_clients_() {
    boost::recursive_mutex::scoped_lock lk(cs);
    for( array::iterator b = clients.begin(), e = clients.end(); b != e; ++b)
        (*b)->set_clients_logged_in();
}
void accept_thread() {
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), port_));
    while ( true) {
        client_ptr new_( new talk_to_client);
        acceptor.accept(new_->sock());

        boost::recursive_mutex::scoped_lock lk(cs);
        clients.push_back(new_);
    }
}

void handle_clients_thread() {
    while ( true) {
        boost::this_thread::sleep( millisec(1));
        boost::recursive_mutex::scoped_lock lk(cs);
        for ( array::iterator b = clients.begin(), e = clients.end(); b != e; ++b)
            (*b)->answer_to_client();
        // erase clients that timed out
        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     boost::bind(&talk_to_client::timed_out,_1)), clients.end());
    }
}
typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;



void client_session(socket_ptr sock) {
    std::cerr << "Connected";
    try
    {
        for (;;)
        {
            char data[4096*2];
            boost::system::error_code error;
            size_t length = sock->read_some(boost::asio::buffer(data), error);
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            if (length > 0){
                length = boost::asio::write(*sock, buffer("ok", 2), error);
            }
            if (error == boost::asio::error::eof || length == 0){
                break;
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

void test_asio()
{
    ip::tcp::endpoint ep(ip::tcp::v4(), 10086);
    std::cerr << "listening.....\n";
    ip::tcp::acceptor acc(service, ep);
    while (true) {
        socket_ptr sock(new ip::tcp::socket(service));
        acc.accept(*sock);
        boost::thread( boost::bind(client_session, sock));
    }
}

int test_proto()
{
    return 0;
}
int add_fun(int a, int b)
{
    return a+b;
}

void secft_server_listening() {

}

int secft_server_start_up(int port)
{
    plog::init(plog::debug, "secft_server");
    port_ = port;
    boost::thread_group threads;
    threads.create_thread(accept_thread);
    threads.create_thread(handle_clients_thread);
    threads.join_all();
    return 0;
}
