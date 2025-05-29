// string_utils.c - Реализация строковых утилит (добавлен memset_custom)

#include "string_utils.h"

int strlen_custom(const char* str) {
    int len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

int strcmp_custom(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strcpy_custom(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++));
    return original_dest;
}

void* memset_custom(void* s, int c, unsigned int n) {
    unsigned char* p = (unsigned char*)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}