#include "StringBuffer.h"
#include <memory.h>
#include <malloc.h>

StringBuffer::StringBuffer() :
    m_Buffer((char *)malloc(STRING_BUFFER_DEFAULT_BUFFER_LEN)),
    m_Pos(0),
    m_Size(0),
    m_Capacity(STRING_BUFFER_DEFAULT_BUFFER_LEN) {
}

void StringBuffer::push(const char *str, size_t count) {
    if (str[count - 1] == '\0')count -= 1;

    _buffer(str, count);
}

char *StringBuffer::pop() {
    char *p0 = m_Buffer + m_Pos;
    char *p1 = p0;
    char *p2 = p1 + m_Size;
    for (; p1 != p2; ++p1) {
        if (*p1 == '\r'&&*(p1 + 1) == '\n') {
            *p1 = '\0';

            m_Pos += (p1 - p0) + 2;
            m_Size = (p2 - p1) - 2;

            return p0;
        }
    }

    return NULL;
}

size_t StringBuffer::size() {
    return m_Size;
}

StringBuffer::~StringBuffer() {
    if (m_Buffer)free(m_Buffer);
}

void StringBuffer::_buffer(const char *str, size_t count) {
    if (m_Capacity < m_Size + count) {
        m_Capacity = m_Size + count + STRING_BUFFER_DEFAULT_BUFFER_LEN;
        m_Buffer = (char *)realloc(m_Buffer, m_Capacity);
        //TODO if(m_Buffer==NULL)
    }

    if (m_Capacity < m_Pos + m_Size + count) {
        memcpy(m_Buffer, m_Buffer + m_Pos, m_Size);
        m_Pos = 0;
    }

    memcpy(m_Buffer + m_Pos + m_Size, str, count);

    m_Size += count;
}