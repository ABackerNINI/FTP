#pragma once

#ifndef NINI_FTP_NETWORK_H
#define NINI_FTP_NETWORK_H

#include <stdio.h>
#include <WinSock2.h>
#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <list>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Kernel32.lib")

#define BUFFER_LEN 1024


namespace network {
	enum OPERATION_TYPE {
		ACCEPT, RECV, SEND, UNDEFINED
	};

	//struct PER_IO_CONTEXT {
	//	SOCKET       m_sockAccept;          // 这个I/O操作所使用的Socket，每个连接的都是一样的  
	//};

	struct PER_SOCKET_CONTEXT {
		OVERLAPPED		m_Overlapped;
		SOCKET			m_ClientSocket;
		SOCKADDR_IN		m_ClientAddr;
		WSABUF			m_wsaBuf;
		char			m_szBuffer[BUFFER_LEN];
		OPERATION_TYPE  m_OpType;
	};

	class Server;

	struct WORKER_PARAMS {
		Server *m_Server;
		unsigned int m_ThreadNo;
	};
	struct config_server {
		int port;
		int max_connect;
	};

	typedef void(*callback_server)(int ev, void *data);

	class Server {
	public:
		Server();

		bool Start(config_server *cs, callback_server callback);

		bool Stop();

	protected:
		bool _Start(int port, int max_connect);

		void _Stop(SOCKET sockid);

		bool _InitSock(int _Port, unsigned int _Max_Connect);

		bool _InitComplitionPort();

		bool _PostAccept(PER_SOCKET_CONTEXT *_pSocketContext);

		bool _PostRecv(PER_SOCKET_CONTEXT *_pSocketContext);

		bool _DoAccept(PER_SOCKET_CONTEXT *_pSocketContext);

		bool _DoRecv(PER_SOCKET_CONTEXT* _pSocketContext);

		bool _DoSend(PER_SOCKET_CONTEXT* _pSocketContext);

		static DWORD WINAPI ServerWorkThread(LPVOID IpParam);

	private:
		SOCKET m_Sockid;

		HANDLE m_CompletionPort;

		LPFN_ACCEPTEX m_pAcceptEx;

		LPFN_GETACCEPTEXSOCKADDRS m_pGetAcceptExSockAddrs;
	};

	class Client {
	public:
		bool Send();

	private:

	};
}
#endif //NINI_FTP_NETWORK_H