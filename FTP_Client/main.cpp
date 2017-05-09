#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "../Resource/Utility/Network/Network.h"

network::ClientConfig _ClientConfig;

network::Client _Client;

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

	char _Cmd[10000+1];
	int _Strlen;
	//for (int i = 0; i < 10000; ++i) {
	//	_Cmd[i] = 'a' + (i % 26);
	//	_Client.Send(_Cmd, i);
	//}

	//printf("over\n");


	while (true) {
		fgets(_Cmd, 10000, stdin);

		if (strcmp(_Cmd, "quit") == 0) {
			_Client.Close();
			continue;
		}
		_Strlen = strlen(_Cmd);
		if (_Strlen > 1) {
			_Cmd[_Strlen - 1] = '\r';
			_Cmd[_Strlen++] = '\n';
			_Client.Send(_Cmd, _Strlen);
		}
	}

	_getch();

	_Client.Close();

	return 0;
}
