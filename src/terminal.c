#include "kernel.h"

// Screen structure
typedef struct {
    size_t row;
    size_t column;
    uint8_t color;
    uint16_t buffer[VGA_HEIGHT * VGA_WIDTH];
} screen_t;

// Global state
static screen_t screens[MAX_SCREENS];
static int current_screen = 0;
static int num_screens = 1;
static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;

// Utility functions
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static void update_cursor(size_t row, size_t col) {
    uint16_t pos = row * VGA_WIDTH + col;
    
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) (pos >> 8));
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
}

static void scroll_screen(int screen_id) {
    screen_t* screen = &screens[screen_id];
    
    // Move all lines up by one
    for (size_t i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        screen->buffer[i] = screen->buffer[i + VGA_WIDTH];
    }
    
    // Clear the last line
    for (size_t i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        screen->buffer[i] = vga_entry(' ', screen->color);
    }
    
    screen->row = VGA_HEIGHT - 1;
}

static void refresh_display(void) {
    // Copy current screen buffer to VGA memory
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = screens[current_screen].buffer[i];
    }
    update_cursor(screens[current_screen].row, screens[current_screen].column);
}

void terminal_initialize(void) {
    screens[0].row = 0;
    screens[0].column = 0;
    screens[0].color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Clear screen 0
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        screens[0].buffer[i] = vga_entry(' ', screens[0].color);
        vga_buffer[i] = screens[0].buffer[i];
    }
    
    update_cursor(0, 0);
}

void terminal_init_multiscreen(int requested_screens) {
    if (requested_screens > MAX_SCREENS) requested_screens = MAX_SCREENS;
    num_screens = requested_screens;
    
    for (int s = 0; s < num_screens; s++) {
        screens[s].row = 0;
        screens[s].column = 0;
        
        // Set different colors for each screen
        switch (s) {
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
        
        // Clear each screen buffer
        for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
            screens[s].buffer[i] = vga_entry(' ', screens[s].color);
        }
    }
    
    current_screen = 0;
    refresh_display();
}

int terminal_switch_screen(int screen_id) {
    if (screen_id < 0 || screen_id >= num_screens) return -1;
    
    current_screen = screen_id;
    refresh_display();
    return 0;
}

int terminal_get_current_screen(void) {
    return current_screen;
}

void terminal_putchar(char c) {
    terminal_putchar_screen(current_screen, c);
}

void terminal_putchar_screen(int screen_id, char c) {
    if (screen_id < 0 || screen_id >= num_screens) return;
    
    screen_t* screen = &screens[screen_id];
    
    if (c == '\n') {
        screen->column = 0;
        screen->row++;
    } else if (c == '\b') {
        // Handle backspace - move cursor back and clear character
        if (screen->column > 0) {
            screen->column--;
        } else if (screen->row > 0) {
            // Move to end of previous line
            screen->row--;
            screen->column = VGA_WIDTH - 1;
            // Find the actual end of text on the previous line
            while (screen->column > 0) {
                size_t index = screen->row * VGA_WIDTH + screen->column;
                if ((screen->buffer[index] & 0xFF) != ' ') {
                    screen->column++;
                    break;
                }
                screen->column--;
            }
        }
        // Clear the character at cursor position
        size_t index = screen->row * VGA_WIDTH + screen->column;
        screen->buffer[index] = vga_entry(' ', screen->color);
    } else if (c == '\t') {
        // Tab to next 4-character boundary
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
    } else if (c == '\r') {
        // Carriage return - go to beginning of line
        screen->column = 0;
    } else if (c >= 32 && c <= 126) {
        // Only handle printable ASCII characters
        size_t index = screen->row * VGA_WIDTH + screen->column;
        screen->buffer[index] = vga_entry(c, screen->color);
        screen->column++;
        
        if (screen->column >= VGA_WIDTH) {
            screen->column = 0;
            screen->row++;
        }
    }
    
    if (screen->row >= VGA_HEIGHT) {
        scroll_screen(screen_id);
    }
    
    // If this is the current screen, update display
    if (screen_id == current_screen) {
        refresh_display();
    }
}

void terminal_writestring(const char* data) {
    terminal_writestring_screen(current_screen, data);
}

void terminal_writestring_screen(int screen_id, const char* data) {
    for (size_t i = 0; data[i]; i++) {
        terminal_putchar_screen(screen_id, data[i]);
    }
}

void terminal_setcolor(uint8_t color) {
    screens[current_screen].color = color;
}

void terminal_setcolor_screen(int screen_id, uint8_t color) {
    if (screen_id >= 0 && screen_id < num_screens) {
        screens[screen_id].color = color;
    }
}

void terminal_clear(void) {
    terminal_clear_screen(current_screen);
}

void terminal_clear_screen(int screen_id) {
    if (screen_id < 0 || screen_id >= num_screens) return;
    
    screen_t* screen = &screens[screen_id];
    screen->row = 0;
    screen->column = 0;
    
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        screen->buffer[i] = vga_entry(' ', screen->color);
    }
    
    if (screen_id == current_screen) {
        refresh_display();
    }
}