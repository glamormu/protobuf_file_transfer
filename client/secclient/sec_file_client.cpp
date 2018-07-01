#include "sec_file_client.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "plog/Log.h"

sec_file_client::sec_file_client():sock_(service), started_(true){}

void sec_file_client::stop()
{
    sock_.close();
}

bool sec_file_client::send_request(Request request)
{
    //https://stackoverflow.com/questions/9496101/protocol-buffer-over-socket-in-c
    int siz = request.ByteSize()+4;
    char *pkt = new char [siz];
    google::protobuf::io::ArrayOutputStream aos(pkt,siz);
    google::protobuf::io::CodedOutputStream *coded_output = new google::protobuf::io::CodedOutputStream(&aos);
    coded_output->WriteVarint32(request.ByteSize());
    request.SerializeToCodedStream(coded_output);

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


int sec_file_client::read_reply(Reply & reply) {
    char buff[4] = {0};
    alread_read_ = 0;
    try{
        int acc_read = sock_.receive(buffer(buff, 4),
                                     ip::tcp::socket::message_peek);
        LOG_DEBUG << "acc_read is " << acc_read << std::endl;
        google::protobuf::uint32 size;
        google::protobuf::io::ArrayInputStream ais_hdr(buff, 4);
        google::protobuf::io::CodedInputStream coded_input_hdr(&ais_hdr);

        coded_input_hdr.ReadVarint32(&size);
        LOG_DEBUG << "size of payload is " << size << std::endl;
        alread_read_ = read(sock_, buffer(buffer_, 4 + size));

        if(alread_read_ <= 0) {
            LOG_DEBUG << "alread_read_ is " << alread_read_;
            return -1;
        }
        google::protobuf::io::ArrayInputStream ais(buffer_, size+4);
        google::protobuf::io::CodedInputStream coded_input(&ais);
        coded_input.ReadVarint32(&size);
        google::protobuf::io::CodedInputStream::Limit msgLimit =
                coded_input.PushLimit(size);
        reply.ParseFromCodedStream(&coded_input);
        coded_input.PopLimit(msgLimit);
    }catch(std::exception& e) {
        LOG_ERROR << e.what() << std::endl;
        return -1;
    }
    if(reply.status() == Status::STATUS_SUCCESS) {
        LOG_DEBUG << "Status::STATUS_SUCCESS";
    }
    else {
        LOG_DEBUG << "Status: ERROR";
    }
    return 0;
}



