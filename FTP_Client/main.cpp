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
		_Client.~Client();
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

	_getch();

	return 0;
}
