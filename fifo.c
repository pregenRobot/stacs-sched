#include "common.h"
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include "utility.h"
#include <sched.h>

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
            
            execvp(current->info->executable_path, current->info->arguments);
        }else if(pid > 1){
            kill(pid, SIGSTOP);
            printf("Command: %s  - pid: %d - cpu: %d\n", current->info->executable_path, pid, sched_getcpu());

            current->info->process_id = pid;
            current->info->status = 0;
            log_startup(current->info);

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

        log_execute_start(current->info);
        kill(current->info->process_id, SIGCONT);
        current->info->status = 1;
        int status;
        int terminated = waitpid(current->info->process_id, &status, 0);

        int64_t burst_end = micros();

        if(WIFEXITED(status)){
            log_execute_finish(current->info);
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