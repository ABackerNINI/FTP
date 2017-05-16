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
#define DEBUG_TRACE 0
#define DEBUG_LOG 1

#define FEATURE_RECV_ON_ACCEPT 0		//Recv Data on Accept.
										//This may arise a problem that when the client does not send data on connect,
										//the server won't get the event OnAccepted immediately until the client sends data.

#define DEFAULT_MAX_CONNECT 30
#define DEFAULT_MAX_BUFFER_LEN 100
#define DEFAULT_MAX_POST_ACCEPT 10
#define DEFAULT_WORKER_THREADS_PER_PROCESSOR 2

namespace network {

#if(DEBUG&(DEBUG_TRACE|DEBUG_LOG))
	enum CONSOLE_COLOR {
		CC_BLACK = 0,
		CC_BLUE = 9,
		CC_GREEN = 10,
		CC_RED = 12,
		CC_PINK = 13,
		CC_YELLOW = 14,
		CC_WHITE = 15
	};

	inline void SetColor(int ForgC) {
		WORD wColor;

		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
			wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
			SetConsoleTextAttribute(hStdOut, wColor);
		}
	}
#endif

#if(DEBUG&DEBUG_TRACE)

	//static CRITICAL_SECTION CRITICAL_PRINT;

#define TRACE_PRINT _TRACE_PRINT
	template<class... T>
	inline void _TRACE_PRINT(T&&... _Args) {
		//EnterCriticalSection(&CRITICAL_PRINT);

		SetColor(CC_GREEN);
		printf(std::forward<T>(_Args)...);
		SetColor(CC_WHITE);
		fflush(stdout);

		//LeaveCriticalSection(&CRITICAL_PRINT);
	}

	static unsigned int _DEBUG_TRACE = 0;
#endif

#if(DEBUG&DEBUG_LOG)
#define LOG _LOG

	template<class... T>
	inline void _LOG(CONSOLE_COLOR _Color, T&&... _Args) {
		SetColor(_Color);
		printf(std::forward<T>(_Args)...);
		SetColor(CC_WHITE);
		fflush(stdout);
	}
