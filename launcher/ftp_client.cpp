#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "../protocol/agent/ftp_client/ftp_client_pi.h"

ftp_client_pi::ftp_client_pi _Client;

BOOL CALLBACK ConsoleHandler(DWORD _Ev) {
    BOOL _Ret = FALSE;
    switch (_Ev) {
    case CTRL_CLOSE_EVENT:
        _Client.Close();
        _Ret = TRUE;
        break;
    default:
        break;
    }
    return _Ret;
}

int main() {
    SetConsoleCtrlHandler(ConsoleHandler, true);

    const char *addr = "192.168.1.107";
    unsigned int port = 21;

    char _Cmd[1000 + 1];
    size_t _Strlen;

    _Client.FtpConnect(addr, port);

    while (true) {
        printf(">");
        fgets(_Cmd, 1000, stdin);

        if (_stricmp(_Cmd, "QUIT\n") == 0) {
            _Client.Close();
            break;
        } else if (_stricmp(_Cmd, "RECONN\n") == 0) {
            _Client.FtpConnect(addr, port);
            continue;
        }

        _Strlen = strlen(_Cmd);
        if (_Strlen > 1) {
            _Cmd[_Strlen - 1] = '\r';
            _Cmd[_Strlen] = '\n';
            _Cmd[_Strlen + 1] = '\0';
            _Client.FtpSend(_Cmd, _Strlen + 2);
        }
    }

    _getch();

    _Client.Close();

    network::Cleanup();

    return 0;
}
