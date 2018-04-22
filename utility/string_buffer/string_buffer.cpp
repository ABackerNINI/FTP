#include "string_buffer.h"
#include <memory.h>
#include <malloc.h>

string_buffer::string_buffer() :
    m_buffer((char *)malloc(STRING_BUFFER_DEFAULT_BUFFER_LEN)),
    m_pos(0),
    m_size(0),
    m_capacity(STRING_BUFFER_DEFAULT_BUFFER_LEN) {
}

void string_buffer::push(const char *str, size_t count) {
    if (str[count - 1] == '\0')count -= 1;

    _buffer(str, count);
}

char *string_buffer::pop() {
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

size_t string_buffer::size() {
    return m_size;
}

string_buffer::~string_buffer() {
    if (m_buffer)free(m_buffer);
}

void string_buffer::_buffer(const char *str, size_t count) {
    if (m_capacity < m_size + count) {
        //TODO string len check
        m_capacity = m_size + count + STRING_BUFFER_DEFAULT_BUFFER_LEN;
        m_buffer = (char *)realloc(m_buffer, m_capacity);
        //TODO if(m_buffer==NULL)
    }

    if (m_capacity < m_pos + m_size + count) {
        memcpy(m_buffer, m_buffer + m_pos, m_size);
        m_pos = 0;
    }

    memcpy(m_buffer + m_pos + m_size, str, count);

    m_size += count;
}