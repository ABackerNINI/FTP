#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "../utility/string_buffer/string_buffer.h"

void test_string_buffer() {
    string_buffer sb;
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
}

int main() {
    test_string_buffer();

    _getch();

    return 0;
}