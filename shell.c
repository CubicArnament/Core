// shell.c - Реализация логики командной оболочки (добавлена команда furmark)

#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "string_utils.h"
#include "donut.h" // Для команды furmark

static char cmd_buffer[CMD_BUFFER_SIZE];
static int cmd_buffer_len = 0;

static void process_command() {
    terminal_putchar('\n');

    if (strcmp_custom(cmd_buffer, "help") == 0) {
        terminal_writestring("KomaruCatOS Minimal Kernel v0.0.3 (Furmark)\n");
        terminal_writestring("Available commands:\n");
        terminal_writestring("  help   - Show this help message\n");
        terminal_writestring("  clear  - Clear the screen\n");
        terminal_writestring("  neofetch - Display system info (placeholder)\n");
        terminal_writestring("  echo [text] - Print text\n");
        terminal_writestring("  color [fg] [bg] - Set text colors (0-15)\n");
        terminal_writestring("  furmark - Run ASCII 3D donut benchmark\n"); // Новая команда
    } else if (strcmp_custom(cmd_buffer, "clear") == 0) {
        terminal_initialize();
    } else if (strcmp_custom(cmd_buffer, "neofetch") == 0) {
        terminal_writestring("\n");
        terminal_writestring("  OS: KomaruCatOS Minimal\n");
        terminal_writestring("  Kernel: 0.0.3\n");
        terminal_writestring("  CPU: QEMU Virtual CPU\n");
        terminal_writestring("  Shell: Built-in\n\n");
    } else if (strlen_custom(cmd_buffer) > 5 && cmd_buffer[0] == 'e' && cmd_buffer[1] == 'c' && cmd_buffer[2] == 'h' && cmd_buffer[3] == 'o' && cmd_buffer[4] == ' ') {
        terminal_writestring(&cmd_buffer[5]);
        terminal_putchar('\n');
    } else if (strlen_custom(cmd_buffer) > 6 && cmd_buffer[0] == 'c' && cmd_buffer[1] == 'o' && cmd_buffer[2] == 'l' && cmd_buffer[3] == 'o' && cmd_buffer[4] == 'r' && cmd_buffer[5] == ' ') {
        int fg = -1, bg = -1;
        int arg_idx = 6;
        // Parse fg
        if (cmd_buffer[arg_idx] >= '0' && cmd_buffer[arg_idx] <= '9') fg = cmd_buffer[arg_idx] - '0';
        else if (cmd_buffer[arg_idx] >= 'a' && cmd_buffer[arg_idx] <= 'f') fg = cmd_buffer[arg_idx] - 'a' + 10;
        arg_idx++;
        if (fg == 1 && cmd_buffer[arg_idx-1] != ' ' && cmd_buffer[arg_idx] >= '0' && cmd_buffer[arg_idx] <= '5') { // for 10-15
             fg = 10 + (cmd_buffer[arg_idx] - '0'); arg_idx++;
        }
        // Parse bg if present
        if (cmd_buffer[arg_idx] == ' ') {
            arg_idx++;
            if (cmd_buffer[arg_idx] >= '0' && cmd_buffer[arg_idx] <= '9') bg = cmd_buffer[arg_idx] - '0';
            else if (cmd_buffer[arg_idx] >= 'a' && cmd_buffer[arg_idx] <= 'f') bg = cmd_buffer[arg_idx] - 'a' + 10;
            arg_idx++;
             if (bg == 1 && cmd_buffer[arg_idx-1] != ' ' && cmd_buffer[arg_idx] >= '0' && cmd_buffer[arg_idx] <= '5') { // for 10-15
                bg = 10 + (cmd_buffer[arg_idx] - '0');
            }
        }

        if (fg != -1 && fg <=15 && bg != -1 && bg <= 15) {
            terminal_set_full_color((enum vga_color)fg, (enum vga_color)bg);
            terminal_writestring("Color updated.\n");
        } else if (fg != -1 && fg <=15 && bg == -1) {
             terminal_set_fg_color((enum vga_color)fg);
             terminal_writestring("Foreground color updated.\n");
        } else {
            terminal_writestring("Usage: color <fg_0-f> [bg_0-f]\n");
        }
    } else if (strcmp_custom(cmd_buffer, "furmark") == 0) { // Новая команда
        run_donut_benchmark();
    }
     else if (cmd_buffer_len > 0) {
        terminal_writestring("Unknown command: ");
        terminal_writestring(cmd_buffer);
        terminal_putchar('\n');
    }

    memset_custom(cmd_buffer, 0, CMD_BUFFER_SIZE);
    cmd_buffer_len = 0;
}

void shell_init() {
    memset_custom(cmd_buffer, 0, CMD_BUFFER_SIZE);
    cmd_buffer_len = 0;
}

void shell_run_cycle() {
    char c = keyboard_getchar();
    if (c == '\n') {
        process_command();
    } else if (c == '\b') {
        if (cmd_buffer_len > 0) {
            cmd_buffer_len--;
            cmd_buffer[cmd_buffer_len] = '\0';
            terminal_putchar('\b');
        }
    } else if (c != 0 && c != '\t' && c != 0x1B /*ESC*/ && cmd_buffer_len < CMD_BUFFER_SIZE - 1) {
        cmd_buffer[cmd_buffer_len] = c;
        cmd_buffer_len++;
        terminal_putchar(c);
    } else if (cmd_buffer_len >= CMD_BUFFER_SIZE - 1 && c != '\n') {
        // Buffer full, but no newline yet. Process what we have.
        terminal_writestring("\nCommand too long!\n");
        process_command(); // Process the truncated command
    }
}