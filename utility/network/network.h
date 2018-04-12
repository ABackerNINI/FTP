#pragma once

#ifndef _NINI_NETWORK_IOCP_H_
#define _NINI_NETWORK_IOCP_H_

#include <stdio.h>
#include <iostream>
#include <WinSock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <list>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Kernel32.lib")

namespace network {

#define DEBUG 1
#define DEBUG_TRACE 1
#define DEBUG_LOG 1

#define FEATURE_RECV_ON_ACCEPT 0		//Recv Data on Accept.
    //This may arise a problem that when the client does not send data on connect,
    //the server won't get the event OnAccepted immediately until the client sends data.

#define DEFAULT_MAX_BUFFER_LEN 100
#define DEFAULT_MAX_POST_ACCEPT 10
#define DEFAULT_WORKER_THREADS_PER_PROCESSOR 2


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

    template<typename dst_type, typename src_type>
    inline dst_type pointer_cast(src_type src) {
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
        SVROP_NULL,
        SVROP_ACCEPTING,
        SVROP_RECVING,
        SVROP_SENDING,
        SVROP_CLOSING
    };

    struct SVR_SOCKET_CONTEXT {
        OVERLAPPED		m_Overlapped;
        SOCKET			m_ClientSocket;
        SOCKADDR_IN		m_ClientAddr;
        WSABUF			m_wsaBuf;
        char*			m_szBuffer;
        unsigned int	m_BytesTransferred;
        SVR_OP			m_OpType;
        void*			m_Extra;

#if(DEBUG&DEBUG_TRACE)
        unsigned int _DEBUG_TRACE;
#endif

        SVR_SOCKET_CONTEXT(SOCKET _Socket, const char *_Buffer, size_t _BufferLen);

        SVR_SOCKET_CONTEXT(size_t _MaxBufferLen = DEFAULT_MAX_BUFFER_LEN);

        void RESET_BUFFER();

        ~SVR_SOCKET_CONTEXT();
    };

    struct ServerConfig {
        /*	M:Mandatory
            O:Optional
            O[n]:Optional Set [n]
         */
        unsigned int M_Port;
        unsigned int O_MaxConnect;
        unsigned int O_MaxPostAccept;
        unsigned int O_MaxBufferLen;
        unsigned int O0_WorkerThreadsPerProcessor;
        unsigned int O0_WorkerThreads;

        ServerConfig(unsigned int _Port = 0, unsigned int _MaxConnect = SOMAXCONN);
    };

    enum SVR_EV {
        SVREV_ACCEPTED,
        SVREV_RECEIVED,
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

        bool Send(SOCKET _Socket, const char *_SendBuffer, size_t _BufferLen);

        bool Close(SOCKET _Socket);

        bool Stop();

        virtual void OnAccepted(SVR_SOCKET_CONTEXT *_SocketContext);

        virtual void OnRecvd(SVR_SOCKET_CONTEXT *_SocketContext);

        virtual void OnSent(SVR_SOCKET_CONTEXT *_SocketContext);

        virtual void OnClosed(SVR_SOCKET_CONTEXT *_SocketContext);

    protected:
        bool _Start(unsigned int _Port, unsigned int _MaxConnect);

        void _Stop(SOCKET _Sockid);

        bool _InitSock(unsigned int _Port, unsigned int _MaxConnect);

        bool _InitComplitionPort();

        bool _PostAccept(SVR_SOCKET_CONTEXT *_SocketContext);

        bool _PostRecv(SVR_SOCKET_CONTEXT *_SocketContext);

        bool _PostSend(SVR_SOCKET_CONTEXT *_SocketContext);

        bool _DoAccepted(SVR_SOCKET_CONTEXT *_SocketContext);

        bool _DoRecvd(SVR_SOCKET_CONTEXT* _SocketContext);

        bool _DoSent(SVR_SOCKET_CONTEXT* _SocketContext);

        static bool _IsClientAlive(SOCKET _Sockid);

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
        const char *	M0_Ip_String;
        unsigned long	M0_Ip_ULong;
        unsigned int	M_Port;

        IP_PORT();
    };

    struct ClientConfig {
        /*	M:Mandatory
            O:Optional
            O[n]:Optional Set [n]
            A[n]:Alternative Set [n]
        */
        unsigned int				O0_WorkerThreadsPerProcessor;
        unsigned int				O0_WorkerThreads;

        ClientConfig();
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
        size_t	        m_BytesTransferred;
        SOCKET			m_Socket;
        CLT_OP          m_OpType;
        void*           m_Extra;

#if(DEBUG&DEBUG_TRACE)
        int _DEBUG_TRACE;
#endif

        CLT_SOCKET_CONTEXT(size_t _MaxBufferLen = DEFAULT_MAX_BUFFER_LEN);

        CLT_SOCKET_CONTEXT(const char *_Buffer, size_t _BufferLen);

        void RESET_BUFFER();

        ~CLT_SOCKET_CONTEXT();
    };

    class Client {
    public:
        Client();

        Client(const ClientConfig &_ClientConfig);

        void SetConfig(const ClientConfig &_ClientConfig);

        SOCKET Connect(const IP_PORT *_IpPort, unsigned int *_LocalPort = NULL);

        SOCKET Connect(const char *_Address, unsigned int *_LocalPort);

        bool Send(SOCKET _Sockid, const char *_SendBuffer, size_t _BufferLen);

        bool Close(SOCKET _Sockid);

        virtual void OnConnected(CLT_SOCKET_CONTEXT *_SocketContext);

        virtual void OnSent(CLT_SOCKET_CONTEXT *_SocketContext);

        virtual void OnRecvd(CLT_SOCKET_CONTEXT *_SocketContext);

        virtual void OnClosed(CLT_SOCKET_CONTEXT *_SocketContext);

        ~Client();

    protected:
        int _Init();

        SOCKET _InitSock(unsigned int *_Port);

        bool _InitCompletionPort();

        bool _PostConnect(SOCKET _Sockid, unsigned long _Ip, unsigned int _Port);

        bool _PostSend(CLT_SOCKET_CONTEXT *_SocketContext);

        bool _PostRecv(CLT_SOCKET_CONTEXT *_SocketContext);

        bool _DoConnected(CLT_SOCKET_CONTEXT *_SocketContext);

        bool _DoSent(CLT_SOCKET_CONTEXT *_SocketContext);

        bool _DoRecvd(CLT_SOCKET_CONTEXT *_SocketContext);

        static DWORD WINAPI ClientWorkThread(LPVOID _LpParam);

        static bool _IsServerAlive(SOCKET _Sockid);

    protected:
        HANDLE				m_CompletionPort;

        ClientConfig		m_ClientConfig;

        LPFN_CONNECTEX		m_ConnectEx;
    };
}

#endif //_NINI_NETWORK_IOCP_H_