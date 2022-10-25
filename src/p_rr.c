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

static rr_block *load(pcb **pcbs, int pcb_count, int quantum) {
    int i = 0;
    rr_block *head = calloc(1,sizeof(rr_block));
    head->info = pcbs[0];
    head->quantum = quantum;
    for (i =1; i < pcb_count; i++){
        rr_block *insert_position = head;
        rr_block *new_node = malloc(sizeof(rr_block));
        // swap heads if new priority is lower priority than the head
        if(head->info->priority >= pcbs[i]->priority){
            rr_block* new_head = new_node;
            new_node->info = pcbs[i];
            new_head->next = head;
            head = new_head;
        }else{
            // iterate over all the nodes and insert in the appropriate position
            while(insert_position->next != 0x0){
                if(insert_position->next->info->priority < pcbs[i]->priority){
                    insert_position = insert_position->next;
                }else{
                    break;
                }
            }
            rr_block *tail = insert_position->next;
            insert_position->next = new_node;
            new_node->info = pcbs[i];
            new_node->next = tail;
        }
        new_node->quantum = quantum;
    }
    return head;
}

static int startup(rr_block *head, int executed) {
}

static void free_blocks(rr_block* head){
}


static int execute(rr_block *head, int executed) {
}

void p_rr_free_blocks(blocks* b){
    free_blocks(b->rr_head);
}

int p_rr_startup(blocks *b, int commands) {
    return startup(b->rr_head, 0);
}

int p_rr_execute(blocks *b, int commands) {
}

blocks *p_rr_load(pcb **pcbs, int pcb_count, char **args) {
    blocks *head_wrapper = malloc(sizeof(blocks)); // free OK
    int quantum = (int)((uintmax_t) strtoumax(args[0], NULL, 10));
    if(quantum <= 0){
        printf("Invalid quantum for RR scheduler\n");
        exit(1);
    }
    head_wrapper->rr_head = load(pcbs,pcb_count, quantum);
    return head_wrapper;
}