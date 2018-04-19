#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "../protocol/agent/ftp_client/ftp_client_pi.h"

ftp_client_pi::ftp_client_pi client;

BOOL CALLBACK ConsoleHandler(DWORD ev) {
    BOOL ret = FALSE;
    switch (ev) {
    case CTRL_CLOSE_EVENT:
        client.close();
        ret = TRUE;
        break;
    default:
        break;
    }
    return ret;
}

int main() {
    SetConsoleCtrlHandler(ConsoleHandler, true);

    const char *addr = "192.168.1.107";
    unsigned int port = 21;

    char cmd[1000 + 1];
    size_t len;

    client.ftp_connect(addr, port);

    while (true) {
        printf(">");
        fgets(cmd, 1000, stdin);

        if (_stricmp(cmd, "QUIT\n") == 0) {
            client.close();
            break;
        } else if (_stricmp(cmd, "RECONN\n") == 0) {
            client.ftp_connect(addr, port);
            continue;
        }

        len = strlen(cmd);
        if (len > 1) {
            client.ftp_input(cmd, len);
        }
    }

    _getch();

    client.close();

    network::Cleanup();

    return 0;
}
