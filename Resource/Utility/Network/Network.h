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

	typedef struct _PER_IO_CONTEXT {
		OVERLAPPED   m_Overlapped;          // ÿһ���ص�I/O���������Ҫ��һ��                
		SOCKET       m_sockAccept;          // ���I/O������ʹ�õ�Socket��ÿ�����ӵĶ���һ����  
		WSABUF       m_wsaBuf;              // �洢���ݵĻ��������������ص��������ݲ����ģ�����WSABUF���滹�ὲ  
		char         m_szBuffer[BUFFER_LEN]; // ��ӦWSABUF��Ļ�����  
		OPERATION_TYPE  m_OpType;               // ��־����ص�I/O��������ʲô�ģ�����Accept/Recv��  

	} PER_IO_CONTEXT, *PPER_IO_CONTEXT;

	typedef struct _PER_SOCKET_CONTEXT {
		SOCKET                   m_Socket;              // ÿһ���ͻ������ӵ�Socket  
		SOCKADDR_IN              m_ClientAddr;          // ����ͻ��˵ĵ�ַ  
		std::list<_PER_IO_CONTEXT*>  m_arrayIoContext;   // ���飬���пͻ���IO�����Ĳ�����  
														 // Ҳ����˵����ÿһ���ͻ���Socket  
														 // �ǿ���������ͬʱͶ�ݶ��IO�����  
	} PER_SOCKET_CONTEXT, *PPER_SOCKET_CONTEXT;

	class Server;

	typedef struct _WORKER_PARAMS {
		Server *m_Server;
		unsigned int m_ThreadNo;
	}WORKER_PARAMS;
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

		bool _PostAccept(_PER_IO_CONTEXT *_PerIoContext);

		bool _PostRecv(PER_IO_CONTEXT *_pIoContext);

		bool _DoAccept(PER_SOCKET_CONTEXT *_pSocketContext, PER_IO_CONTEXT *_pIoContext);

		bool _DoRecv(PER_SOCKET_CONTEXT* _pSocketContext, PER_IO_CONTEXT* _pIoContext);

		bool _DoSend(PER_SOCKET_CONTEXT* _pSocketContext, PER_IO_CONTEXT* _pIoContext);

		static DWORD WINAPI ServerWorkThread(LPVOID IpParam);

	private:
		SOCKET m_Sockid;

		HANDLE m_CompletionPort;

		PER_SOCKET_CONTEXT *m_pListenContext;

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