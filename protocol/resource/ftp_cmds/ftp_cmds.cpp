#include "ftp_cmds.h"

enum ftp_cmds::FTP_CMDS ftp_cmds::cmd_dispatch(char **str) {
    char *p = *str;
    FTP_CMDS ret = FTP_CMD_ERR;

    while (*p == ' ')++p;

    for (int i = 0; i < FTP_CMDS_NUM; ++i) {
        if (stricmp_n_1(FTP_CMDS_INF[i].m_cmd, p) == 0) {
            ret = (FTP_CMDS)i;
        }
    }

    *str += 4;

    while (**str == ' ')++(*str);

    if (**str == '\0')*str = NULL;

    return ret;
}
