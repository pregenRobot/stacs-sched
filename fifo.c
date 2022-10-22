#include "common.h"
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <time.h>

static fifo_block* load(pcb** pcbs, int pcb_count){
    int i = 0;
    fifo_block* head = malloc(sizeof(fifo_block));
    head->info = pcbs[0];
    fifo_block* current = head;
    for(i = 1; i < pcb_count; i++){
        fifo_block* next = malloc(sizeof(fifo_block));
        current->next = next;
        current = current->next;
        current->info = pcbs[i];
    }
    current->next = NULL;
    return head;
}

static int startup(fifo_block* head, int executed){
    fifo_block* current = head;
    if(current != NULL){
        int pid = fork();
        if(pid == 0){
            current->info->status = 1;
            execv(current->info->executable_path, current->info->arguments);
        }else if(pid > 1){
            kill(pid, SIGSTOP);
            printf("Command: %s  - pid: %d\n", current->info->executable_path, pid);

            current->info->process_id = pid;
            current->info->status = 0;
            current->info->response_time = -1;
            current->info->begin = clock();

            return startup(current->next, executed) + 1;
        }else{
            printf("Fork failed.\n");
            return startup(current->next, executed);
        }
    }
    return 0;
}

static int execute(fifo_block* head, int executed){
    fifo_block* current = head;
    if(current != NULL){
        clock_t burst_start = clock();
        kill(current->info->process_id, SIGCONT);
        current->info->status = 1;
        int status;
        if(current->info->response_time == -1){
            current->info->response_time = clock() - current->info->begin;
        }
        int terminated = waitpid(current->info->process_id, &status, 0);
        if(WIFEXITED(status)){
            
            clock_t burst_end = clock();
            current->info->turnaround_time = burst_end - current->info->begin;
            current->info->burst_time = burst_end - burst_start;
            current->info->waiting_time = current->info->turnaround_time - current->info->burst_time;

            return execute(current->next, executed) + 1;
        }else{
            perror("Something went wrong");
            exit(1);
        }
    }
    return 0;
}

blocks* fifo_load(pcb** pcbs, int pcb_count, char** args){
    blocks* head_wrapper = malloc(sizeof(blocks));
    head_wrapper->fifo_head = load(pcbs, pcb_count);
    return head_wrapper;
}

int fifo_startup(blocks* b, int executed){
    return startup(b->fifo_head, executed);
}

int fifo_execute(blocks* b, int executed){
    return execute(b->fifo_head, executed);
}