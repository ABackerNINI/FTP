#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "../protocol/resource/ftp_cmd_buffer/ftp_cmd_buffer.h"

#pragma warning(disable:4996)

void test_normal() {
    ftp_cmd_buffer sb;
    char *s1 = new char[100];
    char *s2 = new char[100];
    char *s3 = new char[100];
    char *s4 = new char[100];

    strcpy(s1, "214 ");
    strcpy(s2, "Supply a user password: PASS password");
    strcpy(s3, "\r\n");
    strcpy(s4, "123\r\n");

    sb.push(s4, strlen(s4));

    printf("%s\n", sb.pop());

    sb.push(s1, strlen(s1));
    sb.push(s2, strlen(s2));
    sb.push(s3, strlen(s3));

    printf("%s\n", sb.pop());

    delete[] s1;
    delete[] s2;
    delete[] s3;
    delete[] s4;
}

void test_long_cmd() {
    ftp_cmd_buffer sb;
    char *s1 = new char[10000];
    char *s2 = new char[100];

    for (int i = 0; i < 10000; ++i) {
        s1[i] = '0';
    }
    s1[9999] = '\0';

    strcpy(s2, "Supply a user password: PASS password\r\n");

    sb.push(s1, strlen(s1));

    printf("%s\n", sb.pop());

    printf("%s\n", sb.pop());
    
    sb.push(s2, strlen(s2));

    printf("%s\n", sb.pop());

    delete[] s1;
}

int main() {
    test_normal();

    test_long_cmd();

    _getch();

    return 0;
}