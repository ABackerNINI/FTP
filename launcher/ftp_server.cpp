#include <stdio.h>
#include <iostream>
#include <conio.h>
#include "../protocol/agent/ftp_server/ftp_server_pi.h"

ftp_server_pi::ftp_server_pi server;

int main() {
    unsigned int port = 21;
    network::ServerConfig config;
    config.o_max_connect = 10;

    server.set_config(config);
    server.start_listen(port);

    _getch();

    network::Cleanup();

    return 0;
}
