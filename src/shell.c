#include "kernel.h"
#include "gdt.h"

#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 16

// Command structure
typedef struct {
    char *name;
    char *description;
    void (*function)(int argc, char **argv);
} shell_command_t;

// Command buffer
static char command_buffer[MAX_COMMAND_LENGTH];
static int command_pos = 0;

// Command history (simple)
static char last_command[MAX_COMMAND_LENGTH];

// Shell command functions
void cmd_help(int argc, char **argv);
void cmd_stack(int argc, char **argv);
void cmd_gdt(int argc, char **argv);
void cmd_reboot(int argc, char **argv);
void cmd_halt(int argc, char **argv);
void cmd_clear(int argc, char **argv);
void cmd_echo(int argc, char **argv);
void cmd_uptime(int argc, char **argv);

// Available commands
shell_command_t commands[] = {
    {"help", "Display this help message", cmd_help},
    {"stack", "Print kernel stack information", cmd_stack},
    {"gdt", "Display GDT information", cmd_gdt},
    {"clear", "Clear the screen", cmd_clear},
    {"echo", "Echo arguments", cmd_echo},
    {"uptime", "Show system uptime", cmd_uptime},
    {"reboot", "Reboot the system", cmd_reboot},
    {"halt", "Halt the system", cmd_halt},
    {NULL, NULL, NULL} // Terminator
};

// Simple uptime counter (incremented by timer if available)
static uint32_t uptime_seconds = 0;

void increment_uptime(void) {
    uptime_seconds++;
}

// Command implementations
void cmd_help(int argc, char **argv) {
    (void)argc; (void)argv;
    
    kprintf("Available commands:\n");
    kprintf("==================\n");
    
    for (int i = 0; commands[i].name != NULL; i++) {
        kprintf("%-10s - %s\n", commands[i].name, commands[i].description);
    }
    kprintf("\nPress Tab for command completion, Up arrow for last command\n");
}

void cmd_stack(int argc, char **argv) {
    (void)argc; (void)argv;
    
    print_stack_info();
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

void cmd_uptime(int argc, char **argv) {
    (void)argc; (void)argv;
    
    uint32_t hours = uptime_seconds / 3600;
    uint32_t minutes = (uptime_seconds % 3600) / 60;
    uint32_t seconds = uptime_seconds % 60;
    
    kprintf("System uptime: %d:%02d:%02d\n", hours, minutes, seconds);
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

// Execute a command
void execute_command(char *input) {
    char *argv[MAX_ARGS];
    int argc = parse_command(input, argv, MAX_ARGS);
    
    if (argc == 0) return;
    
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

// Handle special keys in shell
void shell_handle_key(char c) {
    switch (c) {
        case '\n':
            // Execute command
            kprintf("\n");
            command_buffer[command_pos] = '\0';
            
            if (command_pos > 0) {
                // Save to history
                strcpy(last_command, command_buffer);
                
                // Execute
                execute_command(command_buffer);
            }
            
            // Reset for next command
            command_pos = 0;
            shell_prompt();
            break;
            
        case '\b':
            // Backspace
            if (command_pos > 0) {
                command_pos--;
                terminal_putchar('\b');
            }
            break;
            
        case '\t':
            // Tab completion (simple implementation)
            kprintf("\nAvailable commands: ");
            for (int i = 0; commands[i].name != NULL; i++) {
                kprintf("%s ", commands[i].name);
            }
            kprintf("\n");
            shell_prompt();
            
            // Redisplay current command
            for (int i = 0; i < command_pos; i++) {
                terminal_putchar(command_buffer[i]);
            }
            break;
            
        default:
            // Regular character
            if (command_pos < MAX_COMMAND_LENGTH - 1 && c >= 32 && c <= 126) {
                command_buffer[command_pos++] = c;
                terminal_putchar(c);
            }
            break;
    }
}

// Display shell prompt
void shell_prompt(void) {
    kprintf("kfs> ");
}

// Initialize shell
void shell_init(void) {
    kprintf("\n=== KFS_2 Debug Shell ===\n");
    kprintf("Kernel From Scratch - Simple Shell\n");
    kprintf("Type 'help' for available commands\n");
    kprintf("========================\n\n");
    
    command_pos = 0;
    shell_prompt();
}

// Check if shell should handle this character
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