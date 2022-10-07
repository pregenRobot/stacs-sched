#ifndef common
#define common

#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>

typedef struct {
    int process_id;
    int status;
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

// wrapper for FIFO and PQ data structures for inheritance
typedef struct blocks {
    fifo_block *fifo_head;
    pq_block *pq_head;
} blocks;

// Generic function definitions
typedef blocks* (*load_func)(pcb**, int);
typedef int (*startup_func)(blocks*, int);
typedef int (*execute_func)(blocks*, int);

// Implementations for each scheduler
// FIFO - First in first Out
blocks* fifo_load(pcb** pcbs, int pcb_count);
int fifo_startup(blocks* b, int executed);
int fifo_execute(blocks* b, int executed);

// Priority Queue - Maxheap
blocks* pq_load(pcb** pcbs, int pcb_count);
int pq_startup(blocks* b, int executed);
int pq_execute(blocks* b, int executed);

// Parsers and configurers
int readconfig(char **commands_ref);
int parseconfig(char **commands_ref, pcb pcbs, int command_count);
char** str_split(char *a_str, const char a_delim);
char* join_strings(char **strings, char *separator);
int handle_args(load_func loader, startup_func starter, execute_func executor, int argc, char** argv);
#endif