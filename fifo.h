#ifndef fifo
#define fifo

#include <stdbool.h>
#include <sys/types.h>
#include "./common.h"

typedef struct fifo_block {
    process_info* info;
    struct fifo_block* next_block;
} fifo_block;

fifo_block* load(process_info** processes, int process_count);

int initial_execution(fifo_block* head, int executed);

int execute(fifo_block* head, int executed);

#endif