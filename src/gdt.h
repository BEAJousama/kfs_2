#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// GDT entry structure
struct gdt_entry {
    uint16_t limit_low;      // Lower 16 bits of limit
    uint16_t base_low;       // Lower 16 bits of base
    uint8_t  base_middle;    // Next 8 bits of base
    uint8_t  access;         // Access flags
    uint8_t  granularity;    // Granularity and limit (bits 16-19)
    uint8_t  base_high;      // Last 8 bits of base
} __attribute__((packed));

// GDT pointer structure
struct gdt_ptr {
    uint16_t limit;          // Size of GDT - 1
    uint32_t base;           // Address of GDT
} __attribute__((packed));

// GDT segment selectors
#define GDT_KERNEL_CODE_SEGMENT  0x08  // Kernel code segment selector
#define GDT_KERNEL_DATA_SEGMENT  0x10  // Kernel data segment selector
#define GDT_KERNEL_STACK_SEGMENT 0x18  // Kernel stack segment selector
#define GDT_USER_CODE_SEGMENT    0x20  // User code segment selector
#define GDT_USER_DATA_SEGMENT    0x28  // User data segment selector
#define GDT_USER_STACK_SEGMENT   0x30  // User stack segment selector

// Access byte flags
#define GDT_ACCESS_PRESENT       0x80  // Present bit
#define GDT_ACCESS_PRIVL_RING0   0x00  // Ring 0 (kernel)
#define GDT_ACCESS_PRIVL_RING3   0x60  // Ring 3 (user)
#define GDT_ACCESS_EXECUTABLE    0x08  // Executable bit
#define GDT_ACCESS_DIRECTION     0x04  // Direction bit (0 = grow up, 1 = grow down)
#define GDT_ACCESS_RW            0x02  // Read/Write bit
#define GDT_ACCESS_ACCESSED      0x01  // Accessed bit

// Granularity byte flags
#define GDT_GRAN_4K              0x80  // 4KB granularity
#define GDT_GRAN_32BIT           0x40  // 32-bit selector
#define GDT_GRAN_16BIT           0x00  // 16-bit selector

// Function declarations
void gdt_init(void);
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

// Global GDT pointer (used by assembly code)
extern struct gdt_ptr gp;

// Stack functions
void print_kernel_stack(void);
void print_stack_info(void);
void print_gdt_info(void);
uint32_t get_stack_pointer(void);
uint32_t get_base_pointer(void);

#endif // GDT_H