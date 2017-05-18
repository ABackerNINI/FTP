#include "FtpClient.h"

/*-----------------------------------------------------------FtpClientData Section-----------------------------------------------------------*/

FtpClientData::FtpClientData() {
}

FtpClientData::FtpClientData(const FtpClientConfig & _FtpClientConfig) :network::Client(_FtpClientConfig), m_Port(_FtpClientConfig.M_Port) {
}

void FtpClientData::SetConfig(const FtpClientConfig & _FtpClientConfig) {
	network::Client::SetConfig(_FtpClientConfig);

	m_Port = _FtpClientConfig.M_Port;
}

bool FtpClientData::FtpSend(const char * _Buffer, int _Count) {
	return false;
}

void FtpClientData::OnConnected(network::CLT_SOCKET_CONTEXT * _SocketContext) {
}

void FtpClientData::OnSent(network::CLT_SOCKET_CONTEXT * _SocketContext) {
}

void FtpClientData::OnRecvd(network::CLT_SOCKET_CONTEXT * _SocketContext) {
}

void FtpClientData::OnClosed(network::CLT_SOCKET_CONTEXT * _SocketContext) {
}

void FtpClientData::_HandleResponse() {
}

/*-----------------------------------------------------------FtpClient Section-----------------------------------------------------------*/

FtpClient::FtpClient() {
}

FtpClient::FtpClient(const FtpClientConfig & _FtpClientConfig) :network::Client(_FtpClientConfig), m_Port(_FtpClientConfig.M_Port), m_FtpClientData(_FtpClientConfig) {
}

void FtpClient::SetConfig(const FtpClientConfig & _FtpClientConfig) {
	network::Client::SetConfig(_FtpClientConfig);

	m_Port = _FtpClientConfig.M_Port;
}

bool FtpClient::FtpConnect() {
	if (Connect()) {
		for (int i = 0; i < 10; ++i) {
			
			if (m_ClientStatus == CLIENT_IO_STATUS::CIS_RSP_HANDLED) {
				printf("\n");

				return true;
			} else if (i == 9) {
				printf("\n");

				break;
			}
			printf(".");
			Sleep(500);
		}
	} else {
	}

	printf("Connect Error,Please Check Your Network.\n");

	return false;
}

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

CLIENT_IO_STATUS FtpClient::GetIoStatus() {
	return m_ClientStatus;
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
	printf("OnClosed\n");
}

void FtpClient::_HandleResponse() {
	char *_Str;

	while (_Str = m_ClientInf.Pop(), _Str) {
		printf("\t%s\n", _Str);
		fflush(stdout);

		m_ClientStatus = CIS_RSP_HANDLED;
	}

	//TODO ERR CHECK
}
