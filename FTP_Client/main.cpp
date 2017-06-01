#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "FtpClient.h"

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

//bool Connect(FtpClient *_FtpClient) {
//	if (_Client.Connect()) {
//		for (int i = 0; i < 10; ++i) {
//			printf(".");
//			if (_FtpClient->GetIoStatus() != CLIENT_IO_STATUS::CIS_CONNECTING) {
//				printf("\n");
//
//				return true;
//			} else if (i == 9) {
//				printf("\n");
//
//				break;
//			}
//			Sleep(500);
//		}
//	} else {
//	}
//
//	printf("Connect Error,Please Check Your Network.\n");
//
//	return false;
//}

int main() {
	SetConsoleCtrlHandler(ConsoleHandler, true);

	network::IP_PORT _IpPort;
	_IpPort.M0_Ip_String = "192.168.1.102";//"192.168.10.123"; 
	_IpPort.M_Port = 21;
	_ClientConfig.M_Port = 1027;

	_Client.SetConfig(_ClientConfig);

	char _Cmd[1000 + 1];
	int _Strlen;

	_Client.FtpConnect(&_IpPort);

	while (true) {
		printf(">");
		fgets(_Cmd, 1000, stdin);

		if (stricmp(_Cmd, "QUIT\n") == 0) {
			_Client.Close();
			break;
		} else if (stricmp(_Cmd, "RECONN\n") == 0) {
			_Client.FtpConnect(&_IpPort);
			continue;
		}

		_Strlen = strlen(_Cmd);
		if (_Strlen > 1) {
			_Cmd[_Strlen - 1] = '\r';
			_Cmd[_Strlen++] = '\n';
			_Client.FtpSend(_Cmd, _Strlen);
		}
	}

	_getch();

	_Client.Close();

	return 0;
}
