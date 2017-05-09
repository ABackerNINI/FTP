#pragma once

#ifndef NINI_FTP_COMMON_H
#define NINI_FTP_COMMON_H

//cut string with cutter,return parts number
//crash when string is NULL
int cut_string(char *_Str, const char _Cutter = ' ');

char upper(char c);

int stricmp_n_1(const char *_Str1, const char *_Str2);

#endif // NINI_FTP_COMMON_H
