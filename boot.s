; boot.s - простой загрузчик для нашего ядра (без изменений)

; Константы для Multiboot заголовка
MB_MAGIC  equ 0x1BADB002
MB_FLAGS  equ 0x00000003
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)

global _start
global multiboot_header

extern kmain
extern init_fpu ; Новая функция для инициализации FPU/SSE

; Indicate that the stack should not be executable.
section .note.GNU-stack noalloc noexec nowrite progbits

section .multiboot_header
align 4
multiboot_header:
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM
    dd 0 ; header_addr
    dd 0 ; load_addr
    dd 0 ; load_end_addr
    dd 0 ; bss_end_addr
    dd 0 ; entry_addr

section .text
bits 32

_start:
    ; Инициализация FPU/SSE перед вызовом C-кода, который может использовать float
    call init_fpu

    mov esp, stack_top
    call kmain

halt_loop:
    cli
    hlt
    jmp halt_loop

; Функция для инициализации FPU и SSE.
; Это важно, если C-код будет использовать операции с плавающей точкой.
init_fpu:
    ; Проверяем наличие FPU
    mov eax, 1
    cpuid
    test edx, 1 ; Проверяем бит FPU (бит 0)
    jz .no_fpu

    ; Инициализация FPU
    finit       ; Инициализирует FPU, сбрасывает управляющее слово, слово состояния и т.д.

    ; Включаем SSE (если доступно и необходимо)
    mov eax, cr0
    and ax, 0xFFFB ; Сбрасываем бит EM (эмуляция)
    or ax, 0x2     ; Устанавливаем бит MP (мониторинг)
    mov cr0, eax

    mov eax, cr4
    or ax, 0x0600 ; Устанавливаем биты OSFXSR (бит 9) и OSXMMEXCPT (бит 10)
    mov cr4, eax

.no_fpu:
    ret


section .bss
align 16
stack_bottom:
    resb 16384 ; 16KB for stack
stack_top: