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

	_Client.SetConfig(_ClientConfig);
	_Client.Connect();

	char _Cmd[10000+1];
	/*for (int i = 0; i < 100; ++i) {
		_Cmd[i] = 'a' + (i % 26);
		_Client.Send(_Cmd, i);
	}

	printf("over\n");*/

	while (true) {
		scanf("%s", _Cmd);

		if (strcmp(_Cmd, "quit") == 0) {
			_Client.Close();
			continue;
		}

		_Client.Send(_Cmd, strlen(_Cmd));
	}

	_getch();

	_Client.Close();

	return 0;
}
