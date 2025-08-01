#include "kernel.h"

// Global context for printf operations
static int default_print_screen = -1; // -1 means use current active screen

void kputchar(char c) {
    if (default_print_screen >= 0) {
        terminal_putchar_screen(default_print_screen, c);
    } else {
        terminal_putchar(c);
    }
}

void kputchar_screen(int screen_id, char c) {
    terminal_putchar_screen(screen_id, c);
}

void kputs(const char* str) {
    for (size_t i = 0; str[i]; i++) {
        kputchar(str[i]);
    }
}

void kputs_screen(int screen_id, const char* str) {
    for (size_t i = 0; str[i]; i++) {
        kputchar_screen(screen_id, str[i]);
    }
}

static void reverse_string(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static int int_to_string(int num, char* str, int base) {
    int i = 0;
    bool is_negative = false;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }
    
    if (num < 0 && base == 10) {
        is_negative = true;
        num = -num;
    }
    
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

static int uint_to_string(unsigned int num, char* str, int base) {
    int i = 0;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }
    
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
    
    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

static int kvprintf(void (*putc)(char), const char* fmt, va_list ap) {
    int count = 0;
    char buf[32];
    
    for (const char* p = fmt; *p; p++) {
        if (*p != '%') {
            putc(*p);
            count++;
            continue;
        }
        
        p++; // Skip '%'
        bool zero_pad = false;
        int width = 0;
        
        // Parse flags
        if (*p == '0') {
            zero_pad = true;
            p++;
        }
        
        // Parse width
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            p++;
        }
        
        char spec = *p;
        switch (spec) {
            case 'd': {
                int val = va_arg(ap, int);
                int len = int_to_string(val, buf, 10);
                
                // Handle padding
                while (len < width) {
                    putc(zero_pad ? '0' : ' ');
                    count++;
                    width--;
                }
                
                for (int i = 0; i < len; i++) {
                    putc(buf[i]);
                    count++;
                }
                break;
            }
            case 'u': {
                unsigned int val = va_arg(ap, unsigned int);
                int len = uint_to_string(val, buf, 10);
                
                // Handle padding
                while (len < width) {
                    putc(zero_pad ? '0' : ' ');
                    count++;
                    width--;
                }
                
                for (int i = 0; i < len; i++) {
                    putc(buf[i]);
                    count++;
                }
                break;
            }
            case 'x': {
                unsigned int val = va_arg(ap, unsigned int);
                int len = uint_to_string(val, buf, 16);
                
                // Handle padding
                while (len < width) {
                    putc(zero_pad ? '0' : ' ');
                    count++;
                    width--;
                }
                
                for (int i = 0; i < len; i++) {
                    putc(buf[i]);
                    count++;
                }
                break;
            }
            case 'X': {
                unsigned int val = va_arg(ap, unsigned int);
                int len = uint_to_string(val, buf, 16);
                
                // Convert to uppercase
                for (int i = 0; i < len; i++) {
                    if (buf[i] >= 'a' && buf[i] <= 'f') {
                        buf[i] = buf[i] - 'a' + 'A';
                    }
                }
                
                // Handle padding
                while (len < width) {
                    putc(zero_pad ? '0' : ' ');
                    count++;
                    width--;
                }
                
                for (int i = 0; i < len; i++) {
                    putc(buf[i]);
                    count++;
                }
                break;
            }
            case 's': {
                const char* s = va_arg(ap, const char*);
                if (!s) s = "(null)";
                
                int str_len = 0;
                const char* temp = s;
                while (*temp++) str_len++; // Calculate string length
                
                // Handle width padding
                while (str_len < width) {
                    putc(' ');
                    count++;
                    width--;
                }
                
                while (*s) {
                    putc(*s++);
                    count++;
                }
                break;
            }
            case 'c': {
                char c = (char)va_arg(ap, int);
                putc(c);
                count++;
                break;
            }
            case '%': {
                putc('%');
                count++;
                break;
            }
            default:
                putc('%');
                putc(spec);
                count += 2;
                break;
        }
    }
    return count;
}

static int kvprintf_screen(int screen_id, const char* fmt, va_list ap) {
    int old_screen = default_print_screen;
    default_print_screen = screen_id;
    int result = kvprintf(kputchar, fmt, ap);
    default_print_screen = old_screen;
    return result;
}

int kprintf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int written = kvprintf(kputchar, fmt, ap);
    va_end(ap);
    return written;
}

int kprintf_screen(int screen_id, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int written = kvprintf_screen(screen_id, fmt, ap);
    va_end(ap);
    return written;
}