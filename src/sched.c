#include "common.h"
#include <assert.h>
#include <inttypes.h>
#include <libgen.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LENGTH 1000
#define NONE ((void *)0)

int main(int argc, char **argv) {

    scheduler *target_scheduler = malloc(sizeof(scheduler)); // free OK

    // Dynamically determine the scheduling algorithm and allocate relevant
    // methods
    if (handle_args(target_scheduler, argc, argv) == -1) {
        printf("Error with configuration\n");
        exit(1);
    }

    // Read configuration file
    char **commands;
    int command_count = readconfig(&commands, argv[1]);
    for (int i = 0; i < command_count; i++)
        printf("Command: %s", commands[i]);

    // Parse configuration file
    pcb **pcbs = malloc(command_count * sizeof(pcb *)); // free OK
    int parse_count = parseconfig(commands, pcbs, command_count);
    printf("Read %d and parsed %d command lines. Schduler running on cpu %d\n",
           command_count, parse_count, sched_getcpu());
    if (parse_count == 0) {
        printf("No runnable configurations found. Exiting...\n");
        exit(0);
    }

    // Create the relevant datastructure for the scheduler
    blocks *head = (target_scheduler->loader)(pcbs, parse_count, &(argv[3]));
    // Startup the relevant processes
    int startup_result = (target_scheduler->starter)(head, parse_count);
    // Execute the processes
    int execute_result = (target_scheduler->executor)(head, parse_count);
    log_stats(pcbs, parse_count);

    // Free the memory used for the program
    free_all(target_scheduler, pcbs, parse_count, head, commands,
             command_count);
    return 0;
}

void free_all(scheduler *target_scheduler, pcb **pcbs, int pcb_count,
              blocks *head, char **commands, int command_count) {
    int i;
    for (i = 0; i < pcb_count; i++) {
        free(pcbs[i]->executable_path);
        free(pcbs[i]->arguments);
        free(pcbs[i]->full_line);
        free(pcbs[i]);
    }
    free(pcbs);
    free(commands);
    target_scheduler->free_blocks(head);
    free(target_scheduler);
    free(head);
}

int log_stats(pcb **pcbs, int pcb_count) {
    int i;
    printf("\n=Benchmark=\n");
    for (i = 0; i < pcb_count; i++) {
        printf("Command: %sresponse_time: %d, burst_time: %d, turnaround_time: "
               "%d, waiting_time: %d\n",
               pcbs[i]->full_line, pcbs[i]->response_time, pcbs[i]->burst_time,
               pcbs[i]->turnaround_time, pcbs[i]->waiting_time);
    }
}

int handle_args(scheduler *target_scheduler, int argc, char **argv) {
    if (argc < 3) {
        printf("Arguments not supplied ./sched {exec.conf path} "
               "{scheduler:fcfs,p_rr,rr} {scheduler args}\n");
        return -1;
    }

    if (argc == 3 && (int)strcmp(argv[2], "rr") == 0) {
        printf("RR scheduler -- Round-robing scheduling requires quantum as "
               "second "
               "argument\n");
        return -1;
    }

    // Determine the methods needed to run the scheduler
    if (argc == 3 && (int)strcmp(argv[2], "fcfs") == 0) {
        target_scheduler->loader = fcfs_load;
        target_scheduler->executor = fcfs_execute;
        target_scheduler->starter = fcfs_startup;
        target_scheduler->free_blocks = fcfs_free_blocks;
        return 0;
    } else if (argc == 4 && (int)strcmp(argv[2], "p_rr") == 0) {
        target_scheduler->loader = p_rr_load;
        target_scheduler->executor = rr_execute;
        target_scheduler->starter = rr_startup;
        target_scheduler->free_blocks = rr_free_blocks;
        return 0;
    } else if (argc == 4 && (int)strcmp(argv[2], "rr") == 0) {
        target_scheduler->loader = rr_load;
        target_scheduler->executor = rr_execute;
        target_scheduler->starter = rr_startup;
        target_scheduler->free_blocks = rr_free_blocks;
        return 0;
    } else {
        printf("Invalid scheduler");
        return -1;
    }
}

int readconfig(char ***commands_ref, char *path) {
    // Setup
    FILE *fp = fopen(path, "read");
    if (fp == NULL) {
        printf("Error: Config file not found\n");
        exit(1);
    }

    // Count max rows and cols of the file to be read
    int n_rows = 0;
    int n_cols = 0;
    int length = 0;
    char *line = malloc(LINE_MAX_LENGTH * sizeof(char)); // free OK
    while (fgets(line, LINE_MAX_LENGTH, fp) != NULL) {
        n_rows++;
        length = (int)strlen(line);
        if (length > n_cols) {
            n_cols = length;
        }
    }

    // Load file onto memory
    rewind(fp);
    *commands_ref = malloc(n_rows * sizeof(char *)); // free OK
    int i = 0;
    while (fgets(line, LINE_MAX_LENGTH, fp) != NULL) {
        (*commands_ref)[i] = malloc((n_cols + 1) * sizeof(char));
        strcpy((*commands_ref)[i], line); // free OK
        i++;
    }

    // Cleanup
    free(line);
    fclose(fp);
    return n_rows;
}

int parseconfig(char **commands_ref, pcb **pcbs, int command_count) {
    int p_i = 0;

    int i;
    for (i = 0; i < command_count; i++) {
        char *empty_line = "\n";
        const char *slash = (char *)'/';
        char *target_command = commands_ref[i];
        char *full_line = strdup(commands_ref[i]);

        // Ignore empty lines
        if (strcmp(target_command, empty_line) == 0) {
            continue;
        }

        // Split each lines into tokens
        char **tokens = str_split(target_command, ' ');
        if (!*(tokens) || !*(tokens + 1) || !isNumeric(*tokens)) {
            printf("\nConfig file has invalid line at %d\n", i + 1);
            continue;
        }

        // Parse line
        pcb *process = (pcb *)malloc(sizeof(pcb)); // free OK

        process->process_id = -1;
        process->status = 0;
        process->priority = (int)((uintmax_t)strtoumax(*tokens, NULL, 10));

        process->executable_path =
            (char *)malloc(sizeof(char) * LINE_MAX_LENGTH); // free OK
        process->executable_path = strdup(*(tokens + 1));

        // Interpret the number of arguments necessary for the command
        process->arguments =
            malloc(sizeof(char *) * LINE_MAX_LENGTH); // free OK
        if (*(tokens + 2)) {
            char **arguments = (tokens + 2);
            process->arguments[0] = basename(process->executable_path);
            int argi = 1;
            while (*(arguments + argi - 1) != NONE) {
                process->arguments[argi] = strdup(*(arguments + (argi - 1)));
                argi += 1;
            }
            process->arg_count = argi;
            process->arguments[argi] = NULL;
        }
        process->full_line = full_line;
        pcbs[p_i] = process;
        p_i++;
        free(tokens);
    }
    return p_i;
}

// https://stackoverflow.com/questions/16644906/how-to-check-if-a-string-is-a-number
bool isNumeric(const char *s) {
    while (*s) {
        if (*s < '0' || *s > '9')
            return false;
        ++s;
    }
    return true;
}

// https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
char **str_split(char *a_str, char a_delim) {
    char **result = 0;
    size_t count = 0;
    a_str[strlen(a_str) - 1] = '\0';
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count); // free all

    if (result) {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }
    return result;
}