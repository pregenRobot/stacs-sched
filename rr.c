
#include "common.h"
#include <stdlib.h>
#include <inttypes.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include "timer.h"
#include <sched.h>


static rr_block* load(pcb** pcbs, int pcb_count, int quantum){
    int i = 0;
    rr_block* head = malloc(sizeof(rr_block));
    head->info = pcbs[0];
    head->quantum = quantum;
    printf("RR Config - Using quantum %d\n",quantum);
    rr_block* current = head;
    for(i = 1; i < pcb_count; i++){
        rr_block* next = malloc(sizeof(fifo_block));
        current->next = next;
        current = current->next;
        current->info = pcbs[i];
        current->quantum = quantum;
    }
    current->next = NULL;
    return head;
}

static int startup(rr_block* head, int executed){
    fifo_block* current = head;


    if(current != NULL){
        int pid = fork();
        if(pid == 0){
            current->info->status = 1;
            execv(current->info->executable_path, current->info->arguments);
        }else{
            kill(pid, SIGSTOP);
            printf("Command: %s  - pid: %d - cpu: %d\n", current->info->executable_path, pid, sched_getcpu());
            current->info->process_id = pid;
            current->info->status = 0;
            log_startup(current->info);
            return startup(current->next, executed) + 1;
        }
    }
    return 0;
}

blocks* rr_load(pcb** pcbs, int pcb_count, char** args){
    blocks* head_wrapper = malloc(sizeof(blocks));
    int quantum = (int)((uintmax_t) strtoumax(args[0], NULL, 10));
    if(quantum <= 0){
        printf("Invalid quantum for RR scheduler\n");
        exit(1);
    }
    head_wrapper->rr_head = load(pcbs, pcb_count, quantum);
    return head_wrapper;
}

int rr_startup(blocks* b, int commands){
    return startup(b->rr_head, 0);
}

static int execute(rr_block* head, int executed){
    rr_block* current = head;
    if(current != NULL){// While we are not at the end of the list of processes
        if(current->info->status == 0){
            //If process has not finished executing (verify from status)

            log_execute_start(current->info);

            kill(current->info->process_id, SIGCONT);
            current->info->status = 1;
            int status;
            int waiting_result = waitpid(current->info->process_id, &status, WNOHANG);
            usleep((__useconds_t)current->quantum);
            kill(current->info->process_id, SIGSTOP);

            if(waiting_result == 0){  // child is still running
                log_execute_pause(current->info);
                current->info->status = 0;
                return execute(current->next, executed);
            }else if (waiting_result == current->info->process_id){ // child has finished executing
                current->info->status = -2;
                log_execute_finish(current->info);
                return execute(current->next,executed) + 1;
            }
        }else{
            return execute(current->next,executed) + 1;
        }
    }
    return 0;
}

int rr_execute(blocks* b, int commands){
    int finished_execution = 0;
    while(finished_execution < commands){
        finished_execution = execute(b->rr_head, 0);
    }
}