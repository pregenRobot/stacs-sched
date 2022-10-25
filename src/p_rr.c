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

// Generate the datastructure for the rr block
static rr_block *load(pcb **pcbs, int pcb_count, int quantum) {
    int i = 0;
    rr_block *head = calloc(1, sizeof(rr_block));
    // Create head
    head->info = pcbs[0];
    head->quantum = quantum;
    // Iterate over all the parsed commands
    for (i = 1; i < pcb_count; i++) {
        rr_block *insert_position = head;
        rr_block *new_node = malloc(sizeof(rr_block));
        // Swap heads if new priority is lower priority than the head
        if (head->info->priority >= pcbs[i]->priority) {
            rr_block *new_head = new_node;
            new_node->info = pcbs[i];
            new_head->next = head;
            head = new_head;
        } else {
            // iterate over all the nodes and find the appropriate insertion
            // position
            while (insert_position->next != 0x0) {
                if (insert_position->next->info->priority < pcbs[i]->priority) {
                    insert_position = insert_position->next;
                } else {
                    break;
                }
            }
            // Keep track of the nodes after the insertion position
            rr_block *tail = insert_position->next;
            // Insert new node in the insertion position
            insert_position->next = new_node;
            new_node->info = pcbs[i];
            // Reattach the tailing nodes
            new_node->next = tail;
        }
        new_node->quantum = quantum;
    }
    return head;
}

// Load the RR datastructure
blocks *p_rr_load(pcb **pcbs, int pcb_count, char **args) {
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