#include <stdio.h>
#include <iostream>
#include <conio.h>
#include "../protocol/agent/ftp_server_pi/ftp_server_pi.h"

class ftp_server {
public:
    int start(unsigned int port, const network::ServerConfig &config) {
        m_server_pi.set_config(config);
        m_server_pi.start_listen(port);

        return 0;
    }

    int close() {
        m_server_pi.close();

        return 0;
    }
private:
    ftp_server_pi::ftp_server_pi    m_server_pi;
    ftp_dtp::ftp_dtp                m_server_dtp;
};


int main() {
    unsigned int port = 21;
    network::ServerConfig config;
    config.o_max_connect = 10;

    ftp_server ftp_server;
    ftp_server.start(port, config);

    _getch();

    ftp_server.close();

    network::Cleanup();

    return 0;
}
