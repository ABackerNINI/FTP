#pragma once

#ifndef _NINI_FTP_RESOURCE_COMMON_H_
#define _NINI_FTP_RESOURCE_COMMON_H_

//cut string with cutter,return parts number
//crash when string is NULL
int cut_string(char *_Str, const char _Cutter = ' ');

char upper(char c);

int stricmp_n_1(const char *_Str1, const char *_Str2);

#endif //_NINI_FTP_RESOURCE_COMMON_H_
