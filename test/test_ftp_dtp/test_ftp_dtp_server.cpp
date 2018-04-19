#include <conio.h>
#include "../../protocol/agent/ftp_dtp/ftp_dtp.h"

ftp_dtp::ftp_dtp dtp;

int main() {
    unsigned int port = 20;

    dtp.set_fpath("file_recvd");
    dtp.set_port(20);
    dtp.set_passive(true);
    dtp.start();

    _getch();

    dtp.abort();

    _getch();

    dtp.close();

    network::Cleanup();

    return 0;
}