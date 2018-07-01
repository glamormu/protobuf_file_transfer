// test/test.c
#include <stdio.h>
#include <stdlib.h>

#include "secft.h"

int main(int argc, char **argv)
{
    // test_asio();
    /*
     secft_server_start_up(int port);
     secft_server_stop();
     //todo secft_server_handle_event();
     */
    secft_server_set_property(SECFT_SERVER_ADD_AUTH, "test:token");
    secft_server_start_up(10086);

    printf("%d\n", add_fun(1,2));
    return 0;
}
