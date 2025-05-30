# Makefile для сборки минимального ядра (многофайловая версия с Furmark)

# Используемые программы
AS = nasm
CC = gcc
LD = ld
QEMU = qemu-system-i386

# Флаги для компиляции и ассемблирования
ASFLAGS = -f elf32
# Добавляем -lm для математической библиотеки, хотя в freestanding это может потребовать libgcc
# GCC должен предоставлять встроенные реализации для float операций
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -nostdlib -Iinclude
LDFLAGS = -m elf_i386 -T linker.ld
QEMUFLAGS = -kernel myos.bin

# Исходные файлы
ASM_SOURCES = boot.s
C_SOURCES = kernel.c vga.c keyboard.c string_utils.c shell.c donut.c math_utils.c
OBJ_FILES = $(ASM_SOURCES:.s=.o) $(C_SOURCES:.c=.o)

# Имя выходного файла ядра
KERNEL_BIN = myos.bin

# Цель по умолчанию: собрать ядро
all: $(KERNEL_BIN)

# Правило для сборки бинарного файла ядра
# Линкуем с libgcc, так как она может содержать реализации для float операций,
# которые компилятор может использовать в freestanding режиме.
$(KERNEL_BIN): $(OBJ_FILES) linker.ld
	# Use gcc as the linker driver. It will correctly find and link the 32-bit libgcc.
	$(CC) -m32 -nostdlib -Tlinker.ld -o $(KERNEL_BIN) $(OBJ_FILES) -lgcc -lm

# Правило для компиляции C-файлов
%.o: %.c $(wildcard include/*.h) Makefile.mk
	$(CC) $(CFLAGS) -c $< -o $@

# Правило для ассемблирования .s файлов
%.o: %.s Makefile.mk
	$(AS) $(ASFLAGS) $< -o $@

# Запустить ядро в QEMU
run: all
	$(QEMU) $(QEMUFLAGS)

# Создать директорию include, если ее нет
setup_dirs:
	mkdir -p include

# Очистить сборочные файлы
clean:
	rm -f $(OBJ_FILES) 

.PHONY: all run clean setup_dirs
