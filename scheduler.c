#include <stdint.h>
#include <stddef.h>
#include "scheduler.h"

// VGA color enum
enum vga_color
{
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

// External declarations from kernel.c
extern void terminal_writestring(const char *data);
extern void terminal_putchar(char c);
extern void terminal_setcolor(uint8_t color);
extern uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
extern size_t strlen(const char *str);

// Scheduler state
process_t processes[MAX_PROCESSES];
uint32_t num_processes = 0;
uint32_t current_process = 0;
uint32_t timer_ticks = 0;
uint32_t context_switches = 0;

// Timer and interrupt handling
void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

// Initialize PIT (Programmable Interval Timer)
void init_timer() {
    // Set PIT frequency to ~100Hz (10ms intervals)
    uint32_t divisor = 1193180 / 100;
    
    outb(0x43, 0x36);  // Command byte
    outb(0x40, divisor & 0xFF);        // Low byte
    outb(0x40, (divisor >> 8) & 0xFF); // High byte
    
    terminal_writestring("Timer initialized (100Hz)\n");
}

// Simple process functions to schedule
void process_a() {
    for (int i = 0; i < 3; i++) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("A");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        
        // Simulate work
        for (volatile int j = 0; j < 100000; j++);
    }
}

void process_b() {
    for (int i = 0; i < 3; i++) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
        terminal_writestring("B");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        
        // Simulate work
        for (volatile int j = 0; j < 100000; j++);
    }
}

void process_c() {
    for (int i = 0; i < 3; i++) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("C");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        
        // Simulate work
        for (volatile int j = 0; j < 100000; j++);
    }
}

void idle_process() {
    for (int i = 0; i < 2; i++) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK));
        terminal_writestring(".");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        
        // Simulate idle
        for (volatile int j = 0; j < 100000; j++);
    }
}

// Create a new process
uint32_t create_process(void (*entry_point)(), const char* name, uint32_t priority) {
    if (num_processes >= MAX_PROCESSES) return 0;
    
    process_t* proc = &processes[num_processes];
    proc->pid = num_processes + 1;
    proc->state = PROCESS_READY;
    proc->priority = priority;
    proc->time_slice = TIME_SLICE;
    proc->cpu_time = 0;
    
    // Copy process name
    for (int i = 0; i < 15 && name[i]; i++) {
        proc->name[i] = name[i];
    }
    proc->name[15] = '\0';
    
    // Set up stack
    proc->esp = (uint32_t)&proc->stack[STACK_SIZE - 4];
    proc->ebp = proc->esp;
    proc->eip = (uint32_t)entry_point;
    
    // Initialize stack with entry point
    uint32_t* stack_ptr = (uint32_t*)proc->esp;
    *stack_ptr = (uint32_t)entry_point;
    
    num_processes++;
    
    return proc->pid;
}

// Context switch function
void context_switch() {
    if (num_processes <= 1) return;
    
    // Save current process state (simplified)
    process_t* current = &processes[current_process];
    if (current->state == PROCESS_RUNNING) {
        current->state = PROCESS_READY;
    }
    
    // Find next ready process (Round Robin)
    uint32_t next = (current_process + 1) % num_processes;
    uint32_t start = next;
    
    do {
        if (processes[next].state == PROCESS_READY) {
            break;
        }
        next = (next + 1) % num_processes;
    } while (next != start);
    
    // Switch to next process
    current_process = next;
    process_t* next_proc = &processes[current_process];
    next_proc->state = PROCESS_RUNNING;
    next_proc->time_slice = TIME_SLICE;
    
    context_switches++;
    
    // Display context switch info (inline, no newlines)
    terminal_writestring("|");
}

// Timer interrupt handler (simplified)
void timer_handler() {
    timer_ticks++;
    
    if (num_processes == 0) return;
    
    process_t* current = &processes[current_process];
    current->time_slice--;
    current->cpu_time++;
    
    // Preemptive scheduling - time slice expired
    if (current->time_slice <= 0) {
        context_switch();
    }
}

