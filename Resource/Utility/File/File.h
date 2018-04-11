#pragma once

#ifndef _NINI_FILE_H_
#define _NINI_FILE_H_

#include <stdio.h>

class File {
public:
    File();

    bool Open(const char *_Path, const char *_Mode = "rb");

    bool Jump(int _Bytes);

    int Read();

    bool Close();

public:
    static int Delete(const char *_Path);

protected:
    long long		m_Cur;

    FILE			*m_File;
};

#endif //_NINI_FILE_H_