#include "FtpServer.h"
#include "../Resource/Common/Common.h"
#include "FtpCmd.h"

void FtpServer::OnAccepted(network::SVR_SOCKET_CONTEXT * _SocketContext) {
	ClientInf *_ClientInf = new ClientInf();

	_ClientInf->Push(_SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);

	_Handle(_SocketContext->m_ClientSocket, _ClientInf);

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

bool FtpServer::_Handle(SOCKET _Socket, ClientInf * _ClientInf) {
	char *_Str;
	char *_Args;
	int _CmdType;
	bool _Cut;
	while (_Str = _ClientInf->Pop(), _Str) {
		printf("%s\n", _Str);
		_Args = _Str;
		_CmdType= _Dispatch(&_Args);

		switch (_CmdType) {
		case FTP_CMD_USER:
			break;
		case FTP_CMD_PASS:
			break;
		}
	}

	return false;
}

int FtpServer::_Dispatch(char **_Str) {
	char *p = *_Str;
	int _Ret = FTP_CMD_ERR;

	while (*p == ' ')++p;

	if (stricmp_n_1("USER", p) == 0) {
		_Ret = FTP_CMD_USER;
	} else if (stricmp_n_1("PASS", p) == 0) {
		_Ret = FTP_CMD_PASS;
	}

	*_Str += 4;

	while (**_Str == ' ')++(*_Str);

	return _Ret;
}