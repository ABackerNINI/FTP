#pragma once

#ifndef _NINI_FTP_DTP_H_
#define _NINI_FTP_DTP_H_

#include "../../../utility/file/file.h"
#include "../../../utility/network/network.h"

namespace ftp_dtp {

#define DEBUG 1
#define DEBUG_ERROR_CHECK 1
#define DEFAULT_BUFFER_LEN 1024

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

    class ftp_dtp_client : public network::Client {
    public:
        ftp_dtp_client();

        bool abort();

    public:
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
        const char *        m_fpath;
        size_t              m_bytes_sent;
        size_t              m_fsize;
        file::file_reader   m_freader;
    };

    class ftp_dtp_server : public network::Server {
    public:
        ftp_dtp_server();

        bool abort();

    public:
        void set_fpath(const char *fpath);
        void set_completion_port_2(HANDLE completion_port_2);
        void set_sockid(SOCKET sockid);

        size_t get_bytes_recvd();

    protected:
        virtual void event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) override;

        void _on_accepted(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx);

    protected:
        HANDLE              m_completion_port_2;//post back
        SOCKET              m_sockid;

        const char *        m_fpath;
        size_t              m_bytes_recvd;
        file::file_writer   m_fwriter;
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
        void set_sockid(SOCKET sockid);
        void set_passive(bool passive);
        void set_fpath(const char *fpath);
        void set_addr(const char *addr);
        void set_port(unsigned int port);

    protected:
        HANDLE              m_completion_port_2;//post back
        SOCKET              m_sockid;

        bool                m_passive;

        const char*         m_fpath;
        const char*         m_addr;
        int                 m_port;

        ftp_dtp_client*     m_client;
        ftp_dtp_server*     m_server;
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