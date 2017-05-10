#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "FtpClient.h"

network::ClientConfig _ClientConfig;

FtpClient _Client;

BOOL CALLBACK ConsoleHandler(DWORD _Ev) {
	BOOL bRet = FALSE;
	switch (_Ev)
	{
	case CTRL_CLOSE_EVENT:
		_Client.Close();
		bRet = TRUE;
		break;
	default:
		break;
	}
	return bRet;
}

int main() {
	SetConsoleCtrlHandler(ConsoleHandler, true);

	_ClientConfig.O_IpPort.M_Ip = "192.168.10.132";//"192.168.1.102";
	_ClientConfig.O_IpPort.M_Port = 21;

	_Client.SetConfig(_ClientConfig);
	_Client.Connect();

	char _Cmd[1000+1];
	int _Strlen;

	while (true) {
		printf(">");
		fgets(_Cmd, 1000, stdin);

		if (stricmp(_Cmd, "quit\n") == 0) {
			_Client.Close();
			continue;
		} else if (stricmp(_Cmd, "reconn\n") == 0) {
			_Client.Connect();
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
