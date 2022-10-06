#include <stdio.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include "fifo.h"

#define LINE_MAX_LENGTH 1000

int main(int argc, char **argv)
{
    char **commands;
    int command_count = readconfig(&commands);

    if (command_count == -1)
    {
        
    }

    printf("Read %d command lines\n", command_count);
    for (int i = 0; i < command_count; i++)
    {
        printf("Command: %s", commands[i]);
    }

    process_info **valid_processes = (process_info**)malloc(command_count*sizeof(process_info*));

    int parse_result = parseconfig(commands, valid_processes, command_count);

    if(parse_result == -1){
        return -1;
    }

    printf("\nParsed %d valid commands\n", parse_result);

    // Executing as Fifo
    fifo_block* processes_head = load(valid_processes, parse_result);
    int initial_execution_result = initial_execution(processes_head, 0);
    printf("\nExecuted %d commands\n", initial_execution_result);

    int execute_result = execute(processes_head, 0);
    
}

int parseconfig(char **commands_ref, process_info **parsed_processes, int command_count){
    int p_i = 0;
    int i;
    for(i = 0; i < command_count; i++){
        char *empty_line = '\n';
        char* space = ' ';
        char *target_command = commands_ref[i];
        char *space_for_concat = " ";

        if(strcmp(target_command, &empty_line) == 0){
            continue;
        }
        char ** tokens = str_split(target_command, space);
        if(!*(tokens) || !*(tokens+1) || !*(tokens + 2)){
            printf("\nInvalid line at %d\n", i);
            continue;
        }

        process_info *process = (process_info*)malloc(sizeof(process_info));

        process->process_id = -1;
        process->current_status = 0;
        process->priority = (int)((uintmax_t) strtoumax(*tokens, NULL, 10));

        process->executable_path = (char*)malloc(sizeof(char)*LINE_MAX_LENGTH);
        process->executable_path = strdup(*(tokens+1));

        process->arguments = (char*)malloc(sizeof(char) * LINE_MAX_LENGTH);
        char **arguments = (tokens+2);
        process->arguments = join_strings(arguments, space_for_concat);
        parsed_processes[p_i] = process;
        p_i++;
    }
    return p_i;
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
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
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

int readconfig(char ***commands_ref)
{
    // Setup
    FILE *fp = fopen("exec.conf", "read");
    if (fp == NULL)
    {
        printf("Error: Config file exec.conf not found in current directory");
        return -1;
    }

    // Count max rows and cols of the file to be read
    int n_rows = 0;
    int n_cols = 0;
    int length = 0;
    char line[LINE_MAX_LENGTH];
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
        strcpy((*commands_ref)[i], &line);
        i++;
    }

    // Cleanup
    fclose(fp);
    return n_rows;
}