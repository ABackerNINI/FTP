#include <stdio.h>
#include <iostream>
#include <conio.h>
#include "../Resource/Utility/Network/Network.h"


int main() {
	network::Server server;

	network::config_server config;
	config.port = 80;
	config.max_connect = 10;
	server.start(&config, NULL);

	getch();

	return 0;
}
