#ifndef _secft_h_
#define _secft_h_
#include <string>
#define DEBUG 1

#define SECFT_SERVER_ADD_AUTH "secft.server.add_auth"
#define SECFT_SERVER_RM_AUTH  "secft.server.rm_auth"
#define SECFT_SERVER_LOG      "secft.server.logpath"
using std::string;

int secft_server_start_up(int port);
int add_token(string token);
int secft_server_stop();
int secft_server_set_property(const char* prop, const char* value);

#ifdef DEBUG
int add_fun(int a, int b);
int test_proto();
void test_asio();
#endif
#endif//_secft_h_
