#pragma once

#ifndef _NINI_FTP_DTP_H_
#define _NINI_FTP_DTP_H_

#include "../../../utility/file/file.h"
#include "../../../utility/network/network.h"

#define DEBUG 1
#define DEBUG_ERROR_CHECK 1
#define DEFAULT_BUFFER_LEN 1024

#define FTP_DTP_TYPE_NULL           (-1)
#define FTP_DTP_TYPE_PASSIVE_SEND   0
#define FTP_DTP_TYPE_PASSIVE_RECV   1
#define FTP_DTP_TYPE_ACTIVE_SEND    2
#define FTP_DTP_TYPE_ACTIVE_RECV    3

namespace ftp_dtp {
    static network::ServerConfig *_server_config;

    class _ftp_dtp_passive_send : public network::Server {
    public:
        _ftp_dtp_passive_send(const char *fpath, unsigned int *local_port, HANDLE completion_port_2, SOCKET sockid_2);

        bool abort();

    protected:
        virtual void event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) override;

        void _on_connected(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx);

    protected:
        HANDLE              m_completion_port_2;//post back
        SOCKET              m_sockid_2;//post back

        file::File          m_file;

        size_t              m_bytes_transfered;
    };

    class _ftp_dtp_passive_recv : public network::Server {
    public:
        _ftp_dtp_passive_recv(const char *fpath, unsigned int *local_port, HANDLE completion_port_2, SOCKET sockid_2);

        bool abort();

    protected:
        virtual void event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) override;

        void _on_connected(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx);

    protected:
        HANDLE              m_completion_port_2;//post back
        SOCKET              m_sockid_2;//post back

        file::File          m_file;

        size_t              m_bytes_transfered;
    };

    class _ftp_dtp_active_send : public network::Client {
    public:
        _ftp_dtp_active_send(const char *fpath, const char *addr, unsigned int port, HANDLE completion_port_2, SOCKET sockid_2);

        bool abort();

    protected:
        virtual void event_handler(network::CLT_SOCKET_CONTEXT *sock_ctx, int ev) override;

        void _on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx);

    protected:
        HANDLE              m_completion_port_2;//post back
        SOCKET              m_sockid_2;//post back

        file::File          m_file;

        size_t              m_bytes_transfered;
    };

    class _ftp_dtp_active_recv : public network::Client {
    public:
        _ftp_dtp_active_recv(const char *fpath, const char *addr, unsigned int port, HANDLE completion_port_2, SOCKET sockid_2);

        bool abort();

    protected:
        virtual void event_handler(network::CLT_SOCKET_CONTEXT *sock_ctx, int ev) override;

        void _on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx);

    protected:
        HANDLE              m_completion_port_2;//post back
        SOCKET              m_sockid_2;//post back

        file::File          m_file;

        size_t              m_bytes_transfered;
    };

    class FtpDtp {
    public:
        FtpDtp();

        bool start(int dtp_type, const char *fpath, const char *addr, unsigned int port, unsigned int *local_port = NULL, HANDLE completion_port_2 = NULL, SOCKET sockid_2 = INVALID_SOCKET);

        bool abort();

        bool close();

        ~FtpDtp();

    protected:
        void *  m_dtp;
        int     m_dtp_type;
    };
}

#endif // _NINI_FTP_DTP_H_