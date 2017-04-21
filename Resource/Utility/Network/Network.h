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

enum OPERATION_TYPE {
	ACCEPT, RECV, SEND, UNDEFINED
};

typedef struct _PER_IO_CONTEXT {
	OVERLAPPED   m_Overlapped;          // 每一个重叠I/O网络操作都要有一个                
	SOCKET       m_sockAccept;          // 这个I/O操作所使用的Socket，每个连接的都是一样的  
	WSABUF       m_wsaBuf;              // 存储数据的缓冲区，用来给重叠操作传递参数的，关于WSABUF后面还会讲  
	char         m_szBuffer[BUFFER_LEN]; // 对应WSABUF里的缓冲区  
	OPERATION_TYPE  m_OpType;               // 标志这个重叠I/O操作是做什么的，例如Accept/Recv等  

} PER_IO_CONTEXT, *PPER_IO_CONTEXT;

typedef struct _PER_SOCKET_CONTEXT {
	SOCKET                   m_Socket;              // 每一个客户端连接的Socket  
	SOCKADDR_IN              m_ClientAddr;          // 这个客户端的地址  
	std::list<_PER_IO_CONTEXT*>  m_arrayIoContext;   // 数组，所有客户端IO操作的参数，  
													 // 也就是说对于每一个客户端Socket  
													 // 是可以在上面同时投递多个IO请求的  
} PER_SOCKET_CONTEXT, *PPER_SOCKET_CONTEXT;

namespace network {
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

		bool _InitComplitionPort(HANDLE *_CompletionPort);

		bool _PostAccept(_PER_IO_CONTEXT *_PerIoContext);

		bool _PostRecv();

		bool _DoAccept();

		bool _DoRecv();

	private:
		SOCKET m_Sockid;

		LPFN_ACCEPTEX m_pAcceptEx;

		LPFN_GETACCEPTEXSOCKADDRS m_pAcceptExSockAddrs;
	};

	class Client {
	public:
		bool Send();

	private:

	};
}
#endif //NINI_FTP_NETWORK_H