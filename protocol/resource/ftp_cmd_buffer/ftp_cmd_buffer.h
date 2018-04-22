#pragma once

#ifndef _NINI_FTP_CMD_BUFFER_H_
#define _NINI_FTP_CMD_BUFFER_H_

#include <stdio.h>

#define FTP_CMD_BUFFER_DEFAULT_BUFFER_LEN   100
#define FTP_CMD_BUFFER_MAX_BUFFER_LEN       2048

class ftp_cmd_buffer {
public:
    ftp_cmd_buffer();

    void push(const char *str, size_t count);

    char *pop();

    size_t size();

    ~ftp_cmd_buffer();

private:
    void _buffer(const char *str, size_t count);

private:
    char *  m_buffer;
    size_t  m_pos;
    size_t  m_size;
    size_t  m_capacity;
};

#endif //_NINI_FTP_CMD_BUFFER_H_
