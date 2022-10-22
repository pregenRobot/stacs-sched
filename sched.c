#include <stdio.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "common.h"
#include <inttypes.h>
#include <libgen.h>
#include <inttypes.h>
#include <sched.h>

#define LINE_MAX_LENGTH 1000
#define NONE ((void *) 0)

int main(int argc, char **argv){

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1) {
        printf("sched_setaffinity");
        exit(1);
    }

    scheduler *target_scheduler = malloc(sizeof(scheduler));

    if(handle_args(target_scheduler, argc, argv) == -1){
        printf("Error with configuration\n");
        exit(1);
    }
    
    char **commands;
    int command_count =readconfig(&commands,argv[1]);
    for (int i = 0; i < command_count; i++) printf("Command: %s", commands[i]);

    pcb **pcbs = malloc(command_count*sizeof(pcb*));
    int parse_count = parseconfig(commands, pcbs, command_count);
    printf("Read %d and parsed %d command lines. Schduler running on cpu %d\n", command_count, parse_count, sched_getcpu());
    if(parse_count == 0){
        printf("No runnable configurations found. Exiting...\n");
        exit(0);
    }

    blocks* head = (target_scheduler->loader)(pcbs, parse_count,&(argv[3]));
    int startup_result = (target_scheduler->starter)(head, parse_count);
    int execute_result = (target_scheduler->executor)(head, parse_count);
    log_stats(pcbs, parse_count);
    return 0;
}

int log_stats( pcb **pcbs, int pcb_count){
    int i;
    for(i = 0; i < pcb_count; i++){
        printf("\nStats for %s | response_time: %d burst_time: %d turnaround_time: %d waiting_time: %d\n", pcbs[i]->full_line, pcbs[i]->response_time, pcbs[i]->burst_time, pcbs[i]->turnaround_time, pcbs[i]->waiting_time);
    }
}


int handle_args(
    scheduler* target_scheduler,
    int argc,
    char** argv
){
    if(argc < 3){
        printf("Arguments not supplied ./sched {exec.conf path} {scheduler:fifo,mlfq,rr} {scheduler args}\n");
        return -1;
    }

    if(argc == 3 && (int)strcmp(argv[2], "rr") == 0){
        printf("RR scheduler -- Round-robing scheduling requires quantum as second argument\n");
        return -1;
    }

    if(argc == 3 && (int)strcmp(argv[2],"fifo") == 0){
        target_scheduler->loader = fifo_load;
        target_scheduler->executor = fifo_execute;
        target_scheduler->starter = fifo_startup;
        return 0;
    } else if(argc == 3 && (int)strcmp(argv[2], "mlfq") == 0){
        target_scheduler->loader = mlfq_load;
        target_scheduler->executor = mlfq_execute;
        target_scheduler->starter = mlfq_startup;
        return 0;
    }else if (argc == 4 && (int)strcmp(argv[2], "rr") == 0){
        target_scheduler->loader = rr_load;
        target_scheduler->executor = rr_execute;
        target_scheduler->starter = rr_startup;
        return 0;
    }else{
        printf("Invalid scheduler");
        return -1;
    }
}


int readconfig(char ***commands_ref, char* path)
{
    // Setup
    FILE *fp = fopen(path, "read");
    if (fp == NULL)
    {
        printf("Error: Config file not found\n");
        exit(1);
    }

    // Count max rows and cols of the file to be read
    int n_rows = 0;
    int n_cols = 0;
    int length = 0;
    char* line = malloc(LINE_MAX_LENGTH * sizeof(char));
    while (fgets(line, LINE_MAX_LENGTH, fp) != NULL)
    {
        n_rows++;
        length = (int)strlen(line);
        if (length > n_cols)
        {
            n_cols = length;
        }
    }

    // Load file onto memory
    rewind(fp);
    *commands_ref = malloc(n_rows * sizeof(char *));
    int i = 0;
    while (fgets(line, LINE_MAX_LENGTH, fp) != NULL)
    {
        (*commands_ref)[i] = malloc((n_cols + 1) * sizeof(char));
        strcpy((*commands_ref)[i], line);
        i++;
    }

    // Cleanup
    fclose(fp);
    return n_rows;
}

int parseconfig(char **commands_ref, pcb **pcbs, int command_count){
    int p_i = 0;

    int i;
    for(i = 0; i < command_count; i++){
        char *empty_line = "\n";
        const char* slash = (char*)'/';
        char *target_command = commands_ref[i];

        if(strcmp(target_command, empty_line) == 0){
            continue;
        }
        char ** tokens = str_split(target_command, ' ');
        if(!*(tokens) || !*(tokens+1) || !isNumeric(*tokens)){
            printf("\nConfig file has invalid line at %d\n", i+1);
            continue;
        }

        pcb *process = (pcb*)malloc(sizeof(pcb));

        process->process_id = -1;
        process->status = 0;
        process->priority = (int)((uintmax_t) strtoumax(*tokens, NULL, 10));

        process->executable_path = (char*)malloc(sizeof(char)*LINE_MAX_LENGTH);
        process->executable_path = strdup(*(tokens+1));

        process->arguments = malloc(sizeof(char*) * LINE_MAX_LENGTH);
        if(*(tokens + 2)){
            char **arguments = (tokens+2);
            process->arguments[0] = basename(process->executable_path);
            int argi = 1;
            while(*(arguments + argi - 1) != NONE){
                process->arguments[argi] = strdup(*(arguments + (argi - 1)));
                argi+=1;
            }
            process->arguments[argi] = NULL;
        }
        process->full_line = commands_ref[i];
        pcbs[p_i] = process;
        p_i++;
    }
    return p_i;
}

// https://stackoverflow.com/questions/16644906/how-to-check-if-a-string-is-a-number
bool isNumeric(const char* s){
    while(*s){
        if(*s < '0' || *s > '9')
            return false;
        ++s;
    }
    return true;
}

char* join_strings(char **strings, char *separator){
    char *str = (char*) malloc(sizeof(str) * LINE_MAX_LENGTH);
    str[0] = '\0';

    int i = 0;
    for(i = 0; *(strings+i); i++){
        strcat(str, strdup(*(strings+i)));
        strcat(str, strdup(separator));
    }
    return str;
}


// https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
char** str_split(char* a_str, char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    a_str[strlen(a_str) - 1] = '\0';
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
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

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }
    return result;
}