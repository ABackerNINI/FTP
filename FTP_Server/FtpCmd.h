#pragma once

#ifndef NINI_FTP_FTP_CMD_H
#define NINI_FTP_FTP_CMD_H

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

struct _FTP_CMDS_INF {
	const char *	m_Cmd;
	bool			m_NeedArgs;
	const char *	m_HelpMsg;
};

void _CmdHandle_USER(char *_Args);

static const int FTP_CMDS_NUM = 34 - 1;//FTP_CMD_ERR Is Not a CMD

static const _FTP_CMDS_INF FTP_CMDS_INF[] = {
	{ "USER", TRUE,  "Supply a username: USER username" },
	{ "PASS", TRUE,  "Supply a user password: PASS password" },
	{ "ACCT", FALSE, ""},
	{ "CWD",  TRUE,	 "Change working directory: CWD [directory-name]" },
	{ "CDUP", FALSE, ""},
	{ "SMNT", FALSE, "" },
	{ "QUIT", FALSE, "Logout or break the connection: QUIT" },
	{ "REIN", FALSE, "" },
	{ "PORT", TRUE,	 "Specify the client port number: PORT a0,a1,a2,a3,a4,a5" },
	{ "PASV", FALSE, "Set server in passive mode: PASV" },
	{ "TYPE", TRUE,	 "Set filetype: TYPE [A | I]" },
	{ "STRU", FALSE, "" },
	{ "MODE", FALSE, "" },
	{ "RETR", TRUE,	 "Get file: RETR file-name" },
	{ "STOR", TRUE,	 "Store file: STOR file-name" },
	{ "STOU", FALSE, "" },
	{ "APPE", FALSE, "" },
	{ "ALLO", FALSE, "" },
	{ "REST", TRUE,	 "Set restart transfer marker: REST marker" },
	{ "RNFR", TRUE,  "Specify old path name of file to be renamed: RNFR file-name" },
	{ "RNTO", TRUE,	 "Specify new path name of file to be renamed: RNTO file-name" },
	{ "ABOR", FALSE, "Abort transfer: ABOR" },
	{ "DELE", TRUE , "Delete file: DELE file-name" },
	{ "RMD",  TRUE,	 "Remove directory: RMD path-name" },
	{ "MKD",  TRUE,	 "Make directory: MKD path-name" },
	{ "PWD",  FALSE, "Get current directory: PWD" },
	{ "LIST", FALSE, "Get directory listing: LIST [path-name]" },
	{ "NLST", FALSE, ""},
	{ "SITE", FALSE, ""},
	{ "SYST", FALSE, "Get operating system type: SYST" } ,
	{ "STAT", FALSE, "" },
	{ "HELP", FALSE, "Show help: HELP [command]" },
	{ "NOOP", FALSE, "Do nothing: NOOP" }
};

#endif//NINI_FTP_FTP_CMD_H