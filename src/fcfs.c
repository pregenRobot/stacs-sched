#include "common.h"
#include "utility.h"
#include <inttypes.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static fcfs_block *load(pcb **pcbs, int pcb_count) {
    int i = 0;
    fcfs_block *head = malloc(sizeof(fcfs_block)); // free OK
    head->info = pcbs[0];
    fcfs_block *current = head;
    for (i = 1; i < pcb_count; i++) {
        fcfs_block *next = malloc(sizeof(fcfs_block)); // free OK
        current->next = next;
        current = current->next;
        current->info = pcbs[i];
    }
    current->next = NULL;
    return head;
}

static void free_blocks(fcfs_block* head){
    if(head != NULL){
        free_blocks(head->next);
        free(head);
    }
}

static int startup(fcfs_block *head, int executed) {
    fcfs_block *current = head;
    if (current != NULL) {
        if (!command_is_executable(current->info->executable_path)) {
            printf("Cannot find executable %s - file does not exist and it is "
                   "not in "
                   "PATH. Skipping...",
                   current->info->executable_path);
            current->info->status = -2;
            return startup(current->next, executed) + 1;
        }
        

        int pid = fork();
        if (pid == 0) {
            current->info->status = 1;

            execvp(current->info->executable_path, current->info->arguments);
        } else if (pid > 1) {
            kill(pid, SIGSTOP);
            printf("Priority: %d Command: %s  - pid: %d - cpu: %d\n", current->info->priority,
                   current->info->executable_path, pid, sched_getcpu());
            current->info->process_id = pid;
            current->info->status = 0;
            log_startup(current->info);
            return startup(current->next, executed) + 1;
        } else {
            printf("Fork failed.\n");
            return startup(current->next, executed);
        }
    }
    return 0;
}

static int execute(fcfs_block *head, int executed) {
    fcfs_block *current = head;
    if (current != NULL) {

        log_execute_start(current->info);
        kill(current->info->process_id, SIGCONT);
        current->info->status = 1;
        int status;
        int terminated = waitpid(current->info->process_id, &status, 0);

        if (WIFEXITED(status)) {
            log_execute_finish(current->info);
            return execute(current->next, executed) + 1;
        } else {
            perror("Something went wrong");
            exit(1);
        }
    }
    return 0;
}


blocks *fcfs_load(pcb **pcbs, int pcb_count, char **args) {
    blocks *head_wrapper = malloc(sizeof(blocks)); // free OK
    head_wrapper->fcfs_head = load(pcbs, pcb_count);
    return head_wrapper;
}

int fcfs_startup(blocks *b, int executed) {
    return startup(b->fcfs_head, executed);
}

void fcfs_free_blocks(blocks* b){
    free_blocks(b->fcfs_head);
}

int fcfs_execute(blocks *b, int executed) {
    return execute(b->fcfs_head, executed);
}