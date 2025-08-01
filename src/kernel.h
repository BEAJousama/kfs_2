#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

// VGA and screen constants
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define MAX_SCREENS 4

// VGA Colors
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// Keyboard scancodes
#define SC_F1 0x3B
#define SC_F2 0x3C
#define SC_F3 0x3D
#define SC_F4 0x3E
#define SC_CTRL 0x1D
#define SC_ALT 0x38
#define SC_SHIFT_L 0x2A
#define SC_SHIFT_R 0x36
#define SC_ESC 0x01

// Arrow key scancodes
#define SC_UP_ARROW 0x48
#define SC_DOWN_ARROW 0x50
#define SC_LEFT_ARROW 0x4B
#define SC_RIGHT_ARROW 0x4D

// IRQ and interrupt structures
struct regs {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

// Port I/O functions
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// String functions (minimal implementations)
size_t strlen(const char* str);
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
int strcmp(const char* str1, const char* str2);
char* strcpy(char* dest, const char* src);

// Terminal functions
void terminal_initialize(void);
void terminal_init_multiscreen(int num_screens);
int terminal_switch_screen(int screen_id);
int terminal_get_current_screen(void);
void terminal_putchar(char c);
void terminal_putchar_screen(int screen_id, char c);
void terminal_writestring(const char* data);
void terminal_writestring_screen(int screen_id, const char* data);
void terminal_setcolor(uint8_t color);
void terminal_setcolor_screen(int screen_id, uint8_t color);
void terminal_clear(void);
void terminal_clear_screen(int screen_id);
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);

// Printf functions
void kputchar(char c);
void kputchar_screen(int screen_id, char c);
void kputs(const char* str);
void kputs_screen(int screen_id, const char* str);
int kprintf(const char* fmt, ...);
int kprintf_screen(int screen_id, const char* fmt, ...);

// Keyboard functions
void keyboard_install(void);
void keyboard_handler(struct regs *r);

// GDT functions
void gdt_init(void);
void print_gdt_info(void);

// Stack functions
void print_kernel_stack(void);
void print_stack_info(void);
uint32_t get_stack_pointer(void);
uint32_t get_base_pointer(void);

// Shell functions (bonus)
void shell_init(void);
void shell_handle_key(char c);
void shell_toggle(void);
bool is_shell_active(void);
void shell_prompt(void);
void increment_uptime(void);

// IRQ system functions (you need to implement these)
void irq_install_handler(int irq, void (*handler)(struct regs*));

// #define SC_SHIFT_L 0x2A
// #define SC_SHIFT_R 0x36

// // Arrow key scancodes
// #define SC_UP_ARROW 0x48
// #define SC_DOWN_ARROW 0x50
// #define SC_LEFT_ARROW 0x4B
// #define SC_RIGHT_ARROW 0x4D

// // IRQ and interrupt structures
// struct regs {
//     unsigned int gs, fs, es, ds;
//     unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
//     unsigned int int_no, err_code;
//     unsigned int eip, cs, eflags, useresp, ss;
// };

// // Port I/O functions
// static inline void outb(uint16_t port, uint8_t val) {
//     __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
// }

// static inline uint8_t inb(uint16_t port) {
//     uint8_t ret;
//     __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
//     return ret;
// }

// // String functions (minimal implementations)
// size_t strlen(const char* str);
// void* memcpy(void* dest, const void* src, size_t n);
// void* memset(void* s, int c, size_t n);

// // Terminal functions
// void terminal_initialize(void);
// void terminal_init_multiscreen(int num_screens);
// int terminal_switch_screen(int screen_id);
// int terminal_get_current_screen(void);
// void terminal_putchar(char c);
// void terminal_putchar_screen(int screen_id, char c);
// void terminal_writestring(const char* data);
// void terminal_writestring_screen(int screen_id, const char* data);
// void terminal_setcolor(uint8_t color);
// void terminal_setcolor_screen(int screen_id, uint8_t color);
// void terminal_clear(void);
// void terminal_clear_screen(int screen_id);
// uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);

// // Printf functions
// void kputchar(char c);
// void kputchar_screen(int screen_id, char c);
// void kputs(const char* str);
// void kputs_screen(int screen_id, const char* str);
// int kprintf(const char* fmt, ...);
// int kprintf_screen(int screen_id, const char* fmt, ...);

// // Keyboard functions
// void keyboard_install(void);
// void keyboard_handler(struct regs *r);

// // IRQ system functions (you need to implement these)
// void irq_install_handler(int irq, void (*handler)(struct regs*));

#endif // KERNEL_H