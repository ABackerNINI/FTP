#pragma once

#ifndef NINI_FTP_FTP_SERVER_H
#define NINI_FTP_FTP_SERVER_H

#include "../Resource/FtpCmds/FtpCmds.h"
#include "../Resource/Utility/Network/Network.h"
#include "../Resource/Common/Common.h"

#define DEFAULT_BUFFER_LEN 1024
#define _CMD(CMD)  network::pointer_cast<_CmdHandler>(&FtpServer::_CmdHandler##CMD)

enum CLIENT_LOGIN_STATUS {
	CLS_CONNECTED,
	CLS_USRNAME_SPECIFIED,
	CLS_PASSWORD_SPECIFIED //Login Successfully
};

struct ClientInf {
	char*	m_Buffer;
	char*	m_pBuffer;
	int		m_PosFront;
	int		m_PosEnd;
	bool	m_FlagUsingBuffer;

	char	m_Usrname[100];
	char	m_Passwd[100];
	char	m_Dir[100];
	bool	m_IsPasv;
	CLIENT_LOGIN_STATUS m_Status;

	ClientInf() :
		m_Buffer(NULL),
		m_pBuffer(NULL),
		m_PosFront(0),
		m_PosEnd(DEFAULT_BUFFER_LEN),
		m_FlagUsingBuffer(false),
		m_IsPasv(false),
		m_Status(CLS_CONNECTED) {
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

class FtpServerData :public network::Server {
public:
	void OnAccepted(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnRecvd(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnSent(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnClosed(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

protected:
};

class FtpServer :public network::Server {
public:

	void OnAccepted(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnRecvd(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnSent(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

	void OnClosed(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

protected:
	enum FTP_CMDS CmdDispatch(char **_Str);

	bool _Handle(SOCKET _Socket, ClientInf *_ClientInf);

	bool _FtpSend(SOCKET _Socket, const char *_Buffer);

protected:
	typedef void(*_CmdHandler)(FtpServer*, SOCKET, ClientInf*, char *);

	const _CmdHandler m_CmdHandler[FTP_CMDS_NUM + 1] = {//Need to Handle FTP_CMD_ERR
		{ _CMD(_USER)},
		{ _CMD(_PASS)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_CWD)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_PORT)},
		{ _CMD(_PASV)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_RETR)},
		{ _CMD(_STOR)},//14
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_NOT_IMPLEMENTED)},
		{ _CMD(_ERR)}
	};

	void _CmdHandler_USER(SOCKET _Socket, ClientInf*, char *_Args);

	void _CmdHandler_PASS(SOCKET _Socket, ClientInf*, char *_Args);

	void _CmdHandler_CWD(SOCKET _Socket, ClientInf*, char *_Args);

	void _CmdHandler_PORT(SOCKET _Socket, ClientInf*, char *_Args);

	void _CmdHandler_PASV(SOCKET _Socket, ClientInf*, char *_Args);

	void _CmdHandler_RETR(SOCKET _Socket, ClientInf*, char *_Args);

	void _CmdHandler_STOR(SOCKET _Socket, ClientInf*, char *_Args);

	void _CmdHandler_ERR(SOCKET _Socket, ClientInf*, char *_Args);

	void _CmdHandler_NOT_IMPLEMENTED(SOCKET _Socket, ClientInf*, char *_Args);
};

#endif //NINI_FTP_FTP_SERVER_H