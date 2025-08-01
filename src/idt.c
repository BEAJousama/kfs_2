#include "kernel.h"

struct idt_entry {
	unsigned short base_lo;    // Lower 16 bits of the address to jump to when this interrupt fires
	unsigned short sel;         // Kernel segment selector
	unsigned char always0;      // This must always be zero
	unsigned char flags;        // Flags
	unsigned short base_hi;   // Upper 16 bits of the address to jump to
} __attribute__((packed));

struct idt_ptr {
	unsigned short limit; // Size of the IDT - 1
	unsigned int base;   // Address of the first IDT entry
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void idt_load();

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) 
{
	/* The interrupt routine's base address */
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    /* The segment or 'selector' that this IDT entry will use
    *  is set here, along with any access flags */
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_install() 
{
	idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
	idtp.base = (unsigned int)idt;

	// // Clear the IDT
	memset(&idt, 0, sizeof(struct idt_entry) * 256);

	// Load the IDT
	idt_load();
}