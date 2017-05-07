#pragma once

#ifndef NINI_FTP_FTP_SERVER_HANDLER_H
#define NINI_FTP_FTP_SERVER_HANDLER_H

#include "../Resource/Utility/Network/Network.h"

#define DEFAULT_BUFFER_LEN 1024

struct Client {
	char *m_Buffer;
	int m_PosFront, m_PosEnd, m_BufferLen;

	Client() :
		m_Buffer(new char[DEFAULT_BUFFER_LEN]),
		m_PosFront(0),
		m_PosEnd(DEFAULT_BUFFER_LEN),
		m_BufferLen(DEFAULT_BUFFER_LEN){
	}

	void Push(const char *_Buffer,unsigned int _Count) {
		
	}

	char *Pop() {
	
	}
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