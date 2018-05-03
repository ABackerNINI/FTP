#include <conio.h>
#include "../../protocol/agent/ftp_dtp/ftp_dtp_user.h"

const char *addr = "192.168.1.107";
unsigned int port = 20;

//void test_ftp_dtp_v1() {
//    ftp_dtp::ftp_dtp dtp;
//
//    dtp.set_fpath("file_to_be_sent");
//    dtp.set_addr(addr);
//    dtp.set_port(port);
//    dtp.start();
//
//    _getch();
//
//    dtp.abort();
//
//    _getch();
//
//    dtp.close();
//}

void test_ftp_dtp_v2() {
    ftp_dtp::FtpDtp dtp;
    //dtp.start(FTP_DTP_TYPE_USER_ACTIVE_RECV, "client_root/file_recved", addr, port);
    dtp.start(FTP_DTP_TYPE_USER_ACTIVE_SEND, "client_root/file_to_be_sent", addr, port);

    _getch();

    dtp.abort();

    _getch();

    dtp.close();
}

int main() {
    test_ftp_dtp_v2();

    network::Cleanup();

    return 0;
}