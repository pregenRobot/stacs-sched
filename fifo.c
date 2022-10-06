#include <stdio.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include "fifo.h"
#include <sys/wait.h>
#include <unistd.h>

fifo_block* load(process_info** processes, int process_count){
    int i = 0;
    fifo_block* head = malloc(sizeof(fifo_block));
    head->info = processes[0];
    fifo_block* current_node = head;
    for(i = 1; i < process_count; i++){
        fifo_block* next_node = malloc(sizeof(fifo_block));
        current_node->next_block = next_node;
        current_node = current_node->next_block;
        current_node->info = processes[i];
    }
    current_node->next_block = NULL;
    return head;
}

int execute(fifo_block* head, int executed){
    fifo_block* current = head;
    
    if(current != NULL){
        pid_t pid = fork();
        if(pid == 0){
            // child
            current->info->current_status = 1;
            execl(current->info->executable_path, current->info->arguments);
        }else if (pid > 0){
            // parent
            int status;
            pid_t terminated;
            terminated = waitpid(pid, &status, 0);
            
            // Schudler should allow this as well
            // if(terminated == -1){
            //     perror("child Terminated with error");
            //     exit(1);
            // }

            if(WIFEXITED(status)){
                // run next block
                return execute(current->next_block, executed + 1);
            }else{
                perror("Something waint wrong");
                exit(1);
            }
        }
    }
    return 0;
}

