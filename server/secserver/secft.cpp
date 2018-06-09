#include <stdio.h>
#include "file_transfer.pb.h"
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <thread>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
using namespace boost::asio;

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
    io_service service;
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
