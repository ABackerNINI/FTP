
#include "Network.h"

DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);
//DWORD WINAPI ServerSendThread(LPVOID IpParam);
//HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);

network::Server::Server() {

}

bool network::Server::Start(config_server *cs, callback_server callback) {
	return _Start(cs->port, cs->max_connect);
}

bool network::Server::Stop() {
	_Stop(m_Sockid);

	return true;
}

bool network::Server::_InitComplitionPort() {
	//Completion Port
	m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPort == NULL) {
		return false;
	}

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);

	unsigned int _WorkerThreadsNum = SysInfo.dwNumberOfProcessors * 2;
	HANDLE* _WorkerThreads = new HANDLE[_WorkerThreadsNum];
	memset(_WorkerThreads, 0, sizeof(HANDLE)*_WorkerThreadsNum);
	DWORD ThreadId;
	for (unsigned int i = 0; i < 2; ++i) {
		WORKER_PARAMS *_pWorkerParams = new WORKER_PARAMS();
		_pWorkerParams->m_Server = this;
		_pWorkerParams->m_ThreadNo = i;

		_WorkerThreads[i] = CreateThread(NULL, 0, ServerWorkThread, _pWorkerParams, 0, &ThreadId);
		if (_WorkerThreads[i] == NULL) {
			//TODO CloseHandle
			return false;
		}
	}

	return true;
}

bool network::Server::_InitSock(int _Port, unsigned int _Max_Connect) {
	//WSADATA
	WSADATA Wsadata;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &Wsadata) != 0) {
		return false;
	}
	if (LOBYTE(Wsadata.wVersion) != 2 || HIBYTE(Wsadata.wVersion) != 2) {
		WSACleanup();
		return false;
	}

	//SOCKET
	m_Sockid = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Sockid == INVALID_SOCKET) {
		WSACleanup();
		return false;
	}

	PER_SOCKET_CONTEXT *_pSocketContext = new PER_SOCKET_CONTEXT();
	if (CreateIoCompletionPort((HANDLE)m_Sockid, m_CompletionPort, (ULONG_PTR)_pSocketContext, 0) == NULL) {
		//TODO
		return false;
	}

	SOCKADDR_IN _Addr;
	memset(&_Addr, 0, sizeof(_Addr));
	_Addr.sin_family = AF_INET;
	_Addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	_Addr.sin_port = htons(_Port);

	//BIND
	if (bind(m_Sockid, (SOCKADDR*)&_Addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		closesocket(m_Sockid);
		WSACleanup();
		return false;
	}

	//LISTEN
	if (listen(m_Sockid, _Max_Connect) == SOCKET_ERROR) {
		closesocket(m_Sockid);
		WSACleanup();
		return false;
	}

	//pACCEPTEX pACCEPTEXSOCKADDRS
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		m_Sockid,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&m_pAcceptEx,
		sizeof(m_pAcceptEx),
		&dwBytes,
		NULL,
		NULL)) {
		return false;
	}

	if (SOCKET_ERROR == WSAIoctl(
		m_Sockid,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&m_pGetAcceptExSockAddrs,
		sizeof(m_pGetAcceptExSockAddrs),
		&dwBytes,
		NULL,
		NULL)) {
		return false;
	}

	for (int i = 0; i < 3; ++i) {
		_PostAccept(_pSocketContext);
	}

	return true;
}

bool network::Server::_Start(int port, int max_connect = SOMAXCONN) {

	_InitComplitionPort();

	_InitSock(port,max_connect);

	return true;
}

void network::Server::_Stop(SOCKET sockid) {
	closesocket(sockid);
	WSACleanup();
}

bool network::Server::_PostAccept(PER_SOCKET_CONTEXT *_pSocketContext) {
	DWORD _Flags = 0;

	ZeroMemory(&(_pSocketContext->m_Overlapped), sizeof(OVERLAPPED));

	_pSocketContext->m_OpType = ACCEPT;
	_pSocketContext->m_wsaBuf.buf = _pSocketContext->m_szBuffer;
	_pSocketContext->m_wsaBuf.len = BUFFER_LEN;
	_pSocketContext->m_ClientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	if (m_pAcceptEx(m_Sockid,
		_pSocketContext->m_ClientSocket,
		_pSocketContext->m_wsaBuf.buf,
		_pSocketContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&_Flags,
		&(_pSocketContext->m_Overlapped)) == false) {
		//TODO
		if (WSAGetLastError() != WSA_IO_PENDING) {
			return false;
		}
	}

	return true;
}

