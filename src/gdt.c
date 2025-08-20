#include "../include/kernel.h"

struct gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

struct gdt_entry gdt[7];
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

void print_gdt_info() 
{
	kprintf("GDT Information:\n");
	kprintf("---------------\n");
	kprintf("GDT Base: 0x%08X\n", gp.base);
	kprintf("GDT Limit: 0x%04X\n", gp.limit);
	char *segment_types[] = {
		"Null Descriptor",
		"Kernel Code Segment",
		"Kernel Data Segment",
		"Kernel Stack Segment",
		"User Code Segment",
		"User Data Segment",
		"User Stack Segment"
	};
	
	for (int i = 0; i < 7; i++) {
		kprintf("Entry %d:\n", i);
		kprintf("  Type: %s\n", segment_types[i]);
		kprintf("  Base: 0x%02X%02X%02X\n", gdt[i].base_high, gdt[i].base_middle, gdt[i].base_low);
		kprintf("  Limit: 0x%04X\n", gdt[i].limit_low | (gdt[i].granularity & 0x0F) << 16);
		kprintf("  Access: 0x%02X\n", gdt[i].access);
		kprintf("  Granularity: 0x%02X\n", gdt[i].granularity);
	}
}


void print_kernel_stack() 
{
	uint32_t *stack_ptr = (uint32_t *)get_stack_pointer();
	uint32_t *base_ptr = (uint32_t *)get_base_pointer();

	kprintf("Kernel Stack Pointer: 0x%08X\n", (unsigned int)stack_ptr);
	kprintf("Base Pointer: 0x%08X\n", (unsigned int)base_ptr);
	// gp address of the stack pointer
	kprintf("GP: 0x%08X\n", (unsigned int)&gp);
	kprintf("Kernel Stack Contents:\n");

		kprintf("Address        |         Value\n");
		kprintf("---------------|----------------\n");
	for (int i = 0; i < 10; i++) {
		kprintf("0x%08X     |  0x%08X\n", stack_ptr + i , stack_ptr[i]);
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

void gdt_install() 
{

	gp.limit = (sizeof(struct gdt_entry) * 7) - 1;
	gp.base = (unsigned int)&gdt;
	
    gdt_set_gate(0, 0, 0, 0, 0); // null descriptor
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data segment
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0x96, 0xCF); // kernel stack segment

    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user code segment
    gdt_set_gate(5, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user data segment
	gdt_set_gate(6, 0, 0xFFFFFFFF, 0xF6, 0xCF); // user stack segment

	gdt_flush();
}
