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
#include "secft.h"
#include <map>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <libgen.h>
#include <filesystem>
#include <chrono>
#include <fstream>
using namespace secft::proto::file_transfer_packet;
using namespace boost::asio;
using namespace boost::posix_time;

static int port_;


struct talk_to_client;
typedef boost::shared_ptr<talk_to_client> client_ptr;
typedef std::vector<client_ptr> array;
array clients;

boost::recursive_mutex cs;
std::map<string, string> username_token_map;
const int max_msg = 4096*2;

bool auth_username_token(string username, string token);
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
    typedef boost::system::error_code error_code;
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
        ptime now = microsec_clock::local_time();
        long long ms = (now - last_packet).total_milliseconds();
        return ms > 5000 ;
    }
private:
    void read_request() {
        char buff[4] = {0};
        already_read_ = 0;
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
            std::cout << request.packet().user_name();
            std::cout << request.packet().token();
            client_req = request;
            //Print the message
        }


    }
    void process_request() {
        if(already_read_ <= 0) {
            return;
        }

        last_packet = microsec_clock::local_time();
        //Step 1. handle auth
        if(auth_username_token(client_req.packet().user_name(),
                               client_req.packet().token())) {
            LOG_DEBUG << "Auth ok: user_name is " << client_req.packet().user_name()
                      << " token is " << client_req.packet().token();
        }
        else {
            LOG_DEBUG << "Auth failed: user_name is " << client_req.packet().user_name()
                      << " token is " << client_req.packet().token();
            stop();
            return;
        }
        //Step 2. handle packet
        string download_path = client_req.download_request().path();
        string upload_path = client_req.upload_request().path();
        if(download_path.size() != 0) {
            do_download();
        }
        else if (upload_path.size() != 0) {
            do_upload();
        }
        else{
            LOG_DEBUG << "Invaliad packet: no upload request and download request;";
            stop();
        }
    }
    ssize_t get_packet(string file_path, Packet &packet){

    }
    void send_download_packet(){
        string path =client_req.download_request().path();
        std::filesystem::path p(path);
        string root_path = "./";
        string file_path = root_path + path;
        Reply reply;
        Packet packet;
        ssize_t offset = 0;
        ssize_t file_size = std::filesystem::file_size(p);

        std::ifstream infile;
        infile.open(file_path, std::ios::in|std::ios::binary);
        if(!infile.is_open()){
            reply.set_status(STATUS_FILE_OPEN_ERROR);
            send_reply(reply);

        }
        else {
            while(offset < file_size){
                reply.set_status(STATUS_SUCCESS);
                ssize_t acc_read = get_packet(file_path, packet);
                if(acc_read > 0) {
                    offset += acc_read;
                    reply.set_allocated_packet(&packet);
                    send_reply(reply);
                }
                else {
                    reply.set_status(STATUS_FILE_READ_ERROR);
                    send_reply(reply);
                    break;
                }
            }
        }
        stop();
    }
    void check_download()
    {
        string path =client_req.download_request().path();
        std::filesystem::path p(path);
        LOG_DEBUG << path;
        string root_path = "./";
        string file_path = root_path + path;
        Reply reply;
        LOG_DEBUG << "Try to find " << client_req.download_request().path();
        if(std::filesystem::exists(file_path)){
            reply.set_status(STATUS_PATH_ALREADY_EXISTS);
            if(std::filesystem::is_directory(file_path)){
                //TODO FIX IT
                reply.set_status(STATUS_PATH_NOT_FOUND);
            }
            else {
                FileList *file_list = new FileList();
                FileList::Item *item = file_list->add_item();
                item->set_name(path);
                item->set_is_directory(false);
                item->set_size(std::filesystem::file_size(p));
                std::filesystem::file_time_type ftime
                        = std::filesystem::last_write_time(path);
                //wtime.clock.time_point
                item->set_modification_time(ftime.time_since_epoch().count());
                reply.set_allocated_file_list(file_list);
            }
            send_reply(reply);
        }
        else {
            reply.set_status(STATUS_PATH_NOT_FOUND);
            send_reply(reply);
            stop();
        }
    }
    void do_download(){
        int packet_flag = client_req.packet().flags();
        if(packet_flag == Packet::Flags::Packet_Flags_FLAG_FIRST_PACKET){
            check_download();
        }
        else if(packet_flag == Packet::Flags::Packet_Flags_FLAG_PACKET){
            send_download_packet();
        }
    }
    void do_upload() {
        std::filesystem::path p(client_req.upload_request().path());
        string path = p.stem();
        bool overwrite = client_req.upload_request().overwrite();
        LOG_DEBUG << "upload path is " << path
                  << " and overwrite is " << overwrite;
        Reply reply;
        //save file
        string root_path = "./";
        string file_path = root_path + path;
        std::ofstream outfile;
        int packet_flag = client_req.packet().flags();
        switch (packet_flag) {
        case Packet::Flags::Packet_Flags_FLAG_FIRST_PACKET:
            std::cout << "Start recving packets";
            if(overwrite){
                outfile.open(file_path, std::ios::out|std::ios::binary);
            }
            else{
                if(std::filesystem::exists(file_path)){
                    reply.set_status(Status::STATUS_PATH_ALREADY_EXISTS);
                    send_reply(reply);
                    stop();
                    return;
                }
            }
            break;
        case Packet::Flags::Packet_Flags_FLAG_PACKET:
            LOG_DEBUG << "Recving packets";;
            outfile.open(file_path, std::ios::out|std::ios::binary|std::ios::app);
            break;
        case Packet::Flags::Packet_Flags_FLAG_LAST_PACKET:
            LOG_DEBUG << "Last packet";
            outfile.open(file_path, std::ios::out|std::ios::binary|std::ios::app);
            break;
        default:
            break;
        }

        if(!outfile.is_open()){
            LOG_ERROR << file_path <<"open failed";
            return;
        }
        outfile.write(client_req.packet().data().c_str(), client_req.packet().data().size());
        outfile.close();
        reply.set_status(Status::STATUS_SUCCESS);
        send_reply(reply);
        if(packet_flag == Packet::Flags::Packet_Flags_FLAG_LAST_PACKET) {
            stop();
        }
        //uint64_t file_size = client_req.packet().file_size();
    }
    bool send_reply(Reply reply){
        LOG_DEBUG << "reply status: " << reply.status();
        int siz = reply.ByteSize()+4;
        char *pkt = new char [siz];
        google::protobuf::io::ArrayOutputStream aos(pkt,siz);
        google::protobuf::io::CodedOutputStream *coded_output = new google::protobuf::io::CodedOutputStream(&aos);
        coded_output->WriteVarint32(reply.ByteSize());
        reply.SerializeToCodedStream(coded_output);
        error_code err;
        sock_.write_some(buffer(pkt, siz), err);
        if(err) {
            delete[] pkt;
            return false;
        }
        //todo: read response
        delete[] pkt;
        return true;
    }
