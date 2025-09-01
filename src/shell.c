#include "../include/kernel.h"

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_pos = 0;

shell_command_t commands[] = {
    {"help", "Display this help message", cmd_help},
    {"stack", "Print kernel stack information", cmd_stack},
    {"gdt", "Display GDT information", cmd_gdt},
    {"clear", "Clear the screen", cmd_clear},
    {"echo", "Echo arguments", cmd_echo},
    {"time", "Show system uptime", cmd_time},
    {"reboot", "Reboot the system", cmd_reboot},
    {"halt", "Halt the system", cmd_halt},
    {NULL, NULL, NULL}
};

uint8_t cmos_read(uint8_t reg) {
    asm volatile("outb %1, %0" :: "dN"(0x70), "a"(reg));
    uint8_t val;
    asm volatile("inb %1, %0" : "=a"(val) : "dN"(0x71));
    return val;
}

uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val / 16) * 10);
}

void read_rtc_time(uint8_t* hour, uint8_t* minute, uint8_t* second) {
    *second = bcd_to_bin(cmos_read(0x00));
    *minute = bcd_to_bin(cmos_read(0x02));
    *hour   = bcd_to_bin(cmos_read(0x04));
}

void cmd_help(int argc, char **argv) {
    (void)argc; (void)argv;
    
    kprintf("Available commands:\n");
    kprintf("==================\n");
    
    for (int i = 0; commands[i].name != NULL; i++) {
        kprintf("%12s - %s\n", commands[i].name, commands[i].description);
    }
    kprintf("\nShell features:\n");
    kprintf("==============\n");
    kprintf("      - Tab for command completion\n");
    kprintf("      - Backspace for editing\n");
}

void cmd_stack(int argc, char **argv) {
    (void)argc; (void)argv;
    print_kernel_stack();
}

void cmd_gdt(int argc, char **argv) {
    (void)argc; (void)argv;
    print_gdt_info();
}

void cmd_clear(int argc, char **argv) {
    (void)argc; (void)argv;
    terminal_clear();
}

void cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        kprintf("%s", argv[i]);
        if (i < argc - 1) kprintf(" ");
    }
    kprintf("\n");
}

void cmd_time(int argc, char **argv) {
    (void)argc; (void)argv;
    
    time_t current_time;
    read_rtc_time(&current_time.hour, &current_time.minute, &current_time.second);

    kprintf("Current Time: %02u:%02u:%02u\n", current_time.hour, current_time.minute, current_time.second);
}

void cmd_reboot(int argc, char **argv) {
    (void)argc; (void)argv;
    
    kprintf("Rebooting system...\n");
    outb(0x64, 0xFE);
    __asm__ volatile ("cli");
    __asm__ volatile ("hlt");
}

void cmd_halt(int argc, char **argv) {
    (void)argc; (void)argv;
    
    kprintf("System halted. You can power off now.\n");
    __asm__ volatile ("cli");
    __asm__ volatile ("hlt");
    
    while (1) {
        __asm__ volatile ("hlt");
    }
}

int parse_command(char *input, char **argv, int max_args) {
    int argc = 0;
    char *token = input;
    
    while (*token == ' ' || *token == '\t') token++;
    
    while (*token && argc < max_args - 1) {
        argv[argc++] = token;
        
        while (*token && *token != ' ' && *token != '\t') token++;
        
        if (*token) {
            *token = '\0';
            token++; 
            while (*token == ' ' || *token == '\t') token++;
        }
    }
    
    argv[argc] = NULL;
    return argc;
}

void execute_command(char *input) {
    char *argv[MAX_ARGS];
    int argc = parse_command(input, argv, MAX_ARGS);
    
    if (argc == 0) return;
    
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].function(argc, argv);
            return;
        }
    }
    
    kprintf("Command not found: %s\n", argv[0]);
    kprintf("Type 'help' for available commands.\n");
}

int find_matching_commands(const char* partial, char matches[][32], int max_matches) {
    int count = 0;
    int partial_len = strlen(partial);
    
    if (partial_len == 0) {
        for (int i = 0; commands[i].name != NULL && count < max_matches; i++) {
            strcpy(matches[count], commands[i].name);
            count++;
        }
        return count;
    }
    
    for (int i = 0; commands[i].name != NULL && count < max_matches; i++) {
        if (strncmp(commands[i].name, partial, partial_len) == 0) {
            strcpy(matches[count], commands[i].name);
            count++;
        }
    }
    
    return count;
}

void handle_tab_completion() {
    char matches[16][32];
    int match_count;
    
    if (command_pos >= MAX_COMMAND_LENGTH) command_pos = MAX_COMMAND_LENGTH - 1;
    command_buffer[command_pos] = '\0';

    char *current_word = command_buffer;
    
    while (*current_word == ' ' || *current_word == '\t') current_word++;
    
    bool completing_command = true;
    for (int i = 0; i < command_pos; i++) {
        if (command_buffer[i] == ' ' || command_buffer[i] == '\t') {
            completing_command = false;
            break;
        }
    }
    
    if (!completing_command) {
        kprintf("\nAvailable commands: ");
        for (int i = 0; commands[i].name != NULL; i++) {
            kprintf("%s ", commands[i].name);
        }
        kprintf("\n");
        shell_prompt();
        
        for (int i = 0; i < command_pos; i++) {
            terminal_putchar(command_buffer[i]);
        }
        return;
    }
    
    match_count = find_matching_commands(current_word, matches, 16);
    
    if (match_count == 0) {
        return;
    } else if (match_count == 1) {
        int current_len = strlen(current_word);
        int match_len = strlen(matches[0]);
        int chars_to_add = match_len - current_len;
        
        if (command_pos + chars_to_add < MAX_COMMAND_LENGTH - 1) {
            for (int i = 0; i < chars_to_add; i++) {
                command_buffer[command_pos] = matches[0][current_len + i];
                terminal_putchar(command_buffer[command_pos]);
                command_pos++;
            }
            
            if (command_pos < MAX_COMMAND_LENGTH - 2) {
                command_buffer[command_pos] = ' ';
                terminal_putchar(' ');
                command_pos++;
            }
        }
    } else {
        kprintf("\n");
        for (int i = 0; i < match_count; i++) {
            kprintf("%s ", matches[i]);
        }
        kprintf("\n");
        shell_prompt();
        for (int i = 0; i < command_pos; i++) {
            terminal_putchar(command_buffer[i]);
        }
        
        if (match_count > 1) {
            int common_len = strlen(matches[0]);
            
            for (int i = 1; i < match_count; i++) {
                int match_i_len = strlen(matches[i]);
                for (int j = 0; j < common_len && j < match_i_len; j++) {
                    if (matches[0][j] != matches[i][j]) {
                        common_len = j;
                        break;
                    }
                }
            }
            
            int current_len = strlen(current_word);
            if (common_len > current_len) {
                int chars_to_add = common_len - current_len;
                if (command_pos + chars_to_add < MAX_COMMAND_LENGTH - 1) {
                    for (int i = 0; i < chars_to_add; i++) {
                        command_buffer[command_pos] = matches[0][current_len + i];
                        terminal_putchar(command_buffer[command_pos]);
                        command_pos++;
                    }
                }
            }
        }
    }
    
    if (command_pos < MAX_COMMAND_LENGTH) {
        command_buffer[command_pos] = '\0';
    }
}

void shell_handle_key(char c) {
    static int escape_state = 0;

    if (c == 27) {
        escape_state = 1;
        return;
    }
    
    switch (c) {
        case '\n':
        case '\r':
            kprintf("\n");
            
            if (command_pos >= MAX_COMMAND_LENGTH) command_pos = MAX_COMMAND_LENGTH - 1;
            command_buffer[command_pos] = '\0';
            
            if (command_pos > 0) {
                execute_command(command_buffer);
            }
            
            command_pos = 0;
            command_buffer[0] = '\0';
            shell_prompt();
            break;
            
        case '\b':
        case 127:
            if (command_pos > 0) {
                command_pos--;
                command_buffer[command_pos] = '\0';
                terminal_putchar('\b');
            }
            break;
            
        case '\t':
            handle_tab_completion();
            break;         
        default:
            if (command_pos < MAX_COMMAND_LENGTH - 1 && c >= 32 && c <= 126) {
                command_buffer[command_pos] = c;
                command_pos++;
                command_buffer[command_pos] = '\0';
                terminal_putchar(c);
            }
            break;
    }

}

void shell_prompt(void) {
    kprintf("Shell $> ");
}

void shell_init(void) {
    kprintf("\n=== KFS_2 Debug Shell ===\n");
    kprintf("Kernel From Scratch - Enhanced Shell\n");
    kprintf("Type 'help' for available commands\n");
    kprintf("Features: Tab completion \n");
    kprintf("========================\n\n");
    command_pos = 0;
    command_buffer[0] = '\0';
}

bool shell_active = false;

void shell_toggle(void) {
    shell_active = !shell_active;
    
    if (shell_active) {
        kprintf("\n[Shell activated - ESC to exit]\n");
        shell_init();
        shell_prompt();
    } else {
        kprintf("\n[Shell deactivated]\n");
    }
}

bool is_shell_active(void) {
    return shell_active;
}