#include <stdio.h>
#include <iostream>
#include <conio.h>
#include "../Resource/Utility/Network/Network.h"
#include "FtpServerHandler.h"

network::Server server;

int main() {
	network::ServerConfig config;
	config.M_Port = 80;
	config.O_MaxConnect = 10;
	config.M_ServerCallback = FtpServerHandler::ServerCallback;
	config.O_CommitHandler = FtpServerHandler::CommitHandler;

	server.SetConfig(config);
	server.Start();

	_getch();

	return 0;
}
