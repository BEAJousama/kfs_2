#include "../include/kernel.h"

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_pos = 0;

// Available commands
shell_command_t commands[] = {
    {"help", "Display this help message", cmd_help},
    {"stack", "Print kernel stack information", cmd_stack},
    {"gdt", "Display GDT information", cmd_gdt},
    {"clear", "Clear the screen", cmd_clear},
    {"echo", "Echo arguments", cmd_echo},
    {"time", "Show system uptime", cmd_time},
    {"reboot", "Reboot the system", cmd_reboot},
    {"halt", "Halt the system", cmd_halt},
    {NULL, NULL, NULL} // Terminator
};

// Command history for arrow key navigation
char command_history[10][MAX_COMMAND_LENGTH];
int history_count = 0;
int history_index = -1;

uint8_t cmos_read(uint8_t reg) {
    asm volatile("outb %1, %0" :: "dN"(0x70), "a"(reg));
    uint8_t val;
    asm volatile("inb %1, %0" : "=a"(val) : "dN"(0x71));
    return val;
}

// Convert BCD to binary if needed
uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val / 16) * 10);
}

// Get current time
void read_rtc_time(uint8_t* hour, uint8_t* minute, uint8_t* second) {
    *second = bcd_to_bin(cmos_read(0x00));
    *minute = bcd_to_bin(cmos_read(0x02));
    *hour   = bcd_to_bin(cmos_read(0x04));
}

// Command implementations
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
    // Keyboard controller reset
    outb(0x64, 0xFE);
    
    // If that doesn't work, triple fault
    __asm__ volatile ("cli");
    __asm__ volatile ("hlt");
}

void cmd_halt(int argc, char **argv) {
    (void)argc; (void)argv;
    
    kprintf("System halted. You can power off now.\n");
    __asm__ volatile ("cli");
    __asm__ volatile ("hlt");
    
    // Infinite loop just in case
    while (1) {
        __asm__ volatile ("hlt");
    }
}

// Simple argument parser
int parse_command(char *input, char **argv, int max_args) {
    int argc = 0;
    char *token = input;
    
    // Skip leading whitespace
    while (*token == ' ' || *token == '\t') token++;
    
    while (*token && argc < max_args - 1) {
        argv[argc++] = token;
        
        // Find end of current argument
        while (*token && *token != ' ' && *token != '\t') token++;
        
        // Null-terminate current argument
        if (*token) {
            *token = '\0';
            token++;
            
            // Skip whitespace
            while (*token == ' ' || *token == '\t') token++;
        }
    }
    
    argv[argc] = NULL;
    return argc;
}

// History management functions
void add_to_history(const char* command) {
    if (strlen(command) == 0) return;
    
    // Don't add duplicate of the last command
    if (history_count > 0 && strcmp(command_history[0], command) == 0) {
        return;
    }
    
    // Shift history up
    for (int i = 9; i > 0; i--) {
        strcpy(command_history[i], command_history[i-1]);
    }
    
    // Add new command
    strncpy(command_history[0], command, MAX_COMMAND_LENGTH - 1);
    command_history[0][MAX_COMMAND_LENGTH - 1] = '\0';
    
    if (history_count < 10) history_count++;
    history_index = -1;  // Reset history navigation
}

// Execute a command
void execute_command(char *input) {
    char *argv[MAX_ARGS];
    int argc = parse_command(input, argv, MAX_ARGS);
    
    if (argc == 0) return;
    
    // Add to history before executing
    add_to_history(input);
    
    // Find and execute command
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].function(argc, argv);
            return;
        }
    }
    
    kprintf("Command not found: %s\n", argv[0]);
    kprintf("Type 'help' for available commands.\n");
}

// Tab completion helper functions
int find_matching_commands(const char* partial, char matches[][32], int max_matches) {
    int count = 0;
    int partial_len = strlen(partial);
    
    if (partial_len == 0) {
        // If no partial input, show all commands
        for (int i = 0; commands[i].name != NULL && count < max_matches; i++) {
            strcpy(matches[count], commands[i].name);
            count++;
        }
        return count;
    }
    
    // Find commands that start with the partial string
    for (int i = 0; commands[i].name != NULL && count < max_matches; i++) {
        if (strncmp(commands[i].name, partial, partial_len) == 0) {
            strcpy(matches[count], commands[i].name);
            count++;
        }
    }
    
    return count;
}

// Helper function to safely clear the command line
void clear_command_line(void) {
    // Move cursor to beginning of line and clear it
    for (int i = 0; i < command_pos; i++) {
        terminal_putchar('\b');  // Move cursor back
    }
    for (int i = 0; i < command_pos; i++) {
        terminal_putchar(' ');   // Overwrite with spaces
    }
    for (int i = 0; i < command_pos; i++) {
        terminal_putchar('\b');  // Move cursor back again
    }
}

// Fixed tab completion function
void handle_tab_completion() {
    char matches[16][32]; // Support up to 16 matches of 32 chars each
    int match_count;
    
    // Ensure buffer is null-terminated
    if (command_pos >= MAX_COMMAND_LENGTH) command_pos = MAX_COMMAND_LENGTH - 1;
    command_buffer[command_pos] = '\0';
    
    // Find the start of the current word (simple: assume it's the whole buffer for now)
    char *current_word = command_buffer;
    
    // Skip leading whitespace
    while (*current_word == ' ' || *current_word == '\t') current_word++;
    
    // For simplicity, only complete if we're typing the first word (command name)
    bool completing_command = true;
    for (int i = 0; i < command_pos; i++) {
        if (command_buffer[i] == ' ' || command_buffer[i] == '\t') {
            completing_command = false;
            break;
        }
    }
    
    if (!completing_command) {
        // If we're not completing a command name, just show available commands
        kprintf("\nAvailable commands: ");
        for (int i = 0; commands[i].name != NULL; i++) {
            kprintf("%s ", commands[i].name);
        }
        kprintf("\n");
        shell_prompt();
        
        // Safely redisplay current command
        for (int i = 0; i < command_pos; i++) {
            terminal_putchar(command_buffer[i]);
        }
        return;
    }
    
    match_count = find_matching_commands(current_word, matches, 16);
    
    if (match_count == 0) {
        // No matches, just return silently
        return;
    } else if (match_count == 1) {
        // Exact match - complete the command
        int current_len = strlen(current_word);
        int match_len = strlen(matches[0]);
        
        // Calculate how many characters to add
        int chars_to_add = match_len - current_len;
        
        // Make sure we don't overflow the buffer
        if (command_pos + chars_to_add < MAX_COMMAND_LENGTH - 1) {
            // Add the remaining characters
            for (int i = 0; i < chars_to_add; i++) {
                command_buffer[command_pos] = matches[0][current_len + i];
                terminal_putchar(command_buffer[command_pos]);
                command_pos++;
            }
            
            // Add a space after completion if we have room
            if (command_pos < MAX_COMMAND_LENGTH - 2) {
                command_buffer[command_pos] = ' ';
                terminal_putchar(' ');
                command_pos++;
            }
        }
    } else {
        // Multiple matches - show them
        kprintf("\n");
        for (int i = 0; i < match_count; i++) {
            kprintf("%s ", matches[i]);
        }
        kprintf("\n");
        shell_prompt();
        
        // Safely redisplay current command
        for (int i = 0; i < command_pos; i++) {
            terminal_putchar(command_buffer[i]);
        }
        
        // Try to complete common prefix
        if (match_count > 1) {
            int common_len = strlen(matches[0]);
            
            // Find common prefix length
            for (int i = 1; i < match_count; i++) {
                int match_i_len = strlen(matches[i]);
                for (int j = 0; j < common_len && j < match_i_len; j++) {
                    if (matches[0][j] != matches[i][j]) {
                        common_len = j;
                        break;
                    }
                }
            }
            
            // Add common prefix if it's longer than what we have
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
    
    // Ensure buffer remains null-terminated
    if (command_pos < MAX_COMMAND_LENGTH) {
        command_buffer[command_pos] = '\0';
    }
}

void shell_handle_key(char c) {
    static int escape_state = 0;
    
    switch (escape_state) {
        case 0:  // Normal state
            if (c == 27) {  // ESC
                escape_state = 1;
                return;
            }
            
            switch (c) {
                case '\n':
                case '\r':  // Handle both line endings
                    // Execute command
                    kprintf("\n");
                    
                    // Ensure proper null termination
                    if (command_pos >= MAX_COMMAND_LENGTH) command_pos = MAX_COMMAND_LENGTH - 1;
                    command_buffer[command_pos] = '\0';
                    
                    if (command_pos > 0) {
                        // Execute
                        execute_command(command_buffer);
                    }
                    
                    // Reset for next command
                    command_pos = 0;
                    command_buffer[0] = '\0';  // Clear buffer
                    history_index = -1;  // Reset history navigation
                    shell_prompt();
                    break;
                    
                case '\b':
                case 127:  // DEL key (some keyboards send this for backspace)
                    // Backspace
                    if (command_pos > 0) {
                        command_pos--;
                        command_buffer[command_pos] = '\0';  // Null terminate
                        terminal_putchar('\b');
                    }
                    break;
                    
                case '\t':
                    // Tab completion
                    handle_tab_completion();
                    break;         
                default:
                    // Regular character
                    if (command_pos < MAX_COMMAND_LENGTH - 1 && c >= 32 && c <= 126) {
                        command_buffer[command_pos] = c;
                        command_pos++;
                        command_buffer[command_pos] = '\0';  // Keep null terminated
                        terminal_putchar(c);
                    }
                    // Silently ignore characters that don't fit or are out of range
                    break;
            }
            break;
            
        case 1:  // After ESC
            if (c == '[') {
                escape_state = 2;
                return;
            }
            escape_state = 0;
            // Process as normal character if not part of escape sequence
            shell_handle_key(c);
            break;         
    }
}

// Display shell prompt
void shell_prompt(void) {
    kprintf("Shell $> ");
}

// Initialize shell
void shell_init(void) {
    kprintf("\n=== KFS_2 Debug Shell ===\n");
    kprintf("Kernel From Scratch - Enhanced Shell\n");
    kprintf("Type 'help' for available commands\n");
    kprintf("Features: Tab completion, Command history (Up/Down arrows)\n");
    kprintf("========================\n\n");
    
    // Initialize command buffer and history
    command_pos = 0;
    command_buffer[0] = '\0';
    history_count = 0;
    history_index = -1;
    
    // Clear history
    for (int i = 0; i < 10; i++) {
        command_history[i][0] = '\0';
    }
    
    shell_prompt();
}

// Shell state management
bool shell_active = false;

void shell_toggle(void) {
    shell_active = !shell_active;
    if (shell_active) {
        kprintf("\n[Shell activated - ESC to exit]\n");
        shell_prompt();
    } else {
        kprintf("\n[Shell deactivated]\n");
        kprintf("$ ");
    }
}

bool is_shell_active(void) {
    return shell_active;
}