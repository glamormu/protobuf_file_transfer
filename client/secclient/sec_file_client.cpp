#include "sec_file_client.h"

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
    error_code err;
    std::string msg;
    if(!request.SerializeToString(&msg))
        return false;
    sock_.write_some(buffer(msg), err);
    if(err) {
       return false;
    }
    //todo: read response

    return true;
}



