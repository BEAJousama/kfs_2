#include "kernel.h"

// US QWERTY keyboard scancode to ASCII translation table
static unsigned char kbdus[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 0-9 */
    '9', '0', '-', '=', '\b',                           /* 10-14 */
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', /* 15-24 */
    'p', '[', ']', '\n',                                /* 25-28 */
    0,  /* Control key - 29 */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',  /* 30-39 */
    '\'', '`', 0, '\\',                                 /* 40-43 */
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',  /* 44-53 */
    0,  /* Right shift - 54 */
    '*', /* Keypad * - 55 */
    0,  /* Alt - 56 */
    ' ', /* Space - 57 */
    0,  /* Caps lock - 58 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* F1-F10 - 59-68 */
    0,  /* Num lock - 69 */
    0,  /* Scroll lock - 70 */
    0,  /* Home - 71 */
    0,  /* Up arrow - 72 */
    0,  /* Page up - 73 */
    '-', /* Keypad - - 74 */
    0,  /* Left arrow - 75 */
    0,  /* Keypad 5 - 76 */
    0,  /* Right arrow - 77 */
    '+', /* Keypad + - 78 */
    0,  /* End - 79 */
    0,  /* Down arrow - 80 */
    0,  /* Page down - 81 */
    0,  /* Insert - 82 */
    0,  /* Delete - 83 */
    0, 0, 0,  /* 84-86 */
    0, 0,     /* F11, F12 - 87-88 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 89-104 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 105-120 */
    0, 0, 0, 0, 0, 0, 0                               /* 121-127 */
};

// Shifted characters (when Shift is held)
static unsigned char kbdus_shift[128] = {
    0,  27, '!', '@', '#' , '%', '^', '&', '*',     /* 0-9 */
    '(', ')', '_', '+', '\b',                           /* 10-14 */
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', /* 15-24 */
    'P', '{', '}', '\n',                                /* 25-28 */
    0,  /* Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',  /* 30-39 */
    '"', '~', 0, '|',                                   /* 40-43 */
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',  /* 44-53 */
    0,  /* Right shift */
    '*', /* Keypad * */
    0,  /* Alt */
    ' ', /* Space */
    0,  /* Caps lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* F1-F10 */
    0,  /* Num lock */
    0,  /* Scroll lock */
    0,  /* Home */
    0,  /* Up arrow */
    0,  /* Page up */
    '-', /* Keypad - */
    0,  /* Left arrow */
    0,  /* Keypad 5 */
    0,  /* Right arrow */
    '+', /* Keypad + */
    0,  /* End */
    0,  /* Down arrow */
    0,  /* Page down */
    0,  /* Insert */
    0,  /* Delete */
    0, 0, 0,  /* 84-86 */
    0, 0,     /* F11, F12 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 89-104 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 105-120 */
    0, 0, 0, 0, 0, 0, 0                               /* 121-127 */
};

// Modifier key states
static bool ctrl_pressed = false;
static bool alt_pressed = false;
static bool shift_pressed = false;

// Extended scancode tracking
static bool extended_scancode = false;

static void clear_keyboard_buffer(void) {
    // Clear any pending keyboard data
    while (inb(0x64) & 0x01) {
        inb(0x60);
    }
}

// Add this for debugging - remove later if not needed
static void debug_scancode(unsigned char scancode) {
    // This will help debug what scancodes are being received
    // You can remove this function once everything works
    terminal_putchar('[');
    if (scancode < 10) {
        terminal_putchar('0' + scancode);
    } else if (scancode < 100) {
        terminal_putchar('0' + (scancode / 10));
        terminal_putchar('0' + (scancode % 10));
    } else {
        terminal_putchar('0' + (scancode / 100));
        terminal_putchar('0' + ((scancode / 10) % 10));
        terminal_putchar('0' + (scancode % 10));
    }
    terminal_putchar(']');
}

static void handle_screen_switch_arrows(unsigned char scancode) {
    int current = terminal_get_current_screen();
    int new_screen = current;
    
    switch (scancode) {
        case SC_LEFT_ARROW:
            // Previous screen (wrap around)
            new_screen = (current - 1 + MAX_SCREENS) % MAX_SCREENS;
            break;
        case SC_RIGHT_ARROW:
            // Next screen (wrap around)
            new_screen = (current + 1) % MAX_SCREENS;
            break;
        case SC_UP_ARROW:
            // Go to screen 0
            new_screen = 0;
            break;
        case SC_DOWN_ARROW:
            // Go to last screen
            new_screen = MAX_SCREENS - 1;
            break;
        default:
            return;
    }
    
    if (terminal_switch_screen(new_screen) == 0) {        
        // Add direction indicator
        switch (scancode) {
            case SC_LEFT_ARROW:
                break;
            case SC_RIGHT_ARROW:
                break;
            case SC_UP_ARROW:
                break;
            case SC_DOWN_ARROW:
                break;
        }
    }
}