// Simulate timer interrupts
void simulate_scheduling() {
    terminal_writestring("\n=== PROCESS SCHEDULING DEMO ===\n");
    terminal_writestring("Round-Robin Scheduler | Time Slice: 3 ticks\n\n");
    
    // Create processes
    create_process(process_a, "ProcessA", 1);
    create_process(process_b, "ProcessB", 1); 
    create_process(process_c, "ProcessC", 1);
    create_process(idle_process, "IdleProc", 0);
    
    terminal_writestring("Created 4 processes\n");
    terminal_writestring("Output: A=ProcessA, B=ProcessB, C=ProcessC, .=Idle, |=switch\n\n");
    
    // Start first process
    if (num_processes > 0) {
        processes[0].state = PROCESS_RUNNING;
        current_process = 0;
    }
    
    // Simulate scheduling for a period
    for (int cycle = 0; cycle < 20; cycle++) {
        // Simulate timer interrupt every few iterations
        if (cycle % 2 == 0) {
            timer_handler();
        }
        
        // Execute current process (simplified)
        process_t* current = &processes[current_process];
        if (current->state == PROCESS_RUNNING) {
            // Execute a small portion of the process
            if (current->pid == 1) { // ProcessA
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                terminal_writestring("A");
            } else if (current->pid == 2) { // ProcessB  
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
                terminal_writestring("B");
            } else if (current->pid == 3) { // ProcessC
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                terminal_writestring("C");
            } else { // Idle
                terminal_setcolor(vga_entry_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK));
                terminal_writestring(".");
            }
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        }
        
        // Small delay to visualize
        for (volatile int i = 0; i < 200000; i++);
    }
    
    terminal_writestring("\n\nScheduling complete! Switches: ");
    terminal_putchar('0' + ((context_switches / 10) % 10));
    terminal_putchar('0' + (context_switches % 10));
    terminal_writestring("\n");
}

void display_process_table() {
    terminal_writestring("\n=== PROCESS TABLE ===\n");
    terminal_writestring("PID  Name       State     Priority  CPU Time\n");
    terminal_writestring("---  ---------  --------  --------  --------\n");
    
    for (uint32_t i = 0; i < num_processes; i++) {
        process_t* proc = &processes[i];
        
        terminal_writestring(" ");
        terminal_putchar('0' + proc->pid);
        terminal_writestring("   ");
        terminal_writestring(proc->name);
        
        // Pad name
        for (int j = strlen(proc->name); j < 10; j++) {
            terminal_putchar(' ');
        }
        
        // State
        const char* state_str = (proc->state == PROCESS_READY) ? "READY   " :
                               (proc->state == PROCESS_RUNNING) ? "RUNNING " :
                               (proc->state == PROCESS_BLOCKED) ? "BLOCKED " : "TERM    ";
        terminal_writestring(state_str);
        
        terminal_writestring("    ");
        terminal_putchar('0' + proc->priority);
        terminal_writestring("         ");
        terminal_putchar('0' + ((proc->cpu_time / 10) % 10));
        terminal_putchar('0' + (proc->cpu_time % 10));
        terminal_writestring("\n");
    }
}

void display_final_stats() {
    terminal_writestring("\n=== SCHEDULING STATISTICS ===\n");
    terminal_writestring("Total Context Switches: ");
    terminal_putchar('0' + ((context_switches / 10) % 10));
    terminal_putchar('0' + (context_switches % 10));
    terminal_writestring("\n");
    
    terminal_writestring("Total Timer Ticks: ");
    terminal_putchar('0' + ((timer_ticks / 100) % 10));
    terminal_putchar('0' + ((timer_ticks / 10) % 10));
    terminal_putchar('0' + (timer_ticks % 10));
    terminal_writestring("\n");
    
    terminal_writestring("\nCPU Time Distribution:\n");
    for (uint32_t i = 0; i < num_processes; i++) {
        process_t* proc = &processes[i];
        terminal_writestring(proc->name);
        terminal_writestring(": ");
        terminal_putchar('0' + ((proc->cpu_time / 10) % 10));
        terminal_putchar('0' + (proc->cpu_time % 10));
        terminal_writestring(" ticks\n");
    }
    
    display_process_table();
}
