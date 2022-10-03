#include <stdio.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LENGTH 1000

int main(int argc, char **argv){
    char **commands;
    int command_count = readconfig(&commands);

    if(command_count == -1){
        return -1;
    }
    printf("Read %d command lines\n", command_count);
    for(int i = 0; i < command_count; i++){
        printf("Command: %s", commands[i]);
    }
}

int parseconfig(char ***commands_ref){
}

int readconfig(char ***commands_ref){
    //Setup
    FILE *fp = fopen("exec.conf", "read");
    if(fp == NULL){
        printf("Error: Config file exec.conf not found in current directory");
        return -1;
    }

    // Count max rows and cols of the file to be read
    int n_rows = 0;
    int n_cols = 0;
    int length = 0;
    char line[LINE_MAX_LENGTH];
    while(fgets(line, LINE_MAX_LENGTH, fp) != NULL){
        n_rows++;
        length = (int)strlen(line);
        if(length > n_cols){
            n_cols = length;
        }
    }

    //Load file onto memory
    rewind(fp);
    *commands_ref = malloc(n_rows * sizeof(char*));
    int i = 0;
    while(fgets(line, LINE_MAX_LENGTH, fp) != NULL){
        (*commands_ref)[i] = malloc((n_cols + 1) * sizeof(char));
        strcpy((*commands_ref)[i], &line);
        i++;
    }

    // Cleanup
    fclose(fp);
    return n_rows;
}