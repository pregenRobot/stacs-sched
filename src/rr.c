
#include "common.h"
#include "utility.h"
#include <errno.h>
#include <inttypes.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Generate the datastrucute for the rr block
static rr_block *load(pcb **pcbs, int pcb_count, int quantum) {
    int i = 0;
    rr_block *head = malloc(sizeof(rr_block)); // free OK
    head->info = pcbs[0];
    head->quantum = quantum;
    printf("RR Config - Using quantum %d\n", quantum);
    rr_block *current = head;
    // Iterate over all the parsed commands and generate a linked list
    for (i = 1; i < pcb_count; i++) {
        rr_block *next = malloc(sizeof(fcfs_block)); // free OK
        current->next = next;
        current = current->next;
        current->info = pcbs[i];
        current->quantum = quantum;
    }
    current->next = NULL;
    return head;
}

// Start the programs recursively
static int startup(rr_block *head, int executed) {
    rr_block *current = head;

    if (current != NULL) {

        // Handle cases where command is not found
        if (!command_is_executable(current->info->executable_path)) {
            printf("Cannot find executable %s - file does not exist and it is "
                   "not in "
                   "PATH. Skipping...",
                   current->info->executable_path);
            current->info->status = -2;
            return startup(current->next, executed) + 1;
        }

        // Spawn the new child processes which should run the specified command
        int pid = fork();
        if (pid == 0) {
            current->info->status = 1;
            execvp(current->info->executable_path, current->info->arguments);
        } else {
            // Parent immediately stops the process and moves on the next
            // command
            kill(pid, SIGSTOP);
            printf("Priority: %d Command: %s  - pid: %d - cpu: %d\n",
                   current->info->priority, current->info->executable_path, pid,
                   sched_getcpu());
            current->info->process_id = pid;
            current->info->status = 0;
            log_startup(current->info);
            return startup(current->next, executed) + 1;
        }
    }
    return 0;
}

// Free RR memory
static void free_blocks(rr_block *head) {
    // While current node is not null, recursively free the memory in the linked
    // list
    if (head != NULL) {
        free_blocks(head->next);
        free(head);
    }
}

static int execute(rr_block *head, int executed) {
    rr_block *current = head;
    // While we are not at the end of the list of processes
    if (current != NULL) {
        // If process has not finished executing (verify from status)
        if (current->info->status == 0) {

            // Log the execution start time
            log_execute_start(current->info);

            // Run the program without stopping execution of the parent (the
            // scheduler itself)
            kill(current->info->process_id, SIGCONT);
            current->info->status = 1;
            int status;
            // Wait for the quantum time to pass
            int waiting_result =
                waitpid(current->info->process_id, &status, WNOHANG);
            usleep((__useconds_t)current->quantum);
            // Pause programm
            kill(current->info->process_id, SIGSTOP);

            // If child is still running
            if (waiting_result == 0) {
                // Log pause time
                log_execute_pause(current->info);
                // Indicate child has not finished running
                current->info->status = 0;
                // Move on to the next command
                return execute(current->next, executed);
            } else if (waiting_result == current->info->process_id) {
                // Otherwise indicate the child has finished running
                current->info->status = -2;
                // Log pause time
                log_execute_finish(current->info);
                // Move on to the next command
                return execute(current->next, executed) + 1;
            }
        } else {
            // Move on to the next command
            return execute(current->next, executed) + 1;
        }
    }
    return 0;
}

// Free RR datastructure memory
void rr_free_blocks(blocks *b) { free_blocks(b->rr_head); }

// Start the programs
int rr_startup(blocks *b, int commands) { return startup(b->rr_head, 0); }

// Execute all the programs in RR fashion
int rr_execute(blocks *b, int commands) {
    int finished_execution = 0;
    // While all the commands have not finished, execute it
    while (finished_execution < commands) {
        finished_execution = execute(b->rr_head, 0);
    }
}

// Load the RR datastructure
blocks *rr_load(pcb **pcbs, int pcb_count, char **args) {
    blocks *head_wrapper = malloc(sizeof(blocks)); // free OK
    int quantum = (int)((uintmax_t)strtoumax(args[0], NULL, 10));
    // Quantum must be greater than 0
    if (quantum <= 0) {
        printf("Invalid quantum for RR scheduler\n");
        exit(1);
    }
    head_wrapper->rr_head = load(pcbs, pcb_count, quantum);
    return head_wrapper;
}