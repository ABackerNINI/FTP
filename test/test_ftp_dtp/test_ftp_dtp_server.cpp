#include <conio.h>
#include "../../protocol/agent/ftp_dtp/ftp_dtp.h"

network::ServerConfig server_config;
ftp_dtp::ftp_dtp_server dtp_server;

int main() {
    server_config.m_port = 20;
    dtp_server.set_config(server_config);

    dtp_server.start();

    _getch();

    dtp_server.stop();

    network::Cleanup();

    return 0;
}