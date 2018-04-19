#include <conio.h>
#include "../../protocol/agent/ftp_dtp/ftp_dtp.h"

ftp_dtp::ftp_dtp dtp;

int main() {
    const char *addr = "192.168.1.107";
    unsigned int port = 20;

    dtp.set_fpath("file_to_be_sent");
    dtp.set_addr(addr);
    dtp.set_port(port);
    dtp.start();

    _getch();

    dtp.abort();

    _getch();

    dtp.close();

    network::Cleanup();

    return 0;
}