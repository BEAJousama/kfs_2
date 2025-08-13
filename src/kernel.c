#include "../include/kernel.h"


void stack_test() {
    uint32_t value = 0xBEEFCAFE;

    asm volatile("push %0" :: "r"(value));
    
    uint32_t esp;
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    
    kprintf_screen(1,"ESP = 0x%x, Value = 0x%x\n", esp, *((uint32_t*)esp));

    uint32_t popped;
    asm volatile("pop %0" : "=r"(popped));
    kprintf_screen(1, "Popped value: 0x%x\n", popped);
}


void test_user_stack() {
    uint32_t value = 0xDEADBEEF;

    asm volatile("push %0" :: "r"(value));
    
    uint32_t esp;
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    
    kprintf_screen(1,"User ESP = 0x%x, Value = 0x%x\n", esp, *((uint32_t*)esp));

    uint32_t popped;
    asm volatile("pop %0" : "=r"(popped));
    kprintf_screen(1, "User Popped value: 0x%x\n", popped);
}

void kernel_main(void)
{

	gdt_install();
	idt_install();
	isrs_install();
	irq_install();
    keyboard_install();
	__asm__ __volatile__("sti");
    init_terminal();
    stack_test();
    test_user_stack();
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
