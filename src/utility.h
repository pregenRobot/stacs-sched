
#ifndef utility
#define utility

int64_t nanos();
int64_t millis();
int64_t micros();

void log_startup(pcb *pcb_info);
void log_execute_start(pcb *pcb_info);
void log_execute_pause(pcb *pcb_info);
void log_execute_finish(pcb *pcb_info);
bool command_is_executable(char *command);

#endif