#pragma once

#ifndef _NINI_FTP_DTP_H_
#define _NINI_FTP_DTP_H_

#include "../../../utility/file/file.h"
#include "../../../utility/network/network.h"

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

    class ftp_dtp_client : public network::Client {
    public:
        bool start();

        bool abort();

        bool stop();

        virtual void OnConnected(network::CLT_SOCKET_CONTEXT *sock_ctx) override;

        virtual void OnSent(network::CLT_SOCKET_CONTEXT *sock_ctx) override;

        virtual void OnRecvd(network::CLT_SOCKET_CONTEXT *sock_ctx) override;

        virtual void OnClosed(network::CLT_SOCKET_CONTEXT *sock_ctx) override;

    protected:
        file::file_reader m_fr;
    };

    class ftp_dtp_server : public network::Server {
    public:
        bool start();

        bool abort();

        bool stop();

        virtual void OnAccepted(network::SVR_SOCKET_CONTEXT *sock_ctx) override;

        virtual void OnRecvd(network::SVR_SOCKET_CONTEXT *sock_ctx) override;

        virtual void OnSent(network::SVR_SOCKET_CONTEXT *sock_ctx) override;

        virtual void OnClosed(network::SVR_SOCKET_CONTEXT *sock_ctx) override;

    protected:
        file::file_writer m_fw;
    };

    class ftp_dtp {
    public:
        ftp_dtp();

        bool start();

        bool abort();

        bool stop();

        ~ftp_dtp();

    public:
        enum STATUS get_status();
        bool get_passive();
        enum STRUCTURE_TYPE get_structure_type();
        enum DATA_TYPE get_data_type();
        const char *get_ip();
        int get_port();

        void set_passive(bool passive);
        void set_structure_type(enum STRUCTURE_TYPE sturcture_type);
        void set_data_type(enum DATA_TYPE data_type);
        void set_ip(const char *ip);
        void set_port(int port);

    private:
        STATUS              m_status;
        bool                m_passive;
        STRUCTURE_TYPE      m_sturcture_type;
        DATA_TYPE           m_data_type;

        const char*         m_ip;
        int                 m_port;

        ftp_dtp_client      m_client;
        ftp_dtp_server      m_server;
    };
}

#endif // _NINI_FTP_DTP_H_
