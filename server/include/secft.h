#ifndef _secft_h_
#define _secft_h_
#include <string>
#define DEBUG 1
using std::string;

int secft_server_start_up(int port);
int add_token(string token);
int secft_server_stop();

#ifdef DEBUG
int add_fun(int a, int b);
int test_proto();
void test_asio();
#endif
#endif//_secft_h_
