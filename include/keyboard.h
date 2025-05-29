// include/keyboard.h - Заголовочный файл для функций клавиатуры (добавлена non-blocking проверка)

#ifndef KEYBOARD_H
#define KEYBOARD_H

// Получение символа с клавиатуры (блокирующее)
char keyboard_getchar();

// Проверка, нажата ли клавиша (неблокирующая).
// Возвращает скан-код нажатой клавиши или 0, если ничего не нажато.
// Обрабатывает только нажатия, не отпускания.
unsigned char keyboard_poll_scancode();

// Конвертация скан-кода в ASCII (если возможно)
char scancode_to_ascii(unsigned char scancode);

#endif // KEYBOARD_H