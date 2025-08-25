
#ifndef SHELL_H
#define SHELL_H

#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 16

typedef struct {
    char *name;
    char *description;
    void (*function)(int argc, char **argv);
} shell_command_t;

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} time_t;


void cmd_help(int argc, char **argv);
void cmd_stack(int argc, char **argv);
void cmd_gdt(int argc, char **argv);
void cmd_reboot(int argc, char **argv);
void cmd_halt(int argc, char **argv);
void cmd_clear(int argc, char **argv);
void cmd_echo(int argc, char **argv);
void cmd_time(int argc, char **argv);
void shell_prompt(void);
void shell_init(void);
void shell_handle_key(char c);
void cmd_shutdown(int argc, char **argv);
void shell_toggle(void);
bool is_shell_active(void);
void handle_tab_completion();
void clear_command_line(void);
int find_matching_commands(const char* partial, char matches[][32], int max_matches);
int parse_command(char *input, char **argv, int max_args);

#endif