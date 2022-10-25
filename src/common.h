#ifndef common
#define common

#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <time.h>

typedef struct {
    int process_id;
    int status; // -2 finished (successfully or unsuccessfully) -1 loaded 0
                // waiting 1 running
    char *executable_path;
    char **arguments;
    int arg_count; // number of arguments -- for freeing
    int priority;
    char *full_line;
    int64_t begin;
    int64_t response_time;
    int64_t burst_time;
    int64_t turnaround_time;
    int64_t waiting_time;
} pcb; // process control block

// FCFS
typedef struct fcfs_block {
    pcb *info;
    struct fcfs_block *next;
} fcfs_block;

typedef struct rr_block {
    pcb *info;
    struct rr_block *next;
    int quantum;
} rr_block;

// wrapper for FCFS and p_rr data structures for inheritance
typedef struct blocks {
    fcfs_block *fcfs_head;
    rr_block *rr_head;
} blocks;

// Implementations for each scheduler
// FCFS - First in first Out
blocks *fcfs_load(pcb **pcbs, int pcb_count, char **args);
int fcfs_startup(blocks *b, int executed);
int fcfs_execute(blocks *b, int executed);
void fcfs_free_blocks(blocks *b);

// Priority Queue - Maxheap
blocks *rr_load(pcb **pcbs, int pcb_count, char **args);
int rr_startup(blocks *b, int executed);
int rr_execute(blocks *b, int executed);
void rr_free_blocks(blocks *b);

// Priority Queue
blocks *p_rr_load(pcb **pcbs, int pcb_count, char **args);

// Parsers and configurers
int readconfig(char ***commands_ref, char *path);
int parseconfig(char **commands_ref, pcb **pcbs, int command_count);
char **str_split(char *a_str, const char a_delim);
char *join_strings(char **strings, char *separator);
bool isNumeric(const char *s);
#define NONE ((void *)0)

typedef struct scheduler {
    blocks *(*loader)(pcb **, int, char **);
    int (*starter)(blocks *, int);
    int (*executor)(blocks *, int);
    void (*free_blocks)(blocks *);
} scheduler;

int handle_args(scheduler *target_scheduler, int argc, char **argv);
int log_stats(pcb **pcbs, int pcb_count);

void free_all(scheduler *target_scheduler, pcb **pcbs, int pcb_count,
              blocks *head, char **commands, int command_count);
#endif