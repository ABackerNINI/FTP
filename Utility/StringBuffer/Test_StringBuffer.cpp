#include <stdio.h>
#include <conio.h>
#include "StringBuffer.h"

void StringBufferTest() {
    StringBuffer sb;
    char *s1 = new char[100];
    char *s2 = new char[100];
    char *s3 = new char[100];
    char *s4 = new char[100];

    strcpy(s1, "214 ");
    strcpy(s2, "Supply a user password: PASS password");
    strcpy(s3, "\r\n");
    strcpy(s4, "123\r\n");

    sb.push(s4, strlen(s4));

    printf("%s", sb.pop());

    sb.push(s1, strlen(s1));
    sb.push(s2, strlen(s2));
    sb.push(s3, strlen(s3));

    printf("%s", sb.pop());
}

int main() {
    StringBufferTest();

    _getch();

    return 0;
}