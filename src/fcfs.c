#include "common.h"
#include "utility.h"
#include <inttypes.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// Generate the datastructure for the fcfs block
static fcfs_block *load(pcb **pcbs, int pcb_count) {
    int i = 0;
    fcfs_block *head = malloc(sizeof(fcfs_block)); // free OK
    head->info = pcbs[0];
    fcfs_block *current = head;
    // Iterate over all the parsed commands and generate a linked list
    for (i = 1; i < pcb_count; i++) {
        fcfs_block *next = malloc(sizeof(fcfs_block)); // free OK
        current->next = next;
        current = current->next;
        current->info = pcbs[i];
    }
    current->next = NULL;
    return head;
}

// Free FCFS memory
static void free_blocks(fcfs_block *head) {
    // While current node is not null, recursively free the memory in the linked
    // list
    if (head != NULL) {
        free_blocks(head->next);
        free(head);
    }
}

// Start the programs recursively
static int startup(fcfs_block *head, int executed) {
    fcfs_block *current = head;
    if (current != NULL) {
        // Handle case where command is not found
        if (!command_is_executable(current->info->executable_path)) {
            printf("Cannot find executable %s - file does not exist and it is "
                   "not in "
                   "PATH. Skipping...",
                   current->info->executable_path);
            current->info->status = -2;
            return startup(current->next, executed) + 1;
        }

        // Spawn the new child process which should run the specified command
        int pid = fork();
        if (pid == 0) {
            current->info->status = 1;

            execvp(current->info->executable_path, current->info->arguments);
        } else if (pid > 1) {
            // Parent immediately stops the process and moves on to the next
            // command
            kill(pid, SIGSTOP);
            printf("Priority: %d Command: %s  - pid: %d - cpu: %d\n",
                   current->info->priority, current->info->executable_path, pid,
                   sched_getcpu());
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
    // While the current node is not null (recursive defintion)
    if (current != NULL) {
        // Log the execution start time
        log_execute_start(current->info);
        // Run the program
        kill(current->info->process_id, SIGCONT);
        current->info->status = 1;
        int status;
        // Wait for program to terminate
        int terminated = waitpid(current->info->process_id, &status, 0);

        if (WIFEXITED(status)) {
            // Log the execution finish time
            log_execute_finish(current->info);
            // Move on to the next node
            return execute(current->next, executed) + 1;
        } else {
            perror("Something went wrong");
            exit(1);
        }
    }
    return 0;
}

// Load the FCFS datastructure
blocks *fcfs_load(pcb **pcbs, int pcb_count, char **args) {
    blocks *head_wrapper = malloc(sizeof(blocks)); // free OK
    head_wrapper->fcfs_head = load(pcbs, pcb_count);
    return head_wrapper;
}

// Startup all the commands
int fcfs_startup(blocks *b, int executed) {
    return startup(b->fcfs_head, executed);
}

// Free FCFS datastructure memory
void fcfs_free_blocks(blocks *b) { free_blocks(b->fcfs_head); }

// Execute all programs in FCFS fashion
int fcfs_execute(blocks *b, int executed) {
    return execute(b->fcfs_head, executed);
}