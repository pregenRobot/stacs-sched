#ifndef common
#define common

#include <stdbool.h>
#include <sys/types.h>

typedef struct {
    pid_t process_id;
    bool current_status;
    char *executable_path;
    char *arguments;
    int priority;
} command;

int readconfig(char ***commands_ref);

#endif