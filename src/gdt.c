#include "kernel.h"

struct gdt_entry {
	uint16_t limit_low;    // Lower 16 bits of the limit
	uint16_t base_low;     // Lower 16 bits of the base address
	uint8_t base_middle;   // Next 8 bits of the base address
	uint8_t access;        // Access flags, determine what ring this segment can be used in
	uint8_t granularity;   // Granularity and upper 4 bits of the limit
	uint8_t base_high;     // Last 8 bits of the base address
} __attribute__((packed));

struct gdt_ptr {
	unsigned short limit; // Size of the GDT - 1
	unsigned int base;  // Address of the first GDT entry
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gp;

extern void gdt_flush();

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char granularity) 
{
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);
	gdt[num].granularity |= (granularity & 0xF0);
	gdt[num].access = access;
}

void gdt_install() 
{
	gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
	gp.base = (unsigned int)&gdt;

	// Null segment
	gdt_set_gate(0, 0, 0, 0, 0);

	// Code segment
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	// Data segment
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	// Flush the GDT
	gdt_flush();
}

void print_gdt_info() 
{
	kprintf("GDT Information:\n");
	kprintf("---------------\n");
	kprintf("GDT Base: 0x%08X\n", gp.base);
	kprintf("GDT Limit: 0x%04X\n", gp.limit);
	
	for (int i = 0; i < 3; i++) {
		kprintf("Entry %d:\n", i);
		kprintf("  Base: 0x%02X%02X%02X\n", gdt[i].base_high, gdt[i].base_middle, gdt[i].base_low);
		kprintf("  Limit: 0x%04X\n", gdt[i].limit_low | (gdt[i].granularity & 0x0F) << 16);
		kprintf("  Access: 0x%02X\n", gdt[i].access);
		kprintf("  Granularity: 0x%02X\n", gdt[i].granularity);
	}
}

void print_kernel_stack() 
{
	uint32_t *stack_ptr = (uint32_t *)get_stack_pointer();
	kprintf("Kernel Stack Pointer: 0x%08X\n", (unsigned int)stack_ptr);
	kprintf("Kernel Stack Contents:\n");
	
	for (int i = 0; i < 16; i++) {
		kprintf("  0x%08X\n", stack_ptr[i]);
	}
}

uint32_t get_stack_pointer() 
{
	uint32_t sp;
	__asm__ volatile("mov %%esp, %0" : "=r"(sp));
	return sp;
}

uint32_t get_base_pointer() 
{
	uint32_t bp;
	__asm__ volatile("mov %%ebp, %0" : "=r"(bp));
	return bp;
}

void print_stack_info() 
{
	uint32_t sp = get_stack_pointer();
	uint32_t bp = get_base_pointer();
	
	kprintf("Stack Pointer: 0x%08X\n", sp);
	kprintf("Base Pointer: 0x%08X\n", bp);
	kprintf("Current Stack Frame:\n");
	
	// Print the current stack frame
	for (int i = 0; i < 8; i++) {
		kprintf("  0x%08X\n", *(uint32_t *)(bp + (i * sizeof(uint32_t))));
	}
}

void gdt_init() 
{
	gdt_install();
	print_gdt_info();
	print_kernel_stack();
	print_stack_info();
}



