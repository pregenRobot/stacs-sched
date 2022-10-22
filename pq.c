#include "common.h"
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>



static pq_block* load(pcb** pcbs, int pcb_count){
    return (pq_block*)malloc(sizeof(pq_block));
}

static int startup(pq_block* head, int executed){
    return 0;
}

static int execute(pq_block* head, int executed){
    return 0;
}

blocks* pq_load(pcb** pcbs, int pcb_count, char** argv){
    blocks* head_wrapper = malloc(sizeof(blocks));
    head_wrapper->pq_head = load(pcbs, pcb_count);
    return head_wrapper;
}

int pq_startup(blocks* b, int executed){
    return startup(b->fifo_head, executed);
}

int pq_execute(blocks* b, int executed){
    return execute(b->fifo_head, executed);
}