bool network::Server::_PostRecv(PER_SOCKET_CONTEXT *_pSocketContext) {
	// 初始化变量
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	_pSocketContext->m_OpType = RECV;

	// 初始化完成后，，投递WSARecv请求
	int nBytesRecv = WSARecv(_pSocketContext->m_ClientSocket, &(_pSocketContext->m_wsaBuf), 1, &dwBytes, &dwFlags, &(_pSocketContext->m_Overlapped), NULL);

	// 如果返回值错误，并且错误的代码并非是Pending的话，那就说明这个重叠请求失败了
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError())){
		return false;
	}
	return true;
}

bool network::Server::_DoAccept(PER_SOCKET_CONTEXT *_pSocketContext) {
	SOCKADDR_IN *_ClientAddr, _LocalAddr;
	int _ClientAddrLen = sizeof(SOCKADDR_IN), _LocalAddrLen = sizeof(SOCKADDR_IN);

	m_pGetAcceptExSockAddrs(
		_pSocketContext->m_wsaBuf.buf,
		_pSocketContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		(LPSOCKADDR*)&_LocalAddr, &_LocalAddrLen,
		(LPSOCKADDR*)&_ClientAddr, &_ClientAddrLen);

	PER_SOCKET_CONTEXT *_pNewSocketContex = new PER_SOCKET_CONTEXT();
	_pNewSocketContex->m_Socket = _pIoContext->m_sockAccept;
	//memcpy(&(pNewSocketContex->m_ClientAddr), _ClientAddr, _ClientAddrLen);
	if (CreateIoCompletionPort((HANDLE)_pNewSocketContex->m_Socket, m_CompletionPort, (ULONG_PTR)_pNewSocketContex, 0) == NULL) {
		//TODO
		return false;
	}

	PER_IO_CONTEXT *_pNewIoContext = new PER_IO_CONTEXT();
	_pNewIoContext->m_OpType = RECV;
	_pNewIoContext->m_sockAccept = _pNewSocketContex->m_Socket;
	_pNewIoContext->m_wsaBuf.buf = _pNewIoContext->m_szBuffer;
	_pNewIoContext->m_wsaBuf.len = BUFFER_LEN;
	ZeroMemory(&(_pNewIoContext->m_Overlapped), sizeof(OVERLAPPED));

	_DoSend(_pSocketContext, _pIoContext);

	_PostRecv(_pIoContext);

	return _PostAccept(_pNewSocketContex, _pNewIoContext);
}

bool network::Server::_DoRecv(PER_SOCKET_CONTEXT *_pSocketContext) {
	//SOCKADDR_IN* ClientAddr = &_pSocketContext->m_ClientAddr;

	_pSocketContext->m_OpType = SEND;

	return true;

	//return _PostRecv(_pIoContext);
}

bool network::Server::_DoSend(PER_SOCKET_CONTEXT *_pSocketContext) {
	//WSASend();

	send(_pSocketContext->m_ClientSocket, "jfoaiwe", 8, 0);

	closesocket(_pSocketContext->m_ClientSocket);

	return true;
}

DWORD WINAPI network::Server::ServerWorkThread(LPVOID IpParam) {
	WORKER_PARAMS *_WorkerParams = (WORKER_PARAMS*)IpParam;

	DWORD _BytesTransferred;
	OVERLAPPED *_Overlapped;
	PER_SOCKET_CONTEXT *_pSocketContext;
	PER_IO_CONTEXT *_pIoContext;

	while (true) {
		_BytesTransferred = 0;

		if (GetQueuedCompletionStatus(_WorkerParams->m_Server->m_CompletionPort, &_BytesTransferred, (PULONG_PTR)&_pSocketContext, (LPOVERLAPPED*)&_Overlapped, INFINITE) == false) {
			continue;
		}

		_pIoContext = CONTAINING_RECORD(_Overlapped, PER_IO_CONTEXT, m_Overlapped);

		if (_BytesTransferred == 0 && (_pIoContext->m_OpType == RECV || _pIoContext->m_OpType == SEND)) {
			closesocket(_pSocketContext->m_Socket);
			//TODO
			continue;
		}

		switch (_pIoContext->m_OpType) {
		case ACCEPT:
			_WorkerParams->m_Server->_DoAccept(_pSocketContext, _pIoContext);
			break;
		case RECV:
			_WorkerParams->m_Server->_DoRecv(_pSocketContext,_pIoContext);
			break;
		case SEND:
			_WorkerParams->m_Server->_DoSend(_pSocketContext, _pIoContext);
			break;
		default:
			break;
		}
	}

	return 0;
}
