#include <conio.h>
#include "../../protocol/agent/ftp_dtp/ftp_dtp.h"

network::ServerConfig server_config;
ftp_dtp::ftp_dtp_server dtp_server;

int main() {
    server_config.M_Port = 20;
    dtp_server.SetConfig(server_config);

    dtp_server.Start();

    _getch();

    dtp_server.Stop();

    return 0;
}