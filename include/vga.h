// include/vga.h - Заголовочный файл для VGA текстового режима (без изменений)

#ifndef VGA_H
#define VGA_H

// Цвета VGA
enum vga_color {
    VGA_COLOR_BLACK = 0, VGA_COLOR_BLUE = 1, VGA_COLOR_GREEN = 2, VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4, VGA_COLOR_MAGENTA = 5, VGA_COLOR_BROWN = 6, VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8, VGA_COLOR_LIGHT_BLUE = 9, VGA_COLOR_LIGHT_GREEN = 10, VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12, VGA_COLOR_LIGHT_MAGENTA = 13, VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// Define VGA dimensions as macros for preprocessor visibility
#define VGA_COLS 80
#define VGA_ROWS 25

// Declare global variables defined in vga.c so other files can use them
extern volatile unsigned short* vga_buffer;
extern unsigned char current_term_color;


void terminal_initialize();
void terminal_set_fg_color(enum vga_color color);
void terminal_set_bg_color(enum vga_color color);
void terminal_set_full_color(enum vga_color fg, enum vga_color bg);
void terminal_putchar(char c);
void terminal_writestring(const char* str);
void terminal_clear_row(int row); // Новая функция для очистки строки

#endif // VGA_H