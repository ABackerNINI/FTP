#pragma once

#ifndef _NINI_FTP_CLIENT_PI_H_
#define _NINI_FTP_CLIENT_PI_H_

#include "../ftp_dtp/ftp_dtp_user.h"
#include "../../resource/ftp_cmds/ftp_cmds.h"
#include "../../../utility/network/network.h"
#include "../../resource/ftp_cmd_buffer/ftp_cmd_buffer.h"

namespace ftp_user_pi {

#define DEFAULT_BUFFER_LEN 1024

#define _CMD(CMD)  network::pointer_cast<cmd_handler>(&FtpUserPi::cmd_handler_##CMD)

    enum USER_IO_STATUS {
        UIS_CONNECTING,
        UIS_CONNECTED,
        UIS_SENDING,
        UIS_SENT,
        UIS_RECVD,
        UIS_RSP_HANDLED,
        UIS_CLOSED
    };

    //enum USER_LOGIN_STATUS {
    //    ULS_NOT_CONNECTED,
    //    ULS_CONNECTED,
    //    ULS_USERNAME_SPECIFIED,
    //    ULS_PASSWORD_SPECIFIED,
    //    ULS_LOGIN_SUCCESS,
    //    ULS_DISCONNECTED
    //};

    struct UserInf {
        ftp_cmd_buffer  m_cmd_buffer;
    };

    class FtpUserPi :public network::Client {
    public:
        FtpUserPi();

        bool ftp_connect(const char *addr, unsigned int port, unsigned int *local_port = NULL);

        bool ftp_input(char *buffer, size_t count);

        USER_IO_STATUS get_io_status();

    protected:
        bool _ftp_send(char *buffer, size_t count);

        bool _handle_input(char *buffer, size_t count);

        void _handle_response();

        virtual void event_handler(network::CLT_SOCKET_CONTEXT *sock_ctx, int ev) override;

        void _on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx);

        void _on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx);

    protected:
        typedef void(*cmd_handler)(FtpUserPi*, char *);

        const cmd_handler m_cmd_handler[ftp_cmds::FTP_CMDS_NUM + 1] = {//Need to Handle FTP_CMD_ERR
        { _CMD(USER) },
        { _CMD(PASS) },
        { _CMD(NOT_IMPLEMENTED) },//ACCT
        { _CMD(CWD) },
        { _CMD(NOT_IMPLEMENTED) },//CDUP
        { _CMD(NOT_IMPLEMENTED) },//SMNT
        { _CMD(NOT_IMPLEMENTED) },//QUIT
        { _CMD(NOT_IMPLEMENTED) },//REIN
        { _CMD(PORT) },//PORT
        { _CMD(PASV) },
        { _CMD(NOT_IMPLEMENTED) },//TYPE
        { _CMD(NOT_IMPLEMENTED) },//STRU
        { _CMD(NOT_IMPLEMENTED) },//MODE
        { _CMD(RETR) },
        { _CMD(STOR) },
        { _CMD(NOT_IMPLEMENTED) },//STOU
        { _CMD(NOT_IMPLEMENTED) },//APPE
        { _CMD(NOT_IMPLEMENTED) },//ALLO
        { _CMD(NOT_IMPLEMENTED) },//REST
        { _CMD(NOT_IMPLEMENTED) },//RNFR
        { _CMD(NOT_IMPLEMENTED) },//RNTO
        { _CMD(NOT_IMPLEMENTED) },//ABOR
        { _CMD(NOT_IMPLEMENTED) },//DELE
        { _CMD(NOT_IMPLEMENTED) },//RMD
        { _CMD(NOT_IMPLEMENTED) },//MKD
        { _CMD(NOT_IMPLEMENTED) },//PWD
        { _CMD(NOT_IMPLEMENTED) },//LIST
        { _CMD(NOT_IMPLEMENTED) },//NLST
        { _CMD(NOT_IMPLEMENTED) },//SITE
        { _CMD(NOT_IMPLEMENTED) },//SYST
        { _CMD(NOT_IMPLEMENTED) },//STAT
        { _CMD(HELP) },
        { _CMD(NOOP) },
        { _CMD(ERR) }
        };

        void cmd_handler_USER(char *args);

        void cmd_handler_PASS(char *args);

        void cmd_handler_CWD(char *args);

        void cmd_handler_PORT(char *args);

        void cmd_handler_PASV(char *args);

        void cmd_handler_RETR(char *args);

        void cmd_handler_STOR(char *args);

        void cmd_handler_DELE(char *args);

        void cmd_handler_RMD(char *args);

        void cmd_handler_MKD(char *args);

        void cmd_handler_PWD(char *args);

        void cmd_handler_LIST(char *args);

        void cmd_handler_HELP(char *args);

        void cmd_handler_NOOP(char *args);

        void cmd_handler_ERR(char *args);

        void cmd_handler_NOT_IMPLEMENTED(char *args);

    protected:
        ftp_dtp::FtpDtp     m_dtp;
        ftp_cmds::FTP_CMDS	m_last_cmd;
        UserInf			    m_client_inf;
        USER_IO_STATUS	m_client_status;
    };
}

#endif //_NINI_FTP_CLIENT_PI_H_