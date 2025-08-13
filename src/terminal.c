#include "../include/kernel.h"

typedef struct {
    size_t row;
    size_t column;
    uint8_t color;
    uint16_t buffer[VGA_HEIGHT * VGA_WIDTH];
} screen_t;

static screen_t screens[MAX_SCREENS];
static int current_screen = 0;
static int num_screens = 1;
static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

void update_cursor(size_t row, size_t col)
{
    uint16_t pos = row * VGA_WIDTH + col;
    
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) (pos >> 8));
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
}

void scroll_screen(int screen_id)
{
    screen_t* screen = &screens[screen_id];
    
    for (size_t i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
    {
        screen->buffer[i] = screen->buffer[i + VGA_WIDTH];
    }
    
    for (size_t i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
    {
        screen->buffer[i] = vga_entry(' ', screen->color);
    }
    
    screen->row = VGA_HEIGHT - 1;
}

void refresh_display(void)
{
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        vga_buffer[i] = screens[current_screen].buffer[i];
    }
    update_cursor(screens[current_screen].row, screens[current_screen].column);
}

void terminal_init_multiscreen(int requested_screens)
{
    if (requested_screens > MAX_SCREENS) requested_screens = MAX_SCREENS;
    num_screens = requested_screens;
    
    for (int s = 0; s < num_screens; s++)
    {
        screens[s].row = 0;
        screens[s].column = 0;

        switch (s)
        {
            case 0:
                screens[s].color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                break;
            case 1:
                screens[s].color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
                break;
            case 2:
                screens[s].color = vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
                break;
            case 3:
                screens[s].color = vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
                break;
            default:
                screens[s].color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                break;
        }
        for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        {
            screens[s].buffer[i] = vga_entry(' ', screens[s].color);
        }
    }
    
    current_screen = 0;
    refresh_display();
}

int terminal_switch_screen(int screen_id)
{
    if (screen_id < 0 || screen_id >= num_screens) return -1;
    
    current_screen = screen_id;
    refresh_display();
    return 0;
}

int terminal_get_current_screen(void)
{
    return current_screen;
}

void terminal_putchar(char c)
{
    terminal_putchar_screen(current_screen, c);
}

void terminal_putchar_screen(int screen_id, char c)
{
    if (screen_id < 0 || screen_id >= num_screens) return;
    
    screen_t* screen = &screens[screen_id];
    
    if (c == '\n') 
    {
        screen->column = 0;
        screen->row++;
    } 
    else if (c == '\b')
    {
        if (screen->column > 0)
        {
            screen->column--;
        }
        else if (screen->row > 0)
        {
            screen->row--;
            screen->column = VGA_WIDTH - 1;
            while (screen->column > 0)
            {
                size_t index = screen->row * VGA_WIDTH + screen->column;
                if ((screen->buffer[index] & 0xFF) != ' ')
                {
                    screen->column++;
                    break;
                }
                screen->column--;
            }
        }
        size_t index = screen->row * VGA_WIDTH + screen->column;
        screen->buffer[index] = vga_entry(' ', screen->color);
    }
    else if (c == '\t')
    {
        do {
            size_t index = screen->row * VGA_WIDTH + screen->column;
            screen->buffer[index] = vga_entry(' ', screen->color);
            screen->column++;
            if (screen->column >= VGA_WIDTH) {
                screen->column = 0;
                screen->row++;
                break;
            }
        } while (screen->column % 4 != 0);
    }
    else if (c == '\r')
    {
        screen->column = 0;
    }
    else if (c >= 32 && c <= 126)
    {
        size_t index = screen->row * VGA_WIDTH + screen->column;
        screen->buffer[index] = vga_entry(c, screen->color);
        screen->column++;
        
        if (screen->column >= VGA_WIDTH) {
            screen->column = 0;
            screen->row++;
        }
    }
    if (screen->row >= VGA_HEIGHT)
    {
        scroll_screen(screen_id);
    }
    if (screen_id == current_screen)
    {
        refresh_display();
    }
}

void terminal_writestring_screen(int screen_id, const char* data)
{
    for (size_t i = 0; data[i]; i++) {
        terminal_putchar_screen(screen_id, data[i]);
    }
}

void init_terminal(void)
{
    terminal_init_multiscreen(4);
    kprintf_screen(0, "Screen 0: Hello World %d!!!!\n", 42);
    kprintf_screen(1, "Screen 1: Hello World %d!!!!\n", 42);
    kprintf_screen(2, "Screen 2: Hello World %d!!!!\n", 42);
    kprintf_screen(3, "Screen 3: Hello World %d!!!!\n", 42);
    for (int i = 0; i < 4; i++)
    {
        kprintf_screen(i, "Screen %d: Use SHIFT + ALT + { -> , <- } to switch screens\n", i);
        kprintf_screen(i, "Screen %d: Type to test keyboard input\n", i);
    }
    terminal_switch_screen(0);
}