private:
    ip::tcp::socket sock_;
    std::string username_;
    std::string token_;
    bool logged_in_;
    ptime last_packet;
    char data_buffer[max_msg];
    int already_read_;
    Request client_req;
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
bool auth_username_token(string username, string token){
    std::map<string, string>::iterator find_iter = username_token_map.find(username);
    if(find_iter == username_token_map.end()) {
        LOG_DEBUG << "Can not find user: " << username;
        return false;
    }
    LOG_DEBUG << "Auth " << username << " Failed";
    return find_iter->second == token;
}
int secft_server_set_property(const char* prop, const char* value){
    //"SECFT_SERVER_ADD_AUTH"
    //"SECFT_SERVER_RM_AUTH"
    //"SECFT_SERVER_LOG"
    if(prop == nullptr || value == nullptr) {
        return -1;
    }
    //TODO: if statement is stupid
    if(!strcmp(prop, SECFT_SERVER_ADD_AUTH)){
        std::vector<std::string> strs;
        boost::split(strs, value, boost::is_any_of(":"));
        if(strs.size() != 2) {
            return -1;
        }
        username_token_map[strs[0]] = strs[1];
    }
    else if(!strcmp(prop, SECFT_SERVER_ADD_AUTH)) {
        //todo remove item
        return 0;
    }
    return 0;
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
