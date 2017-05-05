#pragma once

#ifndef NINI_FTP_FTP_SERVER_HANDLER_H
#define NINI_FTP_FTP_SERVER_HANDLER_H

#include "../Resource/Utility/Network/Network.h"

struct Client{
	
};

class FtpServerHandler {
public:
	static void CommitHandler(network::SVR_SOCKET_CONTEXT *_SocketContext, network::ServerCallback _ServerCallback);

	static void ServerCallback(network::SVR_SOCKET_CONTEXT *_SocketContext, int _Ev, void *_Data);
protected:

};

#endif //NINI_FTP_FTP_SERVER_HANDLER_H