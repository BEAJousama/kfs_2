#include "kernel.h"

struct vconsole {
	char buffer[80 * 25 * 2]; // 80x25 characters
	int cursor_x, cursor_y; // Cursor position
	uint8_t fg, bg; // Foreground and background colors
	bool active; // Is this console active?
} vcons[NUM_VCONSOLES];

int curent_vc = 0;

void switch_vconsole(int id) {
	kprintf("Switching to VC %d\n", id);
	if (id < 0 || id >= NUM_VCONSOLES) {
		return;
	}
	memcpy(vcons[curent_vc].buffer, VGA_MEMORY, sizeof(vcons[0].buffer));
	vcons[curent_vc].cursor_x = terminal_column;
	vcons[curent_vc].cursor_y = terminal_row;

	curent_vc = id;
	memcpy(VGA_MEMORY, vcons[id].buffer, sizeof(vcons[id].buffer));
	terminal_column = vcons[id].cursor_x;
	terminal_row = vcons[id].cursor_y;
	terminal_color = vga_entry_color(vcons[id].fg, vcons[id].bg);

	terminal_writestring("Switched to VC:");
	terminal_putchar('1' + id);
	terminal_putchar('\n');
}