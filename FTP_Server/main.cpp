#include <stdio.h>
#include <iostream>
#include <conio.h>
#include "../Resource/Utility/Network/Network.h"


int main() {
	network::ServerConfig config;
	config.M_Port = 80;
	config.O_MaxConnect = 10;

	network::Server server(config);
	server.Start();

	_getch();

	return 0;
}
