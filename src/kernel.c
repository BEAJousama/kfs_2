#include "../include/kernel.h"


void kernel_main(void)
{

	gdt_install();
	idt_install();
	isrs_install();
	irq_install();
    keyboard_install();
	__asm__ __volatile__("sti");
    init_terminal();
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
