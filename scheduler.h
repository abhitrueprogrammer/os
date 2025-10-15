#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

// REAL PROCESS SCHEDULING IMPLEMENTATION

#define MAX_PROCESSES 5
#define STACK_SIZE 4096
#define TIME_SLICE 3  // Timer ticks per process

typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING, 
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

// Process Control Block (PCB)
typedef struct {
    uint32_t pid;
    char name[16];
    process_state_t state;
    uint32_t esp;           // Stack pointer
    uint32_t ebp;           // Base pointer  
    uint32_t eip;           // Instruction pointer
    uint32_t priority;
    uint32_t time_slice;
    uint32_t cpu_time;      // Total CPU time used
    uint8_t stack[STACK_SIZE]; // Process stack
} process_t;

// Function declarations
void init_timer(void);
uint32_t create_process(void (*entry_point)(), const char* name, uint32_t priority);
void context_switch(void);
void timer_handler(void);
void simulate_scheduling(void);
void display_process_table(void);
void display_final_stats(void);

// Process functions
void process_a(void);
void process_b(void);
void process_c(void);
void idle_process(void);

#endif // SCHEDULER_H
