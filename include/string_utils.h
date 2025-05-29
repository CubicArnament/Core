// include/string_utils.h - Заголовочный файл для строковых утилит (без изменений)

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

int strlen_custom(const char* str);
int strcmp_custom(const char* s1, const char* s2);
char* strcpy_custom(char* dest, const char* src);
void* memset_custom(void* s, int c, unsigned int n); // Добавим memset

#endif // STRING_UTILS_H