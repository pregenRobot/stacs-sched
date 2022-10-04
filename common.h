#ifndef common
#define common

#include <stdbool.h>
#include <sys/types.h>

typedef struct {
    pid_t *process_id;
    bool *current_status;
    char *executable_path;
    char *arguments;
    int *priority;
} process_info;

int readconfig(char ***commands_ref);
int parseconfig(char ***commands_ref, process_info*** parsed_processes, int command_count);
char** str_split(char* a_str, const char a_delim);

#endif