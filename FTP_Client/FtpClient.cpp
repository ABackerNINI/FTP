#include "FtpClient.h"

bool FtpClient::FtpSend(const char * _Buffer, int _Count) {

	bool _Sending = false;
	while (true) {
		if (!_Sending && (m_ClientStatus == CIS_RSP_HANDLED || m_ClientStatus == CIS_CONNECTED)) {
			if (Send(_Buffer, _Count) == false) {
				return false;
			}
			m_ClientStatus = CIS_SENDING;
			_Sending = true;
		} else if (_Sending && m_ClientStatus == CIS_RSP_HANDLED) {
			break;
		}
		Sleep(100);
	}

	return true;
}

void FtpClient::OnConnected(network::CLT_SOCKET_CONTEXT * _SocketContext) {
	m_ClientStatus = CIS_CONNECTED;

	if (_SocketContext->m_BytesTransferred > 0) {
		m_ClientInf.Push(_SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);

		_HandleResponse();
	}
}

void FtpClient::OnSent(network::CLT_SOCKET_CONTEXT * _SocketContext) {
	m_ClientStatus = CIS_SENT;
}

void FtpClient::OnRecvd(network::CLT_SOCKET_CONTEXT * _SocketContext) {
	m_ClientStatus = CIS_RECVD;

	m_ClientInf.Push(_SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);

	_HandleResponse();
}

void FtpClient::OnClosed(network::CLT_SOCKET_CONTEXT * _SocketContext) {
	m_ClientStatus = CIS_CLOSED;
}

void FtpClient::_HandleResponse() {
	char *_Str = m_ClientInf.Pop();
	if (_Str) {
		printf("\t%s\n", _Str);
	}

	m_ClientStatus = CIS_RSP_HANDLED;
	//TODO ERR CHECK
}
