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

    for (int i = 0; i < 7; i++) {
        unsigned int base = (gdt[i].base_high << 24) |
                            (gdt[i].base_middle << 16) |
                            gdt[i].base_low;
        unsigned int limit = (gdt[i].limit_low |
                             ((gdt[i].granularity & 0x0F) << 16));

        unsigned char access = gdt[i].access;
        unsigned char gran   = gdt[i].granularity;

        int present = (access >> 7) & 1;
        int dpl     = (access >> 5) & 3;
        int s       = (access >> 4) & 1;
        int type    = (access & 0x0F);

        int g = (gran >> 7) & 1;
        int db = (gran >> 6) & 1;
        int l = (gran >> 5) & 1;

        kprintf("Entry %d:\n", i);
        kprintf("  Base:  0x%08X\n", base);
        kprintf("  Limit: 0x%05X (%s)\n", limit, g ? "4KB pages" : "bytes");
        kprintf("  Present: %s\n", present ? "Yes" : "No");
        kprintf("  DPL: %d (Ring %d)\n", dpl, dpl);
        kprintf("  Descriptor type: %s\n", s ? "Code/Data" : "System");

        if (s) {
            if (type & 0x08) // Executable bit
                kprintf("  Segment: Code (%s)\n", (type & 0x02) ? "Readable" : "Exec only");
            else
                kprintf("  Segment: Data (%s)\n", (type & 0x02) ? "Writable" : "Read-only");
        }

        kprintf("  Default size: %s\n", db ? "32-bit" : "16-bit");
        if (l) kprintf("  64-bit code segment enabled\n");
        kprintf("  Granularity flag: %s\n", g ? "4KB pages" : "Byte limit");
        kprintf("\n");
    }
}

void push_values() {
    __asm__ volatile(
        "push $0x11111111\n\t"
        "push $0x22222222\n\t"
        "push $0x33333333\n\t"
        "push $0x44444444\n\t"
		"push $0x55555555\n\t"
		"push $0x66666666\n\t"
		"push $0x77777777\n\t"
		"push $0x88888888\n\t"
		"push $0x99999999\n\t"
		"push $0xAAAAAAAA\n\t"
		"push $0xBBBBBBBB\n\t"
		"push $0xCCCCCCCC\n\t"
		"push $0xDDDDDDDD\n\t"
    );
}

void print_kernel_stack() 
{
	push_values();
	uint32_t *stack_ptr = (uint32_t *)get_stack_pointer();
	uint32_t *base_ptr = (uint32_t *)get_base_pointer();

	kprintf("Kernel Stack Pointer: 0x%08X\n", (unsigned int)stack_ptr);
	kprintf("Base Pointer: 0x%08X\n", (unsigned int)base_ptr);
	kprintf("Kernel Stack Contents:\n");

		kprintf("Address        |         Value\n");
		kprintf("---------------|----------------\n");
	for (int i = 0; i < 30; i++) {
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
