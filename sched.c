#include <stdio.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#define LINE_MAX_LENGTH 1000

int main(int argc, char **argv)
{
    char **commands;
    int command_count = readconfig(&commands);

    if (command_count == -1)
    {
        return -1;
    }

    printf("Read %d command lines\n", command_count);
    for (int i = 0; i < command_count; i++)
    {
        printf("Command: %s", commands[i]);
    }

    process_info *valid_processes[command_count];
    int parse_result = parseconfig(&commands, &valid_processes, command_count);

    if(parse_result == -1){
        return -1;
    }

    printf("\nParsed %d valid commands\n", parse_result);
}

int parseconfig(char ***commands_ref, process_info ***parsed_processes, int command_count)
{
    int p_i = 0;
    int i;
    for (i = 0; i < command_count; ++i){
        char *empty_line = "\n";
        char* target_command = (*commands_ref)[i];
        if(strcmp(target_command, &empty_line) == 0){
            continue;
        }

        //parse components
        char* space = ' ';
        char** tokens = str_split(target_command, space);
        process_info *process = (process_info*) malloc(sizeof(process_info));

        process->process_id = (int*)malloc(sizeof(int));
        process->process_id = -1;
        process->current_status = (bool*)malloc(sizeof(bool));
        process->current_status = 0;
        if(!*(tokens)){
            printf("Invalid line at %d", i);
            continue;
        }
        process->priority = (int*)malloc(sizeof(int));
        int priority_parsed = (int)((uintmax_t)strtoumax(*(tokens), NULL, 10));
        process->priority = &priority_parsed;

        if(!*(tokens+1)){
            printf("Invalid line at %d", i);
            continue;
        }
        process->executable_path = (char*)malloc(LINE_MAX_LENGTH*sizeof(char));
        process->executable_path = *(tokens+1);
        if(!*(tokens+2)){
            printf("Invalid line at %d", i);
            continue;
        }
        process->arguments = (char*)malloc(LINE_MAX_LENGTH*sizeof(char));
        process->arguments = *(tokens+2);
        int ii;
        for(ii = 3; *(tokens+ii); ii++){
            char space_for_concat[] = " ";
            strcat(space_for_concat, *(tokens+ii));
            strcat(process->arguments,space_for_concat);
        }
        (*parsed_processes)[p_i] = process;
        p_i++;
    }
    
    if(p_i == 0){
        printf("No valid commands to execute were found in config file.");
        return -1;
    }else{
        return p_i;
    }
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