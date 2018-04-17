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

//TODO timeout

namespace network {

    //TODO #define T_PORT unsigned short

#define DEBUG 1
#define DEBUG_TRACE 0
#define DEBUG_LOG 1

#define FEATURE_RECV_ON_ACCEPT 0		//Recv Data on Accept.
    //This may arise a problem that when the client does not send data on connect,
    //the server won't get the event OnAccepted immediately until the client sends data.

#define DEFAULT_MAX_BUFFER_LEN 10240
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
    inline void _TRACE_PRINT(T&&... args) {
        //EnterCriticalSection(&CRITICAL_PRINT);

        SetColor(CC_GREEN);
        printf(std::forward<T>(args)...);
        SetColor(CC_WHITE);
        fflush(stdout);

        //LeaveCriticalSection(&CRITICAL_PRINT);
    }

    static unsigned int _DEBUG_TRACE = 0;
#endif

#if(DEBUG&DEBUG_LOG)
#define LOG _LOG

    template<class... T>
    inline void _LOG(CONSOLE_COLOR color, T&&... args) {
        SetColor(color);
        printf(std::forward<T>(args)...);
        SetColor(CC_WHITE);
        fflush(stdout);
    }
#endif

    //Call it after all network jobs are done.
    int Cleanup();

    template<typename dst_type, typename src_type>
    inline dst_type pointer_cast(src_type src) {
        return *static_cast<dst_type*>(static_cast<void*>(&src));
    }

    inline unsigned int _GetProcessorNum() {
        SYSTEM_INFO sys_inf;
        GetSystemInfo(&sys_inf);

        return sys_inf.dwNumberOfProcessors;
    }

    template<typename _Type>
    struct WORKER_PARAMS {
        _Type *m_instance;
        unsigned int m_thread_num;
    };

    enum SVR_OP {
        SVROP_NULL,
        SVROP_ACCEPTING,
        SVROP_RECVING,
        SVROP_SENDING,
        SVROP_CLOSING
    };

    struct SVR_SOCKET_CONTEXT {
        OVERLAPPED		m_OVERLAPPED;
        WSABUF			m_wsa_buf;
        char*			m_buffer;
        unsigned int	m_bytes_transferred;
        SVR_OP			m_op_type;
        SOCKET			m_client_sockid;
        SOCKADDR_IN		m_client_addr;
        void*			m_extra;

#if(DEBUG&DEBUG_TRACE)
        unsigned int _DEBUG_TRACE;
#endif

        SVR_SOCKET_CONTEXT(size_t max_buffer_len = DEFAULT_MAX_BUFFER_LEN);

        SVR_SOCKET_CONTEXT(SOCKET sockid, char *buffer, size_t buffer_len);

        SVR_SOCKET_CONTEXT(SOCKET sockid, const char *buffer, size_t buffer_len);

        void RESET_BUFFER();

        ~SVR_SOCKET_CONTEXT();
    };

    struct ServerConfig {
        /*	M:Mandatory
            O:Optional
            O[n]:Optional Set [n]
         */
        unsigned int m_port;
        unsigned int o_max_connect;
        unsigned int o_max_post_accept;
        unsigned int o_max_buffer_len;
        unsigned int o0_worker_threads_per_processor;
        unsigned int o0_worker_threads;

        ServerConfig(unsigned int port = 0, unsigned int max_connect = SOMAXCONN);
    };

    class Server {
    public:
        Server();

        Server(const ServerConfig &server_config);

        void set_config(const ServerConfig &server_config);

        bool start();

        bool send(SOCKET sockid, const char *buffer, size_t buffer_len);

        bool close_client(SOCKET sockid);

        bool close();

        virtual void on_accepted(SVR_SOCKET_CONTEXT *sock_ctx);

        virtual void on_recvd(SVR_SOCKET_CONTEXT *sock_ctx);

        virtual void on_sent(SVR_SOCKET_CONTEXT *sock_ctx);

        virtual void on_closed(SVR_SOCKET_CONTEXT *sock_ctx);

    protected:
        bool _start(unsigned int port, unsigned int max_connect);

        bool _init_sock(unsigned int port, unsigned int max_connect);

        bool _init_complition_port();

        bool _post_accept(SVR_SOCKET_CONTEXT *sock_ctx);

        bool _post_recv(SVR_SOCKET_CONTEXT *sock_ctx);

        bool _post_send(SVR_SOCKET_CONTEXT *sock_ctx);

        bool _do_accepted(SVR_SOCKET_CONTEXT *sock_ctx);

        bool _do_recvd(SVR_SOCKET_CONTEXT* sock_ctx);

        bool _do_sent(SVR_SOCKET_CONTEXT* sock_ctx);

        static bool _is_client_alive(SOCKET sockid);

        static DWORD WINAPI ServerWorkThread(LPVOID lpParam);

    protected:
        //TODO Event Register

        ServerConfig				m_server_config;

        SOCKET						m_sockid;

        HANDLE						m_completion_port;

        LPFN_ACCEPTEX				m_pAcceptEx;

        LPFN_GETACCEPTEXSOCKADDRS	m_pGetAcceptExSockAddrs;
    };

    struct ClientConfig {
        /*	M:Mandatory
            O:Optional
            O[n]:Optional Set [n]
            A[n]:Alternative Set [n]
        */
        unsigned int				o0_worker_threads_per_processor;
        unsigned int				o0_Worker_threads;

        ClientConfig();
    };

    enum CLT_OP {
        CLTOP_NULL,
        CLTOP_CONNECTING,
        CLTOP_SENDING,
        CLTOP_RECVING,
        CLTOP_CLOSING
    };

    struct CLT_SOCKET_CONTEXT {
        OVERLAPPED		m_OVERLAPPED;
        char*			m_buffer;
        WSABUF			m_wsa_buf;
        size_t	        m_bytes_transferred;
        CLT_OP          m_op_type;
        void*           m_extra;

#if(DEBUG&DEBUG_TRACE)
        int _DEBUG_TRACE;
#endif

        CLT_SOCKET_CONTEXT(size_t max_buffer_len = DEFAULT_MAX_BUFFER_LEN);

        CLT_SOCKET_CONTEXT(char *buffer, size_t buffer_len);

        CLT_SOCKET_CONTEXT(const char *buffer, size_t buffer_len);

        void RESET_BUFFER();

        ~CLT_SOCKET_CONTEXT();
    };

    class Client {
    public:
        Client();

        Client(const ClientConfig &client_config);

        void set_config(const ClientConfig &client_config);

        SOCKET connect(const char *addr, unsigned int port, unsigned int *local_port = NULL);

        bool send(char *buffer, size_t buffer_len);

        bool send(const char *buffer, size_t buffer_len);

        bool close();

        virtual void on_connected(CLT_SOCKET_CONTEXT *sock_ctx);

        virtual void on_sent(CLT_SOCKET_CONTEXT *sock_ctx);

        virtual void on_recvd(CLT_SOCKET_CONTEXT *sock_ctx);

        virtual void on_closed(CLT_SOCKET_CONTEXT *sock_ctx);

        ~Client();

    protected:
        int _init();

        SOCKET _init_sock(unsigned int *port);

        bool _init_completion_port();

        bool _post_connect(SOCKET sockid, unsigned long ip, unsigned int port);

        bool _post_send(CLT_SOCKET_CONTEXT *sock_ctx);

        bool _post_recv(CLT_SOCKET_CONTEXT *sock_ctx);

        bool _do_connected(CLT_SOCKET_CONTEXT *sock_ctx);

        bool _do_sent(CLT_SOCKET_CONTEXT *sock_ctx);

        bool _do_recvd(CLT_SOCKET_CONTEXT *sock_ctx);

        static bool _is_server_alive(SOCKET sockid);

        static DWORD WINAPI ClientWorkThread(LPVOID lpParam);

    protected:
        HANDLE				m_completion_port;
        SOCKET              m_sockid;
        ClientConfig		m_client_config;
        LPFN_CONNECTEX		m_pConnectEx;
    };
}

#endif //_NINI_NETWORK_IOCP_H_