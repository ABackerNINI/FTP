#include <stdio.h>
#include <iostream>
#include <conio.h>
#include "../protocol/agent/ftp_server/ftp_server_pi.h"

ftp_server_pi::ftp_server_pi server;

int main() {
    network::ServerConfig config;
    config.m_port = 21;
    config.o_max_connect = 10;

    server.set_config(config);
    server.start();

    _getch();

    network::Cleanup();

    return 0;
}
