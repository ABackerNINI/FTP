#pragma once

#ifndef NINI_FTP_FTP_CLIENT_H
#define NINI_FTP_FTP_CLIENT_H

#include "../Resource/Utility/Network/Network.h"

#define DEFAULT_BUFFER_LEN 1024

enum CLIENT_STATUS {
	CLTSTA_CONNECTING,
	CLTSTA_CONNECTED,
	CLTSTA_SENDING,
	CLTSTA_SENT,
	CLTSTA_RECVD,
	CLTSTA_RSP_HANDLED,
	CLTSTA_CLOSED
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

class FtpClient :public network::Client {
public:
	bool FtpSend(const char *_Buffer,int _Count);

	void OnConnected(network::CLT_SOCKET_CONTEXT *_SocketContext) override;

	void OnSent(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

	void OnRecvd(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

	void OnClosed(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

protected:
	void _HandleResponse();

protected:
	//FTP_CMDS		m_LastCmd;

	ClientInf		m_ClientInf;

	CLIENT_STATUS	m_ClientStatus;
};

#endif//NINI_FTP_FTP_CLIENT_H