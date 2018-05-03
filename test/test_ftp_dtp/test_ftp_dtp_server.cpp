#include <conio.h>
#include "../../protocol/agent/ftp_dtp/ftp_dtp.h"

unsigned int local_port = 20;

//void test_ftp_dtp_v1() {
//    ftp_dtp::ftp_dtp dtp;
//
//    dtp.set_fpath("file_recvd");
//    dtp.set_port(20);
//    dtp.set_passive(true);
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
    //dtp.start(FTP_DTP_TYPE_PASSIVE_SEND, "server_root/file_to_be_sent", NULL, 0, &local_port);
    dtp.start(FTP_DTP_TYPE_PASSIVE_RECV, "server_root/file_recved", NULL, 0, &local_port);

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