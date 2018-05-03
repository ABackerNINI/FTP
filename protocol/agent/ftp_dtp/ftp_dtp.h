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

    enum STATUS {
        STA_NULL,
        STA_ERROR,
        STA_SENDING,
        STA_ABORTED,
        STA_COMPLETED
    };

    enum STRUCTURE_TYPE {
        STRU_FILE,
        STRU_RECORD,
        STRU_PAGE
    };

    enum DATA_TYPE {
        TYPE_STREAM,
        TYPE_BLOCK,
        TYPE_COMPRESSED
    };

    class ftp_dtp_active : public network::Client {
    public:
        ftp_dtp_active();

        bool abort();

    public:
        void set_completion_port_2(HANDLE completion_port_2);
        void set_sockid_2(SOCKET sockid);
        void set_is_to_send(bool is_to_send);
        void set_fpath(const char *fpath);

        size_t get_bytes_sent();
        size_t get_fsize();

    protected:
        virtual void event_handler(network::CLT_SOCKET_CONTEXT *sock_ctx, int ev) override;

        void _on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx);

    protected:
        HANDLE              m_completion_port_2;//post back
        SOCKET              m_sockid_2;//post back

        bool                m_is_to_send;
        const char *        m_fpath;
        size_t              m_fsize;
        size_t              m_bytes_transfered;
        file::File          m_file;
    };

    class ftp_dtp_passive : public network::Server {
    public:
        ftp_dtp_passive();

        bool abort();

    public:
        void set_completion_port_2(HANDLE completion_port_2);
        void set_sockid_2(SOCKET sockid);
        void set_is_to_send(bool is_to_send);
        void set_fpath(const char *fpath);

        size_t get_bytes_recvd();

    protected:
        virtual void event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) override;

        void _on_accepted(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx);

    protected:
        HANDLE              m_completion_port_2;//post back
        SOCKET              m_sockid_2;//post back

        bool                m_is_to_send;
        const char *        m_fpath;
        size_t              m_fsize;
        file::File          m_file;

        size_t              m_bytes_transfered;
    };

    class ftp_dtp {
    public:
        ftp_dtp();

        bool start();

        bool abort();

        bool close();

        ~ftp_dtp();

    public:

        bool get_passive();
        const char *get_addr();
        int get_port();

        void set_completion_port_2(HANDLE completion_port_2);
        void set_sockid_2(SOCKET sockid);
        void set_passive(bool passive);
        void set_is_to_send(bool is_to_send);
        void set_fpath(const char *fpath);
        void set_addr(const char *addr);
        void set_port(unsigned int port);

    protected:
        HANDLE              m_completion_port_2;//post back
        SOCKET              m_sockid_2;//post back

        bool                m_passive;

        bool                m_is_to_send;
        const char*         m_fpath;
        const char*         m_addr;
        unsigned int        m_port;

        ftp_dtp_active*     m_dtp_active;
        ftp_dtp_passive*    m_dtp_passive;
    };



    class ftp_dtp_passive_send : public network::Server {
    public:
        ftp_dtp_passive_send(const char *fpath, unsigned int *local_port, HANDLE completion_port_2, SOCKET sockid_2);

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

    class ftp_dtp_passive_recv : public network::Server {
    public:
        ftp_dtp_passive_recv(const char *fpath, unsigned int *local_port, HANDLE completion_port_2, SOCKET sockid_2);

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

    class ftp_dtp_active_send : public network::Client {
    public:
        ftp_dtp_active_send(const char *fpath, const char *addr, unsigned int port, HANDLE completion_port_2, SOCKET sockid_2);

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

    class ftp_dtp_active_recv : public network::Client {
    public:
        ftp_dtp_active_recv(const char *fpath, const char *addr, unsigned int port, HANDLE completion_port_2, SOCKET sockid_2);

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

    class ftp_dtp_2 {
    public:
        ftp_dtp_2();

        bool start(int dtp_type, const char *fpath, const char *addr, unsigned int port, unsigned int *local_port = NULL, HANDLE completion_port_2 = NULL, SOCKET sockid_2 = INVALID_SOCKET);

        bool abort();

        bool close();

        ~ftp_dtp_2();

    protected:
        void *  m_dtp;
        int     m_dtp_type;
    };
}

#endif // _NINI_FTP_DTP_H_

//STATUS              m_status;
//STRUCTURE_TYPE      m_sturcture_type;
//DATA_TYPE           m_data_type;
//enum STATUS get_status();
//enum STRUCTURE_TYPE get_structure_type();
//enum DATA_TYPE get_data_type();
//void set_structure_type(enum STRUCTURE_TYPE sturcture_type);
//void set_data_type(enum DATA_TYPE data_type);