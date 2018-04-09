#include <stdio.h>
#include <iostream>
#include <conio.h>
//#include "../Resource/Utility/Network/Network.h"
#include "FtpServer.h"

FtpServer _Server;

void TestClientInf() {
	ClientInf _ClientInf;

	char *p1 = new char[100];
	char *p2 = new char[100];
	strcpy(p1, "login usrname\r\npassword 123");
	strcpy(p2, "456\r\n");

	_ClientInf.Push(p1, strlen(p1));

	char *s;
	while (s = _ClientInf.Pop(), s) {
		printf("%s\n", s);
	}
	_ClientInf.Push(p2, strlen(p2));
	while (s = _ClientInf.Pop(), s) {
		printf("%s\n", s);
	}
}

int main() {
	//TestClientInf();

	network::ServerConfig config;
	config.M_Port = 21;
	config.O_MaxConnect = 10;

	_Server.SetConfig(config);
	_Server.Start();

	_getch();

	return 0;
}