#endif

	class Server;
	class Client;

	template<typename dst_type, typename src_type>
	dst_type pointer_cast(src_type src) {
		return *static_cast<dst_type*>(static_cast<void*>(&src));
	}

	inline unsigned int _GetProcessorNum() {
		SYSTEM_INFO SysInfo;
		GetSystemInfo(&SysInfo);

		return SysInfo.dwNumberOfProcessors;
	}

	template<typename _Type>
	struct WORKER_PARAMS {
		_Type *m_Instance;
		unsigned int m_ThreadNo;
	};

	enum SVR_OP {
		SVROP_ACCEPTING,
		SVROP_RECVING,
		SVROP_SENDING,
		SVROP_CLOSING,
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
			m_Extra(NULL) {
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
			m_OpType(SVR_OP::SVROP_UNDEFINED),
			m_Extra(NULL) {
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
			if (m_szBuffer)
				delete[] m_szBuffer;

#if(DEBUG&DEBUG_TRACE)
			TRACE_PRINT("PSC Dispose DEBUG_TRACE:%u Socket:%lld OP:%d\n", _DEBUG_TRACE, m_ClientSocket, m_OpType);
#endif
		}
	};

	struct ServerConfig {
		/*	M:Mandatory
			O:Optional
			O[n]:Optional Set [n]
		 */
		int M_Port;
		int O_MaxConnect;
		int O_MaxPostAccept;
		int O_MaxBufferLen;
		int O0_WorkerThreadsPerProcessor;
		int O0_WorkerThreads;

		ServerConfig(int _Port = -1) :
			M_Port(_Port),
			O_MaxConnect(DEFAULT_MAX_CONNECT),
			O_MaxPostAccept(DEFAULT_MAX_POST_ACCEPT),
			O_MaxBufferLen(DEFAULT_MAX_BUFFER_LEN),
			O0_WorkerThreadsPerProcessor(DEFAULT_WORKER_THREADS_PER_PROCESSOR),
			O0_WorkerThreads(-1) {
		}
	};

	enum SVR_EV {
		SVREV_ACCEPTED,
		SVREV_RECVD,
		SVREV_SENT,
		SVREV_CLOSED,
		SVREV_TIMEOUT
	};

	class Server {
	public:
		Server();

		Server(const ServerConfig &_ServerConfig);

		void SetConfig(const ServerConfig &_ServerConfig);

		bool Start();

		bool Send(SOCKET _Socket, const char *_SendBuffer, unsigned int _BufferLen);

		bool Close(SOCKET _Socket);

		//bool AddListenPort(int _Port, int _Max_Connect);

		//bool Connect();

		bool Stop();

		virtual void OnAccepted(SVR_SOCKET_CONTEXT *_SocketContext);

		virtual void OnRecvd(SVR_SOCKET_CONTEXT *_SocketContext);

		virtual void OnSent(SVR_SOCKET_CONTEXT *_SocketContext);

		virtual void OnClosed(SVR_SOCKET_CONTEXT *_SocketContext);

		//virtual void OnCommit(SVR_SOCKET_CONTEXT *_SocketContext);

	protected:
		bool _Start(int _Port, int _MaxConnect);

		void _Stop(SOCKET _Sockid);

		bool _InitSock(int _Port, unsigned int _Max_Connect);

		bool _InitComplitionPort();

		//bool _AddListenPort(int _Port, int _Max_Connect);

		bool _PostAccept(SVR_SOCKET_CONTEXT *_SocketContext);

		bool _PostRecv(SVR_SOCKET_CONTEXT *_SocketContext);

		bool _PostSend(SVR_SOCKET_CONTEXT *_SocketContext);

		bool _DoAccepted(SVR_SOCKET_CONTEXT *_SocketContext);

		bool _DoRecvd(SVR_SOCKET_CONTEXT* _SocketContext);

		bool _DoSent(SVR_SOCKET_CONTEXT* _SocketContext);

		static DWORD WINAPI ServerWorkThread(LPVOID _LpParam);

	protected:
		//TODO Event Register

		ServerConfig				m_ServerConfig;

		SOCKET						m_Socket;

		HANDLE						m_CompletionPort;

		LPFN_ACCEPTEX				m_pAcceptEx;

		LPFN_GETACCEPTEXSOCKADDRS	m_pGetAcceptExSockAddrs;
	};

	struct IP_PORT {
		const char *M_Ip;
		int M_Port;
	};

	struct ClientConfig {
		/*	M:Mandatory
			O:Optional
			O[n]:Optional Set [n]
			A[n]:Alternative Set [n]
		*/
		const char *	A0_Address;
		IP_PORT			A0_IpPort;
		int				O0_WorkerThreadsPerProcessor;
		int				O0_WorkerThreads;

		ClientConfig() :
			A0_Address(NULL),
			A0_IpPort({ NULL,-1 }),
			O0_WorkerThreadsPerProcessor(DEFAULT_WORKER_THREADS_PER_PROCESSOR),
			O0_WorkerThreads(-1) {
		}
	};

	enum CLT_OP {
		CLTOP_CONNECTING,
		CLTOP_SENDING,
		CLTOP_RECVING,
		CLTOP_CLOSING,
		CLTOP_UNDEFINED
	};

	enum CLT_EV {
		CLTEV_CONNECTED,
		CLTEV_SENT,
		CLTEV_RECVD,
		CLTEV_CLOSED,
		CLTEV_TIMEOUT
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

		CLT_SOCKET_CONTEXT(const char *_Buffer, unsigned int _BufferLen) :
			m_OpType(CLT_OP::CLTOP_UNDEFINED) {
			m_szBuffer = new char[_BufferLen + 1];
			memcpy(m_szBuffer, _Buffer, sizeof(char)*(_BufferLen + 1));
			m_wsaBuf.buf = m_szBuffer;
			m_wsaBuf.len = _BufferLen;//one for '\0'

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

		bool Send(const char *_SendBuffer, unsigned int _BufferLen);

		bool Close();

		virtual void OnConnected(CLT_SOCKET_CONTEXT *_SocketContext);

		virtual void OnSent(CLT_SOCKET_CONTEXT *_SocketContext);

		virtual void OnRecvd(CLT_SOCKET_CONTEXT *_SocketContext);

		virtual void OnClosed(CLT_SOCKET_CONTEXT *_SocketContext);

		~Client();

	protected:
		bool _InitSock();

		bool _InitCompletionPort();

		bool _PostConnect(unsigned long _Ip, int _Port);

		bool _PostSend(CLT_SOCKET_CONTEXT *_SocketContext);

		bool _PostRecv(CLT_SOCKET_CONTEXT *_SocketContext);

		bool _DoConnected(CLT_SOCKET_CONTEXT *_SocketContext);

		bool _DoSent(CLT_SOCKET_CONTEXT *_SocketContext);

		bool _DoRecvd(CLT_SOCKET_CONTEXT *_SocketContext);

		static DWORD WINAPI ClientWorkThread(LPVOID _LpParam);

	protected:
		SOCKET				m_Socket;

		HANDLE				m_CompletionPort;

		ClientConfig		m_ClientConfig;

		LPFN_CONNECTEX		m_ConnectEx;
	};
}
#endif //NINI_FTP_NETWORK_H