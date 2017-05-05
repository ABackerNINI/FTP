#pragma once

#ifndef NINI_FTP_FTP_SERVER_HANDLER_H
#define NINI_FTP_FTP_SERVER_HANDLER_H

#include "../Resource/Utility/Network/Network.h"

struct Client {

};

class FtpServer :network::Server {
public:
	void OnAccepted(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnRecvd(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnSent(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnClosed(network::SVR_SOCKET_CONTEXT *_SocketContext) override;
protected:

};

#endif //NINI_FTP_FTP_SERVER_HANDLER_H