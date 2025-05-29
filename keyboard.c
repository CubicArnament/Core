// keyboard.c - Реализация функций для работы с клавиатурой (добавлена non-blocking проверка)

#include "keyboard.h"

// Вспомогательная функция для чтения байта из порта
static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

// Порты клавиатуры
#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64
#define KBD_CMD_PORT    0x64

// Флаги статуса клавиатуры
#define KBD_OUTPUT_BUFFER_FULL 0x01 // Данные готовы к чтению из порта 0x60
#define KBD_INPUT_BUFFER_FULL  0x02 // Данные готовы к записи в порт 0x60 или 0x64

// Таблица скан-кодов (Set 1, основные символы)
static const char sc_ascii_map[] = {
    0,  0x1B, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', // 0x00-0x0E (ESC, Backspace)
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', // 0x0F-0x1C (Tab, Enter)
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', // 0x1D-0x29 (LCtrl)
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, // 0x2A-0x36 (LShift, RShift)
    '*', 0, ' ', 0, // 0x37 (Keypad *), 0x38 (LAlt), 0x39 (Space), 0x3A (CapsLock)
    0,0,0,0,0,0,0,0,0,0, // F1-F10 (0x3B-0x44)
    0,0, // NumLock, ScrollLock (0x45-0x46)
    // Keypad numbers
    '7','8','9', // 0x47-0x49
    '-', // 0x4A (Keypad -)
    '4','5','6', // 0x4B-0x4D
    '+', // 0x4E (Keypad +)
    '1','2','3', // 0x4F-0x51
    '0', // 0x52 (Keypad 0)
    '.', // 0x53 (Keypad .)
};

char scancode_to_ascii(unsigned char scancode) {
    if (scancode < sizeof(sc_ascii_map)) {
        return sc_ascii_map[scancode];
    }
    return 0;
}

unsigned char keyboard_poll_scancode() {
    // Проверяем бит 0 (Output Buffer Status) порта статуса.
    // Если он установлен, значит есть данные для чтения.
    if (inb(KBD_STATUS_PORT) & KBD_OUTPUT_BUFFER_FULL) {
        unsigned char scancode = inb(KBD_DATA_PORT);
        // Нас интересуют только нажатия (старший бит 0)
        if (!(scancode & 0x80)) {
            return scancode;
        }
    }
    return 0; // Нет нажатой клавиши (или это было отпускание)
}


char keyboard_getchar() {
    static unsigned char last_processed_scancode = 0x00; // Для обработки только новых нажатий
    unsigned char scancode;
    char ascii_char;

    while (1) {
        while (!(inb(KBD_STATUS_PORT) & KBD_OUTPUT_BUFFER_FULL)) {
             for (volatile int i = 0; i < 5000; ++i);
        }
        scancode = inb(KBD_DATA_PORT);
        if (!(scancode & 0x80)) { // Key press
            if (scancode != last_processed_scancode) {
                 last_processed_scancode = scancode;
                 ascii_char = scancode_to_ascii(scancode);
                 if (ascii_char != 0) {
                     return ascii_char;
                 }
            }
        } else { // Key release
            if ((scancode & 0x7F) == (last_processed_scancode & 0x7F)) {
                last_processed_scancode = 0x00;
            }
        }
         for (volatile int i = 0; i < 5000; ++i);
    }
}