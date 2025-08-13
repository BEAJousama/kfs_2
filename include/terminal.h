#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>

void update_cursor(size_t row, size_t col);
void scroll_screen(int screen_id);
void refresh_display(void);
void terminal_init_multiscreen(int num_screens);
int terminal_switch_screen(int screen_id);
int terminal_get_current_screen(void);
void terminal_putchar(char c);
void terminal_putchar_screen(int screen_id, char c);
void terminal_writestring_screen(int screen_id, const char* data);
void init_terminal(void);

#endif
