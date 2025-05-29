// vga.c - Реализация функций для VGA текстового режима (добавлена terminal_clear_row)

#include "vga.h"
// vga_buffer and current_term_color are now declared extern in vga.h
// Their definitions remain here.
volatile unsigned short* vga_buffer = (unsigned short*)0xB8000;
// VGA_COLS and VGA_ROWS are now macros in vga.h
int term_col = 0;
int term_row = 0;
// Definition of the global variable
unsigned char current_term_color = (VGA_COLOR_BLACK << 4) | VGA_COLOR_WHITE;

static inline unsigned char make_vga_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

static inline unsigned short make_vga_entry(char c, unsigned char color_attr) {
    return (unsigned short)c | ((unsigned short)color_attr << 8);
}

void terminal_initialize() {
    current_term_color = make_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    for (int row = 0; row < VGA_ROWS; ++row) {
        for (int col = 0; col < VGA_COLS; ++col) {
            const int index = row * VGA_COLS + col;
            vga_buffer[index] = make_vga_entry(' ', current_term_color);
        }
    }
    term_col = 0;
    term_row = 0;
}

void terminal_set_fg_color(enum vga_color color) {
    unsigned char bg = (current_term_color >> 4) & 0x0F;
    current_term_color = make_vga_color(color, (enum vga_color)bg);
}

void terminal_set_bg_color(enum vga_color color) {
    unsigned char fg = current_term_color & 0x0F;
    current_term_color = make_vga_color((enum vga_color)fg, color);
}

void terminal_set_full_color(enum vga_color fg, enum vga_color bg) {
    current_term_color = make_vga_color(fg, bg);
}

void terminal_scroll() {
    for (int row = 1; row < VGA_ROWS; ++row) {
        for (int col = 0; col < VGA_COLS; ++col) {
            const int prev_index = row * VGA_COLS + col;
            const int curr_index = (row - 1) * VGA_COLS + col;
            vga_buffer[curr_index] = vga_buffer[prev_index];
        }
    }
    terminal_clear_row(VGA_ROWS - 1); // Очищаем последнюю строку
    term_col = 0;
    term_row = VGA_ROWS - 1;
}

void terminal_clear_row(int row) {
    if (row < 0 || row >= VGA_ROWS) return;
    for (int col = 0; col < VGA_COLS; ++col) {
        const int index = row * VGA_COLS + col;
        vga_buffer[index] = make_vga_entry(' ', current_term_color);
    }
}


void terminal_putchar(char c) {
    if (c == '\n') {
        term_col = 0;
        term_row++;
    } else if (c == '\r') {
        term_col = 0;
    } else if (c == '\b') {
        if (term_col > 0) {
            term_col--;
            const int index = term_row * VGA_COLS + term_col;
            vga_buffer[index] = make_vga_entry(' ', current_term_color);
        } else if (term_row > 0) {
            term_row--;
            term_col = VGA_COLS - 1;
             // Не стираем символ здесь, так как это может быть нежелательно при переходе строки
        }
    } else {
        if (term_row >= VGA_ROWS) {
            terminal_scroll();
        }
        const int index = term_row * VGA_COLS + term_col;
        vga_buffer[index] = make_vga_entry(c, current_term_color);
        term_col++;
    }

    if (term_col >= VGA_COLS) {
        term_col = 0;
        term_row++;
    }
    if (term_row >= VGA_ROWS) {
        terminal_scroll();
    }
}

void terminal_writestring(const char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        terminal_putchar(str[i]);
    }
}