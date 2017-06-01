#pragma once

#ifndef NINI_FTP_FTP_CLIENT_H
#define NINI_FTP_FTP_CLIENT_H

#include "../Resource/FtpCmds/FtpCmds.h"
#include "../Resource/Utility/Network/Network.h"

#define DEFAULT_BUFFER_LEN 1024

enum CLIENT_IO_STATUS {
	CIS_CONNECTING,
	CIS_CONNECTED,
	CIS_SENDING,
	CIS_SENT,
	CIS_RECVD,
	CIS_RSP_HANDLED,
	CIS_CLOSED
};

enum CLIENT_LOGIN_STATUS {
	CLS_NOT_CONNECTED,
	CLS_CONNECTED,
	CLS_USERNAME_SPECIFIED,
	CLS_PASSWORD_SPECIFIED,
	CLS_LOGIN_SUCCESS,
	CLS_DISCONNECTED
};

struct FtpClientConfig :network::ClientConfig {
	int M_Port;
};

struct ClientInf {
	char*	m_Buffer;
	char*	m_pBuffer;
	bool	m_FlagUsingBuffer;
	int		m_PosFront;
	int		m_PosEnd;

	ClientInf() :
		m_Buffer(NULL),
		m_pBuffer(NULL),
		m_FlagUsingBuffer(false),
		m_PosFront(0),
		m_PosEnd(DEFAULT_BUFFER_LEN) {
	}

	void Push(char *_Buffer, unsigned int _Count) {
		if (!m_FlagUsingBuffer) {
			m_pBuffer = _Buffer;
			m_PosFront = 0;
			m_PosEnd = _Count;
		} else {
			memcpy(m_Buffer + m_PosEnd, _Buffer, _Count * sizeof(char));
			m_PosEnd += _Count;
		}
	}

	char *Pop() {
		char *p = (m_FlagUsingBuffer ? m_Buffer : m_pBuffer) + m_PosFront;
		char *tmp = p;
		for (int i = m_PosFront; i < m_PosEnd; ++i, ++p) {
			if (*p == '\r'&&*(p + 1) == '\n') {
				*p = '\0';

				m_PosFront = i + 2;

				return tmp;
			}
		}

		if (m_PosFront < m_PosEnd) {
			m_FlagUsingBuffer = true;
			if (!m_Buffer) {
				m_Buffer = new char[DEFAULT_BUFFER_LEN];//TODO alloc after login
			}
			memcpy(m_Buffer, m_pBuffer + m_PosFront, m_PosEnd - m_PosFront);
			m_PosEnd = m_PosEnd - m_PosFront;
			m_PosFront = 0;
		}

		return NULL;
	}

	~ClientInf() {
		if (m_Buffer)delete[] m_Buffer;
	}
};

class FtpClientServer :public network::Server {
public:
protected:
	void OnAccepted(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnRecvd(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnSent(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnClosed(network::SVR_SOCKET_CONTEXT *_SocketContext) override;
};

class FtpClientData :public network::Client {
public:
	FtpClientData();

	FtpClientData(const FtpClientConfig &_FtpClientConfig);

	void SetConfig(const FtpClientConfig &_FtpClientConfig);

	bool FtpSend(const char *_Buffer, int _Count);

protected:
	void OnConnected(network::CLT_SOCKET_CONTEXT *_SocketContext) override;

	void OnSent(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

	void OnRecvd(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

	void OnClosed(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

	void _HandleResponse();

protected:
	int	m_Port;
};

class FtpClient :public network::Client {
public:
	FtpClient();

	FtpClient(const FtpClientConfig &_FtpClientConfig);

	void SetConfig(const FtpClientConfig &_FtpClientConfig);

	bool FtpConnect(const network::IP_PORT *_IpPort);

	bool FtpSend(const char *_Buffer,int _Count);

	CLIENT_IO_STATUS GetIoStatus();

	bool Close();

protected:
	void OnConnected(network::CLT_SOCKET_CONTEXT *_SocketContext) override;

	void OnSent(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

	void OnRecvd(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

	void OnClosed(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

	void _HandleResponse();

protected:
	int					m_Socket;

	FTP_CMDS			m_LastCmd;

	ClientInf			m_ClientInf;

	CLIENT_IO_STATUS	m_ClientStatus;

	//FtpClientData		m_FtpClientData;

	//FtpClientServer		m_FtpClientServer;
};

#endif//NINI_FTP_FTP_CLIENT_H