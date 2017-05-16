#include "FtpServer.h"
#include <assert.h>

void FtpServerData::OnAccepted(network::SVR_SOCKET_CONTEXT * _SocketContext) {
}

void FtpServerData::OnRecvd(network::SVR_SOCKET_CONTEXT * _SocketContext) {
}

void FtpServerData::OnSent(network::SVR_SOCKET_CONTEXT * _SocketContext) {
}

void FtpServerData::OnClosed(network::SVR_SOCKET_CONTEXT * _SocketContext) {
}

void FtpServer::OnAccepted(network::SVR_SOCKET_CONTEXT * _SocketContext) {
	ClientInf *_ClientInf = new ClientInf();

	if (_SocketContext->m_BytesTransferred > 0) {
		_ClientInf->Push(_SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);

		_Handle(_SocketContext->m_ClientSocket, _ClientInf);
	}

	_SocketContext->m_Extra = _ClientInf;
}

void FtpServer::OnRecvd(network::SVR_SOCKET_CONTEXT * _SocketContext) {
	ClientInf *_ClientInf = (ClientInf *)(_SocketContext->m_Extra);

	_ClientInf->Push(_SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);

	_Handle(_SocketContext->m_ClientSocket, _ClientInf);
}

void FtpServer::OnSent(network::SVR_SOCKET_CONTEXT * _SocketContext) {
}

void FtpServer::OnClosed(network::SVR_SOCKET_CONTEXT * _SocketContext) {
	delete (ClientInf *)(_SocketContext->m_Extra);
}

enum FTP_CMDS FtpServer::CmdDispatch(char **_Str) {
	char *p = *_Str;
	FTP_CMDS _Ret = FTP_CMD_ERR;

	while (*p == ' ')++p;

	for (int i = 0; i < FTP_CMDS_NUM; ++i) {
		if (stricmp_n_1(FTP_CMDS_INF[i].m_Cmd, p) == 0) {
			_Ret = (FTP_CMDS)i;
		}
	}

	*_Str += 4;

	while (**_Str == ' ')++(*_Str);

	if (**_Str == '\0')*_Str = NULL;

	return _Ret;
}

bool FtpServer::_Handle(SOCKET _Socket, ClientInf * _ClientInf) {
	char *_Str;
	char *_Args;
	int _Cmd;
	while (_Str = _ClientInf->Pop(), _Str) {
		printf("%s\n", _Str);
		_Args = _Str;
		_Cmd = CmdDispatch(&_Args);

		if (FTP_CMDS_INF[_Cmd].m_NeedArgs&&_Args == NULL) {
			_FtpSend(_Socket, "501 Syntax error in parameters or arguments.\r\n");
			continue;
		}

		if ((_Cmd != FTP_CMD_USER&&_Cmd != FTP_CMD_PASS) && _ClientInf->m_Status == CLS_CONNECTED) {
			_FtpSend(_Socket, "530 Please login with USER and PASS.\r\n");
			continue;
		}

		m_CmdHandler[_Cmd](this, _Socket, _ClientInf, _Args);
	}

	return false;
}

bool FtpServer::_FtpSend(SOCKET _Socket, const char * _Buffer) {
	return Send(_Socket, _Buffer, strlen(_Buffer));
}

void FtpServer::_CmdHandler_USER(SOCKET _Socket, ClientInf *_ClientInf, char * _Args) {
	switch (_ClientInf->m_Status) {
	case CLS_CONNECTED:
		_ClientInf->m_Status = CLS_USRNAME_SPECIFIED;
		_FtpSend(_Socket, "331 User name ok,need password.\r\n");
		break;
	case CLS_USRNAME_SPECIFIED:
		_FtpSend(_Socket, "331 User name ok,need password.\r\n");
		break;
	case CLS_PASSWORD_SPECIFIED:
		_ClientInf->m_Status = CLS_USRNAME_SPECIFIED;
		_FtpSend(_Socket, "331 User name ok,need password.\r\n");//TODO
		break;
	default:
		assert(false);
		break;
	}
}

void FtpServer::_CmdHandler_PASS(SOCKET _Socket, ClientInf *_ClientInf, char * _Args) {
	switch (_ClientInf->m_Status) {
	case CLS_CONNECTED:
		_FtpSend(_Socket, "503 Need account for login.\r\n");
		break;
	case CLS_USRNAME_SPECIFIED:
		_ClientInf->m_Status = CLS_PASSWORD_SPECIFIED;
		_FtpSend(_Socket, "230 User logged in, proceed.\r\n");
		break;
	case CLS_PASSWORD_SPECIFIED:
		_FtpSend(_Socket, "230 User logged in, proceed.\r\n");
		break;
	default:
		assert(false);
		break;
	}
}

void FtpServer::_CmdHandler_CWD(SOCKET _Socket, ClientInf *, char * _Args) {
	_FtpSend(_Socket, "500 CWD.\r\n");
}

void FtpServer::_CmdHandler_PORT(SOCKET _Socket, ClientInf *, char * _Args) {
	_FtpSend(_Socket, "500 PORT.\r\n");
}

void FtpServer::_CmdHandler_PASV(SOCKET _Socket, ClientInf *, char * _Args) {
	_FtpSend(_Socket, "500 PASV.\r\n");
}

void FtpServer::_CmdHandler_RETR(SOCKET _Socket, ClientInf *, char * _Args) {
	_FtpSend(_Socket, "500 RETR.\r\n");
}

void FtpServer::_CmdHandler_STOR(SOCKET _Socket, ClientInf *, char * _Args) {
	_FtpSend(_Socket, "500 STOR.\r\n");
}

void FtpServer::_CmdHandler_ERR(SOCKET _Socket, ClientInf *, char * _Args) {
	_FtpSend(_Socket, "500 Syntax error, command unrecognized.\r\n");
}

void FtpServer::_CmdHandler_NOT_IMPLEMENTED(SOCKET _Socket, ClientInf *, char * _Args) {
	_FtpSend(_Socket, "502 Command not implemented.\r\n");
}
