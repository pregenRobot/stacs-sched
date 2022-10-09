#ifndef common
#define common

#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>

typedef struct {
    int process_id;
    int status; // -1 finished 0 waiting 1 running
    char *executable_path;
    char *arguments;
    int priority;
} pcb; // process control block

// FIFO
typedef struct fifo_block {
    pcb *info;
    struct fifo_block* next;
} fifo_block;

// PQ
typedef struct pq_block {
    pcb *info;
    struct pq_block *left;
    struct pq_block *right;
} pq_block;

typedef struct rr_block {
    pcb *info;
    struct rr_block *next;
    int quantum;
} rr_block;

// wrapper for FIFO and PQ data structures for inheritance
typedef struct blocks {
    fifo_block *fifo_head;
    pq_block *pq_head;
    rr_block *rr_head;
} blocks;

// Implementations for each scheduler
// FIFO - First in first Out
blocks* fifo_load(pcb** pcbs, int pcb_count, char** args);
int fifo_startup(blocks* b, int executed);
int fifo_execute(blocks* b, int executed);

// Priority Queue - Maxheap
blocks* rr_load(pcb** pcbs, int pcb_count, char** args);
int rr_startup(blocks* b, int executed);
int rr_execute(blocks* b, int executed);

// Priority Queue - Maxheap
blocks* pq_load(pcb** pcbs, int pcb_count, char** args);
int pq_startup(blocks* b, int executed);
int pq_execute(blocks* b, int executed);

// Parsers and configurers
int readconfig(char ***commands_ref, char* path);
int parseconfig(char **commands_ref, pcb **pcbs, int command_count);
char** str_split(char *a_str, const char a_delim);
char* join_strings(char **strings, char *separator);

typedef struct scheduler {
    blocks* (*loader)(pcb**, int, char**);
    int(*starter)(blocks*, int);
    int(*executor)(blocks*, int);
} scheduler;

int handle_args(
    scheduler* target_scheduler,
    int argc,
    char** argv
);
#endif