#pragma once

#ifndef _NINI_STRING_BUFFER_H_
#define _NINI_STRING_BUFFER_H_

#include <stdio.h>

#define STRING_BUFFER_DEFAULT_BUFFER_LEN 100

class string_buffer {
public:
    string_buffer();
    void push(const char *str, size_t count);
    char *pop();
    size_t size();
    ~string_buffer();
private:
    void _buffer(const char *str, size_t count);
private:
    char *m_Buffer;
    size_t m_Pos;
    size_t m_Size;
    size_t m_Capacity;
};

#endif // !_NINI_STRING_BUFFER_H_
