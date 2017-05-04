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

#define DEFAULT_MAX_CONNECT 30
#define DEFAULT_MAX_BUFFER_LEN 100
#define DEFAULT_MAX_POST_ACCEPT 10
#define DEFAULT_WORKER_THREADS_PER_PROCESSOR 2

namespace network {

#if(DEBUG&DEBUG_TRACE)

	static CRITICAL_SECTION CRITICAL_PRINT;

	inline void SetColor(int ForgC) {
		WORD wColor;

		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
			wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
			SetConsoleTextAttribute(hStdOut, wColor);
		}
	}

#define TRACE_PRINT _TRACE_PRINT
	template<class... T>
	inline void _TRACE_PRINT(T&&... _Args) {
		EnterCriticalSection(&CRITICAL_PRINT);

		SetColor(14);
		printf(std::forward<T>(_Args)...);
		SetColor(15);
		fflush(stdout);

		LeaveCriticalSection(&CRITICAL_PRINT);
	}

	static unsigned int _DEBUG_TRACE = 0;
#endif

	class Server;
	class Client;

	template<typename _Type>
	struct WORKER_PARAMS {
		_Type *m_Instance;
		unsigned int m_ThreadNo;
	};

	enum SVR_OP {
		SVROP_ACCEPTED,
		SVROP_RECVING,
		SVROP_SENDING,
		SVROP_CLOSED,
		SVROP_UNDEFINED
	};

	struct SVR_SOCKET_CONTEXT {
		OVERLAPPED		m_Overlapped;
		SOCKET			m_ClientSocket;
		//SOCKADDR_IN		m_ClientAddr;
		WSABUF			m_wsaBuf;
		char*			m_szBuffer;
		unsigned int	m_BytesTransferred;
		SVR_OP			m_OpType;
		void*			m_Extra;

#if(DEBUG&DEBUG_TRACE)
		unsigned int _DEBUG_TRACE;
#endif

		SVR_SOCKET_CONTEXT(SOCKET _Socket, const char *_Buffer, unsigned int _BufferLen) :
			m_ClientSocket(_Socket),
			m_OpType(SVR_OP::SVROP_UNDEFINED),
			m_Extra(NULL){
			m_szBuffer = new char[_BufferLen + 1];
			memcpy(m_szBuffer, _Buffer, sizeof(char)*(_BufferLen + 1));
			m_wsaBuf.buf = m_szBuffer;
			m_wsaBuf.len = _BufferLen;//one for '\0'

			memset(&m_Overlapped, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
			_DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
		}

		SVR_SOCKET_CONTEXT(int _MaxBufferLen = DEFAULT_MAX_BUFFER_LEN) :
			m_ClientSocket(INVALID_SOCKET),
			m_OpType(SVR_OP::SVROP_UNDEFINED) {
			m_szBuffer = new char[_MaxBufferLen];//TODO user-defined(upper layer) buffer len
			m_wsaBuf.buf = m_szBuffer;
			m_wsaBuf.len = _MaxBufferLen - 1;//one for '\0'

			memset(&m_Overlapped, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
			_DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
		}

		void RESET_BUFFER() {
			m_szBuffer[0] = '\0';
			m_BytesTransferred = 0;
		}

		~SVR_SOCKET_CONTEXT() {
			//if (m_ClientSocket != INVALID_SOCKET)
			//	closesocket(m_ClientSocket);

			if (m_szBuffer)
				delete[] m_szBuffer;

#if(DEBUG&DEBUG_TRACE)
			TRACE_PRINT("PSC Dispose DEBUG_TRACE:%u Socket:%lld OP:%d\n", _DEBUG_TRACE, m_ClientSocket, m_OpType);
#endif
		}
	};

	typedef void(*ServerCallback)(SVR_SOCKET_CONTEXT *_SocketContext, int _Ev, void *_Data);
	typedef void(*CommitHandler)(SVR_SOCKET_CONTEXT *_SocketContext, ServerCallback _ServerCallback);

	struct ServerConfig {
		/* M:Mandatory
		   O:Optional
		 */
		int M_Port;
		int O_MaxConnect;
		int O_MaxPostAccept;
		int O_MaxBufferLen;
		int O_WorkerThreadsPerProcessor;
		ServerCallback M_ServerCallback;
		CommitHandler O_CommitHandler;

		ServerConfig(int _Port = -1, ServerCallback _ServerCallback = NULL, CommitHandler _CommitHandler = NULL) :
			M_Port(_Port),
			O_MaxConnect(DEFAULT_MAX_CONNECT),
			O_MaxPostAccept(DEFAULT_MAX_POST_ACCEPT),
			O_MaxBufferLen(DEFAULT_MAX_BUFFER_LEN),
			O_WorkerThreadsPerProcessor(DEFAULT_WORKER_THREADS_PER_PROCESSOR),
			M_ServerCallback(_ServerCallback),
			O_CommitHandler(_CommitHandler) {
		}
	};

	enum SVR_EV {
		SVREV_ACCEPTED,
		SVREV_RECVD,
		SVREV_SENT,
		SVREV_CLOSED,
		SVREV_COMMIT
	};

	class Server {
	public:
		Server();

		Server(const ServerConfig &_ServerConfig);

		void SetConfig(const ServerConfig &_ServerConfig);

		bool Start();

		static bool Send(SOCKET _Socket, const char *_SendBuffer, unsigned int _BufferLen);

		static bool Close(SOCKET _Socket);

		bool Stop();

	protected:
		bool _Start(int _Port, int _MaxConnect);

		void _Stop(SOCKET _Sockid);

		bool _InitSock(int _Port, unsigned int _Max_Connect);

		bool _InitComplitionPort();

		bool _PostAccept(SVR_SOCKET_CONTEXT *_SocketContext);

		bool _PostRecv(SVR_SOCKET_CONTEXT *_SocketContext);

		bool _PostSend(SVR_SOCKET_CONTEXT *_SocketContext);

		bool _DoAccept(SVR_SOCKET_CONTEXT *_SocketContext);

		bool _DoRecv(SVR_SOCKET_CONTEXT* _SocketContext);

		bool _DoSend(SVR_SOCKET_CONTEXT* _SocketContext);

		//bool _DoClose(PER_SOCKET_CONTEXT* _SocketContext);

		void _Commit(SVR_SOCKET_CONTEXT* _SocketContext);

		void _Call(SOCKET _Socket, int _Ev, void *_Data);

		static unsigned int _GetProcessorNum();

		static DWORD WINAPI ServerWorkThread(LPVOID _LpParam);

	protected:
		//TODO Event Register

		ServerConfig m_ServerConfig;

		SOCKET m_Socket;

		HANDLE m_CompletionPort;

		LPFN_ACCEPTEX m_pAcceptEx;

		LPFN_GETACCEPTEXSOCKADDRS m_pGetAcceptExSockAddrs;
	};

	struct ClientConfig {

	};

	enum CLT_OP {
		CLTOP_CONNECTED,
		CLTOP_SENDING,
		CLTOP_RECVING,
		CLTOP_CLOSED,
		CLTOP_UNDEFINED
	};

	struct CLT_SOCKET_CONTEXT {
		OVERLAPPED		m_Overlapped;
		WSABUF			m_wsaBuf;
		char*			m_szBuffer;
		unsigned int	m_BytesTransferred;
		CLT_OP  m_OpType;

#if(DEBUG&DEBUG_TRACE)
		int _DEBUG_TRACE;
#endif

		CLT_SOCKET_CONTEXT(int _MaxBufferLen = DEFAULT_MAX_BUFFER_LEN) {
			m_szBuffer = new char[_MaxBufferLen];
			m_wsaBuf.buf = m_szBuffer;
			m_wsaBuf.len = _MaxBufferLen - 1;//one for '\0'
			m_OpType = CLT_OP::CLTOP_UNDEFINED;

			memset(&m_Overlapped, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
			_DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
		}

		void RESET_BUFFER() {
			m_szBuffer[0] = '\0';
			m_BytesTransferred = 0;
		}

		~CLT_SOCKET_CONTEXT() {
			if (m_szBuffer)
				delete[] m_szBuffer;

#if(DEBUG&DEBUG_TRACE)
			TRACE_PRINT("PSC Dispose DEBUG_TRACE:%d\n", _DEBUG_TRACE);
#endif
		}
	};

	class Client {
	public:
		Client();

		Client(const ClientConfig &_ClientConfig);

		void SetConfig(const ClientConfig &_ClientConfig);

		bool Connect();

		bool Send();

		bool Disconnect();

		~Client();

	protected:
		bool _InitSock();

		bool _InitCompletionPort();

		bool _PostConnect();

		bool _PostSend();

		bool _Disconnect();

		bool _DoConnect();

		bool _DoSend();

		bool _DoRecv();

		static DWORD WINAPI ServerWorkThread(LPVOID _LpParam);

	protected:
		SOCKET m_Socket;

		HANDLE m_CompletionPort;

		LPFN_CONNECTEX m_ConnectEx;

		ClientConfig m_ClientConfig;
	};
}
#endif //NINI_FTP_NETWORK_H