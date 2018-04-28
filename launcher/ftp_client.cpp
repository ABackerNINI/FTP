#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "../protocol/agent/ftp_client_pi/ftp_client_pi.h"

class ftp_client {
public:
    int start(const char *addr, unsigned int port) {
        m_client_pi.ftp_connect(addr, port);

        return 0;
    }

    int ftp_input(char *buffer, size_t count) {
        if (_stricmp(buffer, "QUIT\n") == 0) {
            close();

            return -1;
        } /*else if (_stricmp(buffer, "RECONN\n") == 0) {
            client.start(addr, port);

            return 1;
        }*/
        m_client_pi.ftp_input(buffer, count);

        return 0;
    }

    int close() {
        m_client_pi.close();

        return 0;
    }

private:
    ftp_client_pi::ftp_client_pi    m_client_pi;
    //ftp_dtp::ftp_dtp                m_client_dtp;
};

ftp_client client;

BOOL CALLBACK ConsoleHandler(DWORD ev) {
    BOOL ret = FALSE;
    switch (ev) {
    case CTRL_CLOSE_EVENT:
        client.close();
        network::Cleanup();
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

    client.start(addr, port);

    while (true) {
        printf(">");
        fgets(cmd, 1000, stdin);

        len = strlen(cmd);
        if (len > 1) {
            if (client.ftp_input(cmd, len) < 0) {
                break;
            }
        }
    }

    client.close();

    network::Cleanup();

    return 0;
}