void keyboard_handler(struct regs *r) {
    (void)r; // Suppress unused parameter warning
    
    unsigned char scancode = inb(0x60);
    
    // Handle extended scancode prefix (0xE0)
    if (scancode == 0xE0) {
        extended_scancode = true;
        return;
    }
    
    // Uncomment this line for debugging scancodes:
    // debug_scancode(scancode);
    
    // Handle key releases (bit 7 set)
    if (scancode & 0x80) {
        scancode &= 0x7F; // Remove release bit
        
        // Handle extended key releases
        if (extended_scancode) {
            extended_scancode = false;
            // Arrow keys released - no action needed
            return;
        }
        
        switch (scancode) {
            case SC_CTRL:
                ctrl_pressed = false;
                break;
            case SC_ALT:
                alt_pressed = false;
                break;
            case SC_SHIFT_L:
            case SC_SHIFT_R:
                shift_pressed = false;
                break;
        }
        return;
    }
    
    // Handle extended keys (arrow keys, etc.)
    if (extended_scancode) {
        extended_scancode = false;
        
        // Check for screen switching with Alt+Arrow keys
        if (alt_pressed) {
            switch (scancode) {
                case SC_LEFT_ARROW:
                case SC_RIGHT_ARROW:
                case SC_UP_ARROW:
                case SC_DOWN_ARROW:
                    handle_screen_switch_arrows(scancode);
                    return;
            }
        }
        
        // Handle other extended keys here if needed
        return;
    }
    
    // Handle regular key presses
    switch (scancode) {
        case SC_CTRL:
            ctrl_pressed = true;
            return;
        case SC_ALT:
            alt_pressed = true;
            return;
        case SC_SHIFT_L:
        case SC_SHIFT_R:
            shift_pressed = true;
            return;
        case SC_ESC:
            // Toggle shell on ESC key
            shell_toggle();
            return;
    }
    
    // Handle regular character input
    if (scancode < 128) {
        char c = 0;
        
        // Get the character from the appropriate table
        if (shift_pressed) {
            c = kbdus_shift[scancode];
        } else {
            c = kbdus[scancode];
        }
        
        // Send to shell if active, otherwise to terminal
        if (c != 0) {
            if (is_shell_active()) {
                shell_handle_key(c);
            } else {
                terminal_putchar(c);
            }
        }
    }
}

void keyboard_install(void) {
    // Clear any pending keyboard data before installing handler
    clear_keyboard_buffer();
    
    // Install the keyboard handler on IRQ1
    irq_install_handler(1, keyboard_handler);
}

// Modifier key states
// static bool ctrl_pressed = false;
// static bool alt_pressed = false;
// static bool shift_pressed = false;

// static void clear_keyboard_buffer(void) {
//     // Clear any pending keyboard data
//     while (inb(0x64) & 0x01) {
//         inb(0x60);
//     }
// }

// static void handle_screen_switch(unsigned char scancode) {
//     int screen_id = -1;
    
//     switch (scancode) {
//         case SC_F1: screen_id = 0; break;
//         case SC_F2: screen_id = 1; break;
//         case SC_F3: screen_id = 2; break;
//         case SC_F4: screen_id = 3; break;
//         default: return;
//     }
    
//     if (terminal_switch_screen(screen_id) == 0) {
//         // Show notification
//         terminal_writestring("[Screen ");
//         terminal_putchar('0' + screen_id);
//         terminal_writestring("]\n");
//     }
// }

// void keyboard_handler(struct regs *r) {
//     (void)r; // Suppress unused parameter warning
    
//     unsigned char scancode = inb(0x60);
    
//     // Handle key releases (bit 7 set)
//     if (scancode & 0x80) {
//         scancode &= 0x7F; // Remove release bit
        
//         switch (scancode) {
//             case SC_CTRL:
//                 ctrl_pressed = false;
//                 break;
//             case SC_ALT:
//                 alt_pressed = false;
//                 break;
//             case SC_SHIFT_L:
//             case SC_SHIFT_R:
//                 shift_pressed = false;
//                 break;
//         }
//         return;
//     }
    
//     // Handle key presses
//     switch (scancode) {
//         case SC_CTRL:
//             ctrl_pressed = true;
//             return;
//         case SC_ALT:
//             alt_pressed = true;
//             return;
//         case SC_SHIFT_L:
//         case SC_SHIFT_R:
//             shift_pressed = true;
//             return;
//     }
    
//     // Check for screen switching (Ctrl+Alt+F1-F4)
//     if (ctrl_pressed && alt_pressed) {
//         handle_screen_switch(scancode);
//         return;
//     }
    
//     // Handle regular character input
//     if (scancode < 128) {
//         char c;
        
//         if (shift_pressed && kbdus_shift[scancode] != 0) {
//             c = kbdus_shift[scancode];
//         } else {
//             c = kbdus[scancode];
//         }
        
//         if (c != 0) {
//             terminal_putchar(c);
//         }
//     }
// }

// void keyboard_install(void) {
//     // Clear any pending keyboard data before installing handler
//     clear_keyboard_buffer();
    
//     // Install the keyboard handler on IRQ1
//     irq_install_handler(1, keyboard_handler);
// }