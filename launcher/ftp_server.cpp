#include <stdio.h>
#include <iostream>
#include <conio.h>
#include "../protocol/agent/ftp_server/ftp_server_pi.h"

ftp_server_pi _Server;

int main() {
    network::ServerConfig config;
    config.M_Port = 21;
    config.O_MaxConnect = 10;

    _Server.SetConfig(config);
    _Server.Start();

    _getch();

    return 0;
}
