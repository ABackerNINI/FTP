#include <stdio.h>
#include <iostream>
#include <conio.h>
#include "../Resource/Utility/Network/Network.h"


int main() {
	network::Server server;

	network::config_server config;
	config.port = 80;
	config.max_connect = 100;
	server.Start(&config, NULL);

	_getch();

	return 0;
}
