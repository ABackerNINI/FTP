#pragma once

#ifndef NINI_FTP_FILE_H
#define NINI_FTP_FILE_H

#include <stdio.h>

class File {
public:
	File();

	bool Open(const char *_Path,const char *_Mode = "rb");

	bool Jump(int _Bytes);

	int Read();

	bool Close();

public:
	static int Delete(const char *_Path);

protected:
	long long		m_Cur;
	
	FILE			*m_File;
};

#endif //NINI_FTP_FILE_H