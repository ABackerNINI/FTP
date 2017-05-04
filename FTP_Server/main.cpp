#include <stdio.h>
#include <iostream>
#include <conio.h>
#include "../Resource/Utility/Network/Network.h"


void CALLBACK ServerCallback(SOCKET _Socket, int _Ev, void *_Data) {

}

int main() {
	network::ServerConfig config;
	config.M_Port = 80;
	config.O_MaxConnect = 10;
	config.M_ServerCallback = ServerCallback;

	network::Server server(config);
	server.Start();

	_getch();

	return 0;
}
