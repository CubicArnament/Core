// kernel.c - Основная функция ядра и главный цикл (без изменений от предыдущей многофайловой версии)

#include "vga.h"
#include "keyboard.h"
#include "shell.h"

// Основная функция ядра
void kmain() {
    terminal_initialize();
    terminal_set_fg_color(VGA_COLOR_GREEN);
    terminal_writestring("Welcome to KomaruCatOS Minimal Kernel (Multifile + Furmark)!\n");
    terminal_writestring("Type 'help' for a list of commands.\n\n");
    terminal_set_fg_color(VGA_COLOR_WHITE); // Возвращаем стандартный цвет

    shell_init(); // Инициализация оболочки (если необходимо)

    while (1) {
        terminal_writestring("> "); // Приглашение командной строки
        shell_run_cycle(); // Запускаем один цикл обработки ввода и выполнения команды
    }
}