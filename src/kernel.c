#include "kernel.h"

void kernel_main(void) {

	gdt_install();
	idt_install();
	isrs_install();
	irq_install();
    keyboard_install();

	__asm__ __volatile__("sti"); 

    terminal_init_multiscreen(4);
    terminal_writestring_screen(0, "=== KFS_2: GDT & Stack ===\n");
    terminal_writestring_screen(0, "Kernel From Scratch - Assignment 2\n");
    terminal_writestring_screen(0, "==================================\n");
    terminal_writestring_screen(0, "Navigation:\n");
    terminal_writestring_screen(0, "- Alt+Left/Right: Previous/Next screen\n");
    terminal_writestring_screen(0, "- Alt+Up: Go to first screen (0)\n");
    terminal_writestring_screen(0, "- Alt+Down: Go to last screen (3)\n");
    terminal_writestring_screen(0, "- ESC: Toggle debug shell\n\n");
    
    terminal_writestring_screen(1, "Screen 1: Debug Console (Green)\n");
    terminal_writestring_screen(1, "================================\n");
    terminal_writestring_screen(1, "GDT Status: ");
    kprintf_screen(1, "Initialized at 0x%x\n", 0x00000800);
    terminal_writestring_screen(1, "Stack monitoring active\n");
    terminal_writestring_screen(1, "Use ESC to access debug shell\n\n");
    
    terminal_writestring_screen(2, "Screen 2: System Monitor (Cyan)\n");
    terminal_writestring_screen(2, "================================\n");
    terminal_writestring_screen(2, "Memory segments configured:\n");
    terminal_writestring_screen(2, "- Kernel Code: 0x08\n");
    terminal_writestring_screen(2, "- Kernel Data: 0x10\n");
    terminal_writestring_screen(2, "- Kernel Stack: 0x18\n");
    terminal_writestring_screen(2, "- User Code: 0x20\n");
    terminal_writestring_screen(2, "- User Data: 0x28\n\n");
    
    terminal_writestring_screen(3, "Screen 3: Stack Information (Red)\n");
    terminal_writestring_screen(3, "==================================\n");
    kprintf_screen(3, "Current ESP: 0x%x\n", get_stack_pointer());
    kprintf_screen(3, "Current EBP: 0x%x\n", get_base_pointer());
    terminal_writestring_screen(3, "Stack grows downward\n");
    terminal_writestring_screen(3, "Use 'stack' command in shell for details\n\n");

    // for (int i = 0; i < 4; i++) {
    //     terminal_writestring_screen(i, "Use Ctrl + Alt + { -> , <- } to switch screens\n");
    //     terminal_writestring_screen(i, "Type to test keyboard input\n");
    // }    
    
    // Start on screen 0
    terminal_switch_screen(0);
    // shell_init();
    
    kprintf("\nKFS_2 System Ready!\n");
    kprintf("GDT configured with 6 segments at address 0x00000800\n");
    kprintf("Stack monitoring enabled\n");
    kprintf("Press ESC for debug shell, Alt+Arrows to switch screens\n\n");
    kprintf("$ ");
    
    // Main kernel loop
    while (1) {
        // Increment uptime counter (simple timer simulation)
        static uint32_t timer_tick = 0;
        timer_tick++;
        if (timer_tick >= 1000000) { // Rough approximation
            increment_uptime();
            timer_tick = 0;
        }
        
        // Your kernel code here
        __asm__ volatile ("hlt"); // Halt until interrupt
    }
}
