#include "common.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
/// Convert seconds to milliseconds
#define SEC_TO_MS(sec) ((sec)*1000)
/// Convert seconds to microseconds
#define SEC_TO_US(sec) ((sec)*1000000)
/// Convert seconds to nanoseconds
#define SEC_TO_NS(sec) ((sec)*1000000000)

/// Convert nanoseconds to seconds
#define NS_TO_SEC(ns) ((ns) / 1000000000)
/// Convert nanoseconds to milliseconds
#define NS_TO_MS(ns) ((ns) / 1000000)
/// Convert nanoseconds to microseconds
#define NS_TO_US(ns) ((ns) / 1000)

/// Get a time stamp in milliseconds.
uint64_t millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t ms =
        SEC_TO_MS((uint64_t)ts.tv_sec) + NS_TO_MS((uint64_t)ts.tv_nsec);
    return ms;
}

/// Get a time stamp in microseconds.
uint64_t micros() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t us =
        SEC_TO_US((uint64_t)ts.tv_sec) + NS_TO_US((uint64_t)ts.tv_nsec);
    return us;
}

/// Get a time stamp in nanoseconds.
uint64_t nanos() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t ns = SEC_TO_NS((uint64_t)ts.tv_sec) + (uint64_t)ts.tv_nsec;
    return ns;
}

void log_startup(pcb *pcb_info) {
    pcb_info->response_time = -1;
    pcb_info->begin = micros();
}

int64_t burst_start;
void log_execute_start(pcb *pcb_info) {
    burst_start = micros();
    if (pcb_info->response_time == -1) {
        pcb_info->response_time = burst_start - pcb_info->begin;
        pcb_info->burst_time = 0;
    }
}

void log_execute_pause(pcb *pcb_info) {
    int64_t burst_end = micros();
    pcb_info->burst_time += burst_end - burst_start;
}

void log_execute_finish(pcb *pcb_info) {
    int64_t burst_end = micros();
    pcb_info->turnaround_time = burst_end - pcb_info->begin;
    pcb_info->burst_time += burst_end - burst_start;
    pcb_info->waiting_time = pcb_info->turnaround_time - pcb_info->burst_time;
}

// https://stackoverflow.com/questions/890894/portable-way-to-find-out-if-a-command-exists-c-c
bool command_is_executable(char *command) {
    char whichcommand[strlen(command) + 25];
    sprintf(whichcommand, "which %s > /dev/null 2>&1", command);
    if (system(whichcommand) == 0) {
        // command exists in path
        return true;
    } else {
        return false;
    }
}