#pragma once

#ifndef _NINI_FTP_RESOURCE_FTP_CMD_H_
#define _NINI_FTP_RESOURCE_FTP_CMD_H_

#include <stdio.h>
#include "../Common/Common.h"

//#define FTP_CMD_USER 1
//#define FTP_CMD_PASS 2
//#define FTP_CMD_ACCT 3
//#define FTP_CMD_CWD  4
//#define FTP_CMD_CDUP 5
//#define FTP_CMD_SMNT 6
//#define FTP_CMD_QUIT 7
//#define FTP_CMD_REIN 8
//#define FTP_CMD_PORT 9
//#define FTP_CMD_PASV 10
//#define FTP_CMD_TYPE 11
//#define FTP_CMD_STRU 12
//#define FTP_CMD_MODE 13
//#define FTP_CMD_RETR 14
//#define FTP_CMD_STOR 15
//#define FTP_CMD_STOU 16
//#define FTP_CMD_APPE 17
//#define FTP_CMD_ALLO 18
//#define FTP_CMD_REST 19
//#define FTP_CMD_RNFR 20
//#define FTP_CMD_RNTO 21
//#define FTP_CMD_ABOR 22
//#define FTP_CMD_DELE 23
//#define FTP_CMD_RMD  24
//#define FTP_CMD_MKD  25
//#define FTP_CMD_PWD  26
//#define FTP_CMD_LIST 27
//#define FTP_CMD_NLST 28
//#define FTP_CMD_SITE 29
//#define FTP_CMD_SYST 30
//#define FTP_CMD_STAT 31
//#define FTP_CMD_HELP 32
//#define FTP_CMD_NOOP 33
//#define FTP_CMD_ERR  34

enum FTP_CMDS {
    FTP_CMD_USER,
    FTP_CMD_PASS,
    FTP_CMD_ACCT,
    FTP_CMD_CWD,
    FTP_CMD_CDUP,
    FTP_CMD_SMNT,
    FTP_CMD_QUIT,
    FTP_CMD_REIN,
    FTP_CMD_PORT,
    FTP_CMD_PASV,
    FTP_CMD_TYPE,
    FTP_CMD_STRU,
    FTP_CMD_MODE,
    FTP_CMD_RETR,
    FTP_CMD_STOR,
    FTP_CMD_STOU,
    FTP_CMD_APPE,
    FTP_CMD_ALLO,
    FTP_CMD_REST,
    FTP_CMD_RNFR,
    FTP_CMD_RNTO,
    FTP_CMD_ABOR,
    FTP_CMD_DELE,
    FTP_CMD_RMD,
    FTP_CMD_MKD,
    FTP_CMD_PWD,
    FTP_CMD_LIST,
    FTP_CMD_NLST,
    FTP_CMD_SITE,
    FTP_CMD_SYST,
    FTP_CMD_STAT,
    FTP_CMD_HELP,
    FTP_CMD_NOOP,
    FTP_CMD_ERR
};

enum FTP_CMDS_NEED_ARGS {
    FCNA_NONE,
    FCNA_MANDATORY,
    FCNA_OPTIONAL
};

struct _FTP_CMDS_INF {
    const char *			m_Cmd;
    FTP_CMDS_NEED_ARGS		m_NeedArgs;
    const char *			m_HelpMsg;
};

static const int FTP_CMDS_NUM = 34 - 1;//FTP_CMD_ERR Is Not a CMD

static const _FTP_CMDS_INF FTP_CMDS_INF[] = {
    { "USER", FCNA_MANDATORY, "Supply a username: USER username" },
    { "PASS", FCNA_MANDATORY, "Supply a user password: PASS password" },
    { "ACCT", FCNA_NONE, ""},
    { "CWD",  FCNA_MANDATORY, "Change working directory: CWD [directory-name]" },
    { "CDUP", FCNA_NONE, ""},
    { "SMNT", FCNA_NONE, "" },
    { "QUIT", FCNA_NONE, "Logout or break the connection: QUIT" },
    { "REIN", FCNA_NONE, "" },
    { "PORT", FCNA_MANDATORY, "Specify the client port number: PORT a0,a1,a2,a3,a4,a5" },
    { "PASV", FCNA_NONE, "Set server in passive mode: PASV" },
    { "TYPE", FCNA_MANDATORY, "Set filetype: TYPE [A | I]" },
    { "STRU", FCNA_NONE, "" },
    { "MODE", FCNA_NONE, "" },
    { "RETR", FCNA_MANDATORY, "Get file: RETR file-name" },
    { "STOR", FCNA_MANDATORY, "Store file: STOR file-name" },
    { "STOU", FCNA_NONE, "" },
    { "APPE", FCNA_NONE, "" },
    { "ALLO", FCNA_NONE, "" },
    { "REST", FCNA_MANDATORY, "Set restart transfer marker: REST marker" },
    { "RNFR", FCNA_MANDATORY, "Specify old path name of file to be renamed: RNFR file-name" },
    { "RNTO", FCNA_MANDATORY, "Specify new path name of file to be renamed: RNTO file-name" },
    { "ABOR", FCNA_NONE, "Abort transfer: ABOR" },
    { "DELE", FCNA_MANDATORY, "Delete file: DELE file-name" },
    { "RMD",  FCNA_MANDATORY, "Remove directory: RMD path-name" },
    { "MKD",  FCNA_MANDATORY, "Make directory: MKD path-name" },
    { "PWD",  FCNA_NONE, "Get current directory: PWD" },
    { "LIST", FCNA_NONE, "Get directory listing: LIST [path-name]" },
    { "NLST", FCNA_NONE, ""},
    { "SITE", FCNA_NONE, ""},
    { "SYST", FCNA_NONE, "Get operating system type: SYST" } ,
    { "STAT", FCNA_NONE, "" },
    { "HELP", FCNA_OPTIONAL, "Show help: HELP [command]" },
    { "NOOP", FCNA_NONE, "Do nothing: NOOP" }
};

static const char HELP_MSG[] =
"214 The following commands are recognized:\r\n"
"USER\r\n"
"PASS\r\n"
"ACCT\r\n"
""
""
""
""
""
""
"214 HELP command successful.\r\n";

#endif //_NINI_FTP_RESOURCE_FTP_CMD_H_