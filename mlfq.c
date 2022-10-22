#include "common.h"
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>



static mlfq_block* load(pcb** pcbs, int pcb_count){
    return (mlfq_block*)malloc(sizeof(mlfq_block));
}

static int startup(mlfq_block* head, int executed){
    return 0;
}

static int execute(mlfq_block* head, int executed){
    return 0;
}

blocks* mlfq_load(pcb** pcbs, int pcb_count, char** argv){
    blocks* head_wrapper = malloc(sizeof(blocks));
    head_wrapper->mlfq_head = load(pcbs, pcb_count);
    return head_wrapper;
}

int mlfq_startup(blocks* b, int executed){
    return startup(b->fifo_head, executed);
}

int mlfq_execute(blocks* b, int executed){
    return execute(b->fifo_head, executed);
}
