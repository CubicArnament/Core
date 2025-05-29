// math_utils.c - Реализация математических утилит

#include "math_utils.h"

// Используем встроенные функции GCC для sinf и cosf, если доступны.
// Они обычно предоставляются через libgcc.
// Это самый простой способ получить float-математику в freestanding.
// Если бы их не было, пришлось бы реализовывать таблицы поиска или ряды Тейлора.

float custom_sinf(float x) {
    return __builtin_sinf(x);
}

float custom_cosf(float x) {
    return __builtin_cosf(x);
}

// Простая реализация sqrt через метод Ньютона, если понадобится.
// Для пончика она обычно не нужна.
float custom_sqrtf(float number) {
    if (number < 0.0f) return 0.0f; // Не обрабатываем комплексные числа, возвращаем 0 или NaN-эквивалент
    if (number == 0.0f) return 0.0f;

    float x = number;
    float y = 1.0f;
    float e = 0.000001f; // Точность
    while (x - y > e) {
        x = (x + y) / 2.0f;
        y = number / x;
    }
    return x;
}