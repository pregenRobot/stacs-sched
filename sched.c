#include <stdio.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "common.h"

#define LINE_MAX_LENGTH 1000

int main(int argc, char **argv){
    return 0;
    blocks* (*loader)(pcb**, int);
    int* (*starter)(blocks*, int);
    int* (*executor)(blocks**, int);
    handle_args(loader, starter, executor, argc, argv);
    int x = 0;
}

int handle_args(load_func loader, startup_func starter, execute_func executor, int argc, char** argv){
    if(argc < 2){
        perror("Arguments not supplied");
        exit(1);
    }

    if(argc == 2 && argv[2] == "rr"){
        perror("RR scheduler -- Round-robing scheduling requires quantum as second argument");
        exit(1);
    }

    if(argc == 2 && argv[2] == "fifo"){
        loader = &fifo_load;
        executor = &fifo_execute;
        starter = &fifo_startup;
        return 0;
    }

    if(argc == 2 && argv[2] == "pq"){
        loader = &pq_load;
        executor = &pq_execute;
        starter = &pq_startup;
        return 0;
    }
}