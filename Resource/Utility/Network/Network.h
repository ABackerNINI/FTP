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

#define DEBUG 1
#define DEBUG_TRACE 1

#define MAX_BUFFER_LEN 100
#define MAX_POST_ACCEPT 100
#define WORKER_THREADS_PER_PROCESSOR 2

namespace network {

#if(DEBUG&DEBUG_TRACE)

	static CRITICAL_SECTION CRITICAL_PRINT;

#define TRACE_PRINT _TRACE_PRINT
	inline void SetColor(int ForgC) {
		WORD wColor;
		//We will need this handle to get the current background attribute
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		//We use csbi for the wAttributes word.
		if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
			//Mask out all but the background attribute, and add in the forgournd color
			wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
			SetConsoleTextAttribute(hStdOut, wColor);
		}
	}

	template<class... T>
	void _TRACE_PRINT(T&&... _Args) {
		EnterCriticalSection(&CRITICAL_PRINT);

		SetColor(14);
		printf(std::forward<T>(_Args)...);
		SetColor(15);

		LeaveCriticalSection(&CRITICAL_PRINT);
	}

	static int _DEBUG_TRACE = 0;
#endif

	enum OPERATION_TYPE {
		ACCEPTED, RECVING, SENDING, CLOSED, UNDEFINED
	};

	struct PER_SOCKET_CONTEXT {
		OVERLAPPED		m_Overlapped;
		SOCKET			m_ClientSocket;
		SOCKADDR_IN		m_ClientAddr;
		WSABUF			m_wsaBuf;
		char*			m_szBuffer;
		unsigned int	m_BytesTransferred;
		OPERATION_TYPE  m_OpType;

#if(DEBUG&DEBUG_TRACE)
		int _DEBUG_TRACE;
#endif

		PER_SOCKET_CONTEXT() {
			m_ClientSocket = INVALID_SOCKET;
			m_szBuffer = new char[MAX_BUFFER_LEN];
			m_wsaBuf.buf = m_szBuffer;
			m_wsaBuf.len = MAX_BUFFER_LEN - 1;//one for '\0'
			m_OpType = UNDEFINED;

			memset(&m_Overlapped, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
			_DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
		}

		void RESET_BUFFER() {
			m_szBuffer[0] = '\0';
			m_BytesTransferred = 0;
			//m_OpType = UNDEFINED;
		}

		~PER_SOCKET_CONTEXT() {
			closesocket(m_ClientSocket);
			delete[] m_szBuffer;

#if(DEBUG&DEBUG_TRACE)
			TRACE_PRINT("PSC Dispose Socket:%lld DEBUG_TRACE:%d\n", m_ClientSocket, _DEBUG_TRACE);
#endif
		}
	};

	struct LISTEN_CONTEXT {

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

		bool _DoClose(PER_SOCKET_CONTEXT* _pSocketContext);

		void _Commit(PER_SOCKET_CONTEXT* _pSocketContext);

		unsigned int _GetProcessorNum();

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