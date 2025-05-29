// donut.c - Реализация ASCII 3D пончика "Furmark"
// (User's version with corrections for current_term_color access)

#include "donut.h"
#include "vga.h"          // Для VGA_COLS, VGA_ROWS, vga_buffer, terminal_*, и extern current_term_color
#include "keyboard.h"     // Для keyboard_poll_scancode, scancode_to_ascii
#include "math_utils.h"   // Используем custom_sinf, custom_cosf
#include "string_utils.h" // Для memset_custom

// Вспомогательная функция для чтения байта из порта (нужна для очистки буфера клавиатуры)
static inline unsigned char inb_local(unsigned short port) __attribute__((always_inline));
static inline unsigned char inb_local(unsigned short port) {
    unsigned char ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}
// Эти определения уже есть в keyboard.c, но для локальной функции inb_local могут быть полезны здесь,
// если keyboard.c не предоставляет прямой доступ к ним или если мы хотим изолировать эту логику.
// Однако, лучше использовать константы из keyboard.h если они там определены и экспортированы.
// Для простоты, если они не экспортированы, дублируем здесь для inb_local.
#define KBD_DATA_PORT_LOCAL   0x60
#define KBD_STATUS_PORT_LOCAL 0x64
#define KBD_OUTPUT_BUFFER_FULL_LOCAL 0x01


// Размеры экрана для пончика
#define DONUT_SCREEN_WIDTH  VGA_COLS // Use macros from vga.h
#define DONUT_SCREEN_HEIGHT VGA_ROWS // Use macros from vga.h

// Параметры тора (пончика)
const float R1 = 1.0f;    // Радиус от центра тора до центра трубки
const float R2 = 2.0f;    // Радиус самой трубки
const float K2 = 5.0f;    // Расстояние от наблюдателя до экрана
// K1 = screen_width * K2 * 3 / (8 * (R1 + R2)) - масштабирование для экрана
const float K1 = (float)DONUT_SCREEN_WIDTH * K2 * 3.0f / (8.0f * (R1 + R2));


void render_donut_frame(float A, float B, char* output_buffer, float* z_buffer) {
    // Очистка буферов
    memset_custom(output_buffer, ' ', DONUT_SCREEN_WIDTH * DONUT_SCREEN_HEIGHT);
    for (int i = 0; i < DONUT_SCREEN_WIDTH * DONUT_SCREEN_HEIGHT; ++i) {
        z_buffer[i] = -10000.0f; // Инициализируем z_buffer очень маленькими значениями (далеко)
    }

    float cosA = custom_cosf(A), sinA = custom_sinf(A);
    float cosB = custom_cosf(B), sinB = custom_sinf(B);

    // Итерируемся по поверхности тора
    for (float phi = 0; phi < 2 * M_PI; phi += 0.07f) { // Шаг по phi
        float cosphi = custom_cosf(phi), sinphi = custom_sinf(phi);
        for (float theta = 0; theta < 2 * M_PI; theta += 0.02f) { // Шаг по theta
            float costheta = custom_cosf(theta), sintheta = custom_sinf(theta);

            float circlex = R2 + R1 * costheta;
            float circley = R1 * sintheta;

            float x = circlex * (cosB * cosphi + sinA * sinB * sinphi) - circley * cosA * sinB;
            float y = circlex * (sinB * cosphi - sinA * cosB * sinphi) + circley * cosA * cosB;
            float z = K2 + cosA * circlex * sinphi + circley * sinA;
            float ooz = 1.0f / z; // one over z

            int xp = (int)(DONUT_SCREEN_WIDTH / 2.0f + K1 * ooz * x);
            int yp = (int)(DONUT_SCREEN_HEIGHT / 2.0f - K1 * ooz * y);

            float L = cosphi * costheta * sinB - cosA * costheta * sinphi - sinA * sintheta + cosB * (cosA * sintheta - costheta * sinA * sinphi);

            if (xp >= 0 && xp < DONUT_SCREEN_WIDTH && yp >=0 && yp < DONUT_SCREEN_HEIGHT) {
                if (ooz > z_buffer[xp + yp * DONUT_SCREEN_WIDTH]) {
                    z_buffer[xp + yp * DONUT_SCREEN_WIDTH] = ooz;
                    char luminance_char = '.'; // По умолчанию
                    // Ваша обновленная шкала яркости
                    if (L > 0) {
                        if (L > 0.8f) luminance_char = '@';
                        else if (L > 0.7f) luminance_char = '$';
                        else if (L > 0.6f) luminance_char = '#';
                        else if (L > 0.5f) luminance_char = '*';
                        else if (L > 0.4f) luminance_char = '!';
                        else if (L > 0.3f) luminance_char = '=';
                        else if (L > 0.2f) luminance_char = ';';
                        else if (L > 0.1f) luminance_char = ':';
                        else luminance_char = '-';
                    }
                    output_buffer[xp + yp * DONUT_SCREEN_WIDTH] = luminance_char;
                }
            }
        }
    }

    // Вывод буфера напрямую в vga_buffer
    // current_term_color теперь доступна благодаря extern в vga.h
    unsigned short color_attribute = (unsigned short)current_term_color << 8;
    for (int j = 0; j < DONUT_SCREEN_HEIGHT; j++) {
        for (int i = 0; i < DONUT_SCREEN_WIDTH; i++) {
            if (j < VGA_ROWS && i < VGA_COLS) { // Убедимся, что не выходим за пределы VGA
                // Сочетаем символ из output_buffer с атрибутом цвета
                 vga_buffer[(j * VGA_COLS) + i] = (unsigned short)(output_buffer[i + j * DONUT_SCREEN_WIDTH]) | color_attribute;
			}
        }
    }
}


void run_donut_benchmark() {
    // Сохраняем текущие цвета терминала
    // current_term_color (из vga.c, доступна через extern в vga.h) содержит (bg << 4) | fg
    enum vga_color old_fg = (enum vga_color)(current_term_color & 0x0F);
    enum vga_color old_bg = (enum vga_color)((current_term_color >> 4) & 0x0F);

    terminal_initialize(); // Очищаем экран стандартным цветом
    terminal_writestring("Запуск Furmark (ASCII Donut). Нажмите 'q' или ESC для выхода.\n");

    // Небольшая пауза, чтобы пользователь успел прочитать
    for(volatile int i=0; i < 10000000; ++i); // Пауза

    float A = 0, B = 0; // Углы вращения

    // Буферы для рендеринга (статическое выделение)
    static char screen_buf[DONUT_SCREEN_WIDTH * DONUT_SCREEN_HEIGHT];
    static float zbuf[DONUT_SCREEN_WIDTH * DONUT_SCREEN_HEIGHT];

    int frame_count = 0;
    // Устанавливаем цвет для пончика
    terminal_set_full_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    // current_term_color в vga.c теперь обновился и будет использоваться в render_donut_frame

    while (1) {
        render_donut_frame(A, B, screen_buf, zbuf);

        A += 0.04f; // Скорость вращения
        B += 0.02f;
        frame_count++;

        unsigned char key_scancode = keyboard_poll_scancode();
        if (key_scancode != 0) {
            char ascii_key = scancode_to_ascii(key_scancode);
            // Скан-код для ESC это 0x01
            if (ascii_key == 'q' || key_scancode == 0x01) {
                // Очищаем буфер клавиатуры от возможного "отпускания" клавиши или других оставшихся байт
                while(inb_local(KBD_STATUS_PORT_LOCAL) & KBD_OUTPUT_BUFFER_FULL_LOCAL) {
                    inb_local(KBD_DATA_PORT_LOCAL);
                }
                break; // Выход из цикла анимации
            }
            // Если нажата другая клавиша, также очистим буфер, чтобы она не влияла на следующий ввод в шелле
             while(inb_local(KBD_STATUS_PORT_LOCAL) & KBD_OUTPUT_BUFFER_FULL_LOCAL) {
                inb_local(KBD_DATA_PORT_LOCAL);
            }
        }
        // for(volatile int d = 0; d < 50000; ++d); // Раскомментируйте для замедления, если нужно
    }

    // Восстанавливаем старые цвета терминала
    terminal_set_full_color(old_fg, old_bg);
    terminal_initialize(); // Очищаем экран после пончика стандартным цветом
    terminal_writestring("Furmark завершен. Кадров: ");

    // Простой вывод числа кадров
    char frame_str[12]; // Максимальное значение для int (2^31-1) имеет 10 цифр + знак + null terminator
    int temp_frames = frame_count;
    int idx = 0;
    if (temp_frames == 0) {
        frame_str[idx++] = '0';
    } else {
        // Сохраняем цифры в обратном порядке
        char temp_buf[11];
        int temp_idx = 0;
        while(temp_frames > 0 && temp_idx < 10) {
            temp_buf[temp_idx++] = (temp_frames % 10) + '0';
            temp_frames /= 10;
        }
        // Копируем в правильном порядке (реверс)
        while(temp_idx > 0) {
            frame_str[idx++] = temp_buf[--temp_idx];
        }
    }
    frame_str[idx] = '\0';
    terminal_writestring(frame_str);
    terminal_putchar('\n');
}

// vga.c - Убедитесь, что current_term_color определена глобально в этом файле
// (это уже должно быть так из предыдущей версии)
// Пример того, как это должно выглядеть в vga.c:

// #include "vga.h" // Теперь vga.h объявляет 'extern unsigned char current_term_color;'

// Указатель на видеопамять в текстовом режиме VGA
/*volatile unsigned short* vga_buffer = (unsigned short*)0xB8000; // Definition is in vga.c, extern in vga.h
// Размеры экрана VGA
const int VGA_COLS_CONST = 80; // Используем другое имя, чтобы избежать конфликта с макросом VGA_COLS из donut.c, если он там был
const int VGA_ROWS_CONST = 25; // Аналогично

// Текущая позиция курсора
int term_col = 0;
int term_row = 0;

// ОПРЕДЕЛЕНИЕ глобальной переменной current_term_color
unsigned char current_term_color = (VGA_COLOR_BLACK << 4) | VGA_COLOR_WHITE; // Definition is in vga.c, extern in vga.h


static inline unsigned char make_vga_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

static inline unsigned short make_vga_entry(char c, unsigned char color_attr) {
    return (unsigned short)c | ((unsigned short)color_attr << 8);
}

void terminal_initialize() {
    // Устанавливаем стандартный цвет при инициализации
    current_term_color = make_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    for (int row = 0; row < VGA_ROWS; ++row) { // Используем VGA_ROWS
        for (int col = 0; col < VGA_COLS; ++col) { // Используем VGA_COLS
            const int index = row * VGA_COLS + col;
            vga_buffer[index] = make_vga_entry(' ', current_term_color);
        }
    }
    term_col = 0;
    term_row = 0;
}

void terminal_set_fg_color(enum vga_color color) {
    unsigned char bg = (current_term_color >> 4) & 0x0F; // Извлекаем текущий фон
    current_term_color = make_vga_color(color, (enum vga_color)bg);
}

void terminal_set_bg_color(enum vga_color color) {
    unsigned char fg = current_term_color & 0x0F; // Извлекаем текущий текст
    current_term_color = make_vga_color((enum vga_color)fg, color);
}

void terminal_set_full_color(enum vga_color fg, enum vga_color bg) {
    current_term_color = make_vga_color(fg, bg);
}

void terminal_scroll() {
    for (int row = 1; row < VGA_ROWS; ++row) { // Используем VGA_ROWS
        for (int col = 0; col < VGA_COLS; ++col) { // Используем VGA_COLS
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
    if (row < 0 || row >= VGA_ROWS) return; // Используем VGA_ROWS
    for (int col = 0; col < VGA_COLS; ++col) { // Используем VGA_COLS
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
            const int index = term_row * VGA_COLS + term_col; // Используем VGA_COLS
            vga_buffer[index] = make_vga_entry(' ', current_term_color);
        } else if (term_row > 0) {
            term_row--;
            term_col = VGA_COLS - 1; // Используем VGA_COLS
        }
    } else {
        if (term_row >= VGA_ROWS) { // Используем VGA_ROWS
            terminal_scroll();
        }
        const int index = term_row * VGA_COLS + term_col; // Используем VGA_COLS
        vga_buffer[index] = make_vga_entry(c, current_term_color);
        term_col++;
    }

    if (term_col >= VGA_COLS) { // Используем VGA_COLS
        term_col = 0;
        term_row++;
    }
    if (term_row >= VGA_ROWS) { // Используем VGA_ROWS
        terminal_scroll();
    }
}

void terminal_writestring(const char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        terminal_putchar(str[i]);
    }
}*/
