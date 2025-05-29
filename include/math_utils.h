// include/math_utils.h - Математические утилиты

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

// GCC в freestanding режиме должен предоставлять __builtin_sinf, __builtin_cosf
// если компонуется с libgcc. Если нет, потребуются таблицы или аппроксимации.
// Для простоты будем полагаться на встроенные функции GCC, если они доступны,
// или предоставим очень грубые аппроксимации/таблицы.

// Определяем PI, если не определено
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

float custom_sinf(float x);
float custom_cosf(float x);
float custom_sqrtf(float x); // Если понадобится (для пончика обычно не критично)

#endif // MATH_UTILS_H