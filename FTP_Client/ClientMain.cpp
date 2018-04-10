#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "FtpClient.h"
#include "../Resource/Utility/StringBuffer/StringBuffer.h"

FtpClientConfig _ClientConfig;

FtpClient _Client;

BOOL CALLBACK ConsoleHandler(DWORD _Ev) {
	BOOL _Ret = FALSE;
	switch (_Ev)
	{
	case CTRL_CLOSE_EVENT:
		_Client.Close();
		_Ret = TRUE;
		break;
	default:
		break;
	}
	return _Ret;
}

void StringBufferTest() {
    StringBuffer sb;
    char *s1 = new char[100];
    char *s2 = new char[100];
    char *s3 = new char[100];
    char *s4 = new char[100];

    strcpy(s1, "214 ");
    strcpy(s2, "Supply a user password: PASS password");
    strcpy(s3, "\r\n");
    strcpy(s4, "123\r\n");

    sb.push(s4, strlen(s4));

    printf("%s", sb.pop());

    sb.push(s1, strlen(s1));
    sb.push(s2, strlen(s2));
    sb.push(s3, strlen(s3));

    printf("%s", sb.pop());
}

int main() {
    //StringBufferTest();

	SetConsoleCtrlHandler(ConsoleHandler, true);

	network::IP_PORT _IpPort;
	_IpPort.M0_Ip_String = "192.168.1.107";
	_IpPort.M_Port = 21;
	_ClientConfig.M_Port = 1027;

	_Client.SetConfig(_ClientConfig);

	char _Cmd[1000 + 1];
	size_t _Strlen;

	_Client.FtpConnect(&_IpPort);

	while (true) {
		printf(">");
		fgets(_Cmd, 1000, stdin);

		if (_stricmp(_Cmd, "QUIT\n") == 0) {
			_Client.Close();
			break;
		} else if (_stricmp(_Cmd, "RECONN\n") == 0) {
			_Client.FtpConnect(&_IpPort);
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

	return 0;
}
