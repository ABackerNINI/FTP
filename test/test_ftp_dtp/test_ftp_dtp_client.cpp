#include <conio.h>
#include "../../protocol/agent/ftp_dtp/ftp_dtp.h"

network::ClientConfig client_config;
ftp_dtp::ftp_dtp_client dtp_client;

int main() {
    const char *addr = "192.168.1.107";
    unsigned int port = 20;

    dtp_client.connect(addr, port);

    _getch();

    dtp_client.close();

    network::Cleanup();

    return 0;
}