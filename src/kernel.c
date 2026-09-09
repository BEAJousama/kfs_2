#include "../include/kernel.h"


void kernel_main(void)
{

	gdt_install();
	idt_install();
	isrs_install();
	irq_install();
	timer_install();
    keyboard_install();
	__asm__ __volatile__("sti");
    init_terminal();
    kprintf("KFS_2 booted. Press ESC to open the debug shell.\n");
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
