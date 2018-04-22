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
    char *  m_buffer;
    size_t  m_pos;
    size_t  m_size;
    size_t  m_capacity;
};

#endif //_NINI_STRING_BUFFER_H_
