#include <stdio.h>
#include <iostream>
#include <conio.h>
#include "../Resource/Utility/Network/Network.h"
#include "FtpServer.h"

network::Server server;

int main() {
	network::ServerConfig config;
	config.M_Port = 80;
	config.O_MaxConnect = 10;

	server.SetConfig(config);
	server.Start();

	_getch();

	return 0;
}
