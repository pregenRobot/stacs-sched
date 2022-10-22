
#ifndef timer
#define timer


int64_t nanos();
int64_t millis();
int64_t micros();

void log_startup(pcb *pcb_info);
void log_execute_start(pcb *pcb_info);
void log_execute_pause(pcb *pcb_info);
void log_execute_finish(pcb *pcb_info);

#endif