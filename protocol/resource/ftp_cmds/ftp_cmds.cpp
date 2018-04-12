#include "ftp_cmds.h"

enum ftp_cmds::FTP_CMDS ftp_cmds::CmdDispatch(char **_Str) {
    char *p = *_Str;
    FTP_CMDS _Ret = FTP_CMD_ERR;

    while (*p == ' ')++p;

    for (int i = 0; i < FTP_CMDS_NUM; ++i) {
        if (stricmp_n_1(FTP_CMDS_INF[i].m_Cmd, p) == 0) {
            _Ret = (FTP_CMDS)i;
        }
    }

    *_Str += 4;

    while (**_Str == ' ')++(*_Str);

    if (**_Str == '\0')*_Str = NULL;

    return _Ret;
}
