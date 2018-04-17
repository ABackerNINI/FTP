#include <conio.h>
#include "../../protocol/agent/ftp_dtp/ftp_dtp.h"

network::ServerConfig server_config;
ftp_dtp::ftp_dtp_server dtp_server;

int main() {
    server_config.m_port = 20;
    dtp_server.set_config(server_config);

    dtp_server.set_fpath("file_recvd");

    dtp_server.start();

    _getch();

    dtp_server.abort();

    _getch();

    dtp_server.close();

    network::Cleanup();

    return 0;
}