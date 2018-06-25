#include "sec_file_client.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
sec_file_client::sec_file_client():sock_(service){}

bool sec_file_client::_start(ip::tcp::endpoint ep)
{
    error_code err;
    sock_.connect(ep, err);
    if(err) {
        return false;
    }
    return true;
}

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
//    std::string msg;
//    if(!request.SerializeToString(&msg))
//        return false;
    sock_.write_some(buffer(pkt, siz), err);
    if(err) {
        delete[] pkt;
        return false;
    }
    //todo: read response
    delete[] pkt;
    return true;
}



