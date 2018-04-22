#include "ftp_cmd_buffer.h"
#include <memory.h>
#include <malloc.h>

ftp_cmd_buffer::ftp_cmd_buffer() :
    m_buffer((char *)malloc(FTP_CMD_BUFFER_DEFAULT_BUFFER_LEN)),
    m_pos(0),
    m_size(0),
    m_capacity(FTP_CMD_BUFFER_DEFAULT_BUFFER_LEN) {
}

void ftp_cmd_buffer::push(const char *str, size_t count) {
    if (str[count - 1] == '\0')count -= 1;

    _buffer(str, count);
}

char *ftp_cmd_buffer::pop() {
    char *p0 = m_buffer + m_pos;
    char *p1 = p0;
    char *p2 = p1 + m_size;
    for (; p1 != p2; ++p1) {
        if (*p1 == '\r'&&*(p1 + 1) == '\n') {
            *p1 = '\0';

            m_pos += (p1 - p0) + 2;
            m_size = (p2 - p1) - 2;

            return p0;
        }
    }

    return NULL;
}

size_t ftp_cmd_buffer::size() {
    return m_size;
}

ftp_cmd_buffer::~ftp_cmd_buffer() {
    if (m_buffer)free(m_buffer);
}

void ftp_cmd_buffer::_buffer(const char *str, size_t count) {
    //if the buffer capacity is not enough,enlarge it.
    if (m_capacity < m_size + count) {
        size_t tmp = m_size + count + FTP_CMD_BUFFER_DEFAULT_BUFFER_LEN;

        //if the cmd is too long,abondon all.
        if (tmp > FTP_CMD_BUFFER_MAX_BUFFER_LEN) {
            m_pos = 0;
            m_size = 0;
            return;
        }

        m_capacity = tmp;
        m_buffer = (char *)realloc(m_buffer, m_capacity);
        //TODO if(m_buffer==NULL)
    }

    //if the rest of the buffer is not enough,move the string to the buffer head.
    if (m_capacity < m_pos + m_size + count) {
        memcpy(m_buffer, m_buffer + m_pos, m_size);
        m_pos = 0;
    }

    //append new string at the end of the old.
    memcpy(m_buffer + m_pos + m_size, str, count);

    m_size += count;
}