#pragma once

#ifndef NINI_FTP_FTP_SERVER_H
#define NINI_FTP_FTP_SERVER_H

#include "../Resource/Utility/Network/Network.h"
#include "../Resource/Common/Common.h"

#define DEFAULT_BUFFER_LEN 1024

enum CLIENT_STATUS {
	CLTSTA_CONNECTED,
	CLTSTA_USRNAME_SPECIFIED,
	CLTSTA_PASSWORD_SPECIFIED
};

struct ClientInf {
	char*	m_Buffer;
	char*	m_pBuffer;
	bool	m_FlagUsingBuffer;
	int		m_PosFront;
	int		m_PosEnd;

	char	m_Usrname[100];
	char	m_Passwd[100];
	char	m_Dir[100];
	CLIENT_STATUS m_Status;

	ClientInf() :
		m_Buffer(NULL),
		m_pBuffer(NULL),
		m_FlagUsingBuffer(false),
		m_PosFront(0),
		m_PosEnd(DEFAULT_BUFFER_LEN),
		m_Status(CLTSTA_CONNECTED) {
		m_Usrname[0] = '\0';
		m_Passwd[0] = '\0';
		m_Dir[0] = '/';
		m_Dir[1] = '\0';
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

class FtpServer :public network::Server {
public:
	void OnAccepted(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnRecvd(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnSent(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnClosed(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

protected:
	bool _Handle(SOCKET _Socket, ClientInf *_ClientInf);

	enum FTP_CMDS _Dispatch(char **_Str);

	bool _FtpSend(SOCKET _Socket, const char *_Buffer);
};

#endif //NINI_FTP_FTP_SERVER_H