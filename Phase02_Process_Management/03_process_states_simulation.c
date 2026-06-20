/* Phase02/03_process_states_simulation.c
 * TOPIC: 5-State Process Model simulation
 * States: NEW → READY ↔ RUNNING → WAITING → TERMINATED
 * Compile: gcc -Wall -pthread -o proc_states 03_process_states_simulation.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

typedef enum { NEW, READY, RUNNING, WAITING, TERMINATED } PState;
const char *state_name[] = { "NEW      ", "READY    ", "RUNNING  ",
                              "WAITING  ", "TERMINATED" };

typedef struct {
    int    pid;
    PState state;
    char   name[32];
    int    cpu_bursts;  /* How many CPU+IO cycles */
} SimProcess;

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void delay_ms(int ms) {
    struct timespec ts = { 0, ms * 1000000L };
    nanosleep(&ts, NULL);
}

void log_transition(SimProcess *p, PState old_s, PState new_s, const char *reason) {
    pthread_mutex_lock(&print_lock);
    printf("[PID %d] %-12s  %s → %s  (%s)\n",
           p->pid, p->name,
           state_name[old_s], state_name[new_s], reason);
    pthread_mutex_unlock(&print_lock);
}

void *simulate_process(void *arg) {
    SimProcess *p = (SimProcess*)arg;

    /* NEW */
    p->state = NEW;
    log_transition(p, NEW, NEW, "created by OS");
    delay_ms(50);

    /* NEW → READY */
    p->state = READY;
    log_transition(p, NEW, READY, "admitted to ready queue");
    delay_ms(80 + (rand() % 60));

    for (int cycle = 0; cycle < p->cpu_bursts; cycle++) {
        /* READY → RUNNING */
        PState prev = p->state;
        p->state = RUNNING;
        log_transition(p, prev, RUNNING, "CPU dispatched");
        delay_ms(100 + (rand() % 100));  /* Simulate CPU burst */

        if (cycle < p->cpu_bursts - 1) {
            /* RUNNING → WAITING (I/O) */
            prev = p->state;
            p->state = WAITING;
            log_transition(p, prev, WAITING, "I/O requested");
            delay_ms(150 + (rand() % 100));  /* Simulate I/O */

            /* WAITING → READY */
            prev = p->state;
            p->state = READY;
            log_transition(p, prev, READY, "I/O completed");
            delay_ms(50);
        }
    }

    /* RUNNING → TERMINATED */
    p->state = TERMINATED;
    log_transition(p, RUNNING, TERMINATED, "process exited");
    return NULL;
}

int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║    PROCESS STATE MACHINE SIMULATION (5-State)  ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");
    printf("States: NEW → READY ↔ RUNNING ↔ WAITING → TERMINATED\n\n");
    printf("Key Rules:\n");
    printf("  • WAITING → RUNNING: NEVER direct (must go WAITING→READY→RUNNING)\n");
    printf("  • Only ONE process per CPU can be in RUNNING state\n");
    printf("  • Multiple processes can be in READY simultaneously\n\n");
    printf("%-6s %-14s  %-10s → %-10s  Reason\n","[PID]","[Name]","From","To");
    printf("------+-----------------------------------------------------------\n");

    srand(42);
    SimProcess procs[] = {
        {101, NEW, "browser",     2},
        {102, NEW, "database",    3},
        {103, NEW, "file_writer", 2},
        {104, NEW, "network_app", 3},
    };
    int n = sizeof(procs) / sizeof(procs[0]);

    pthread_t threads[4];
    for (int i = 0; i < n; i++)
        pthread_create(&threads[i], NULL, simulate_process, &procs[i]);
    for (int i = 0; i < n; i++)
        pthread_join(threads[i], NULL);

    printf("\n[ALL PROCESSES TERMINATED]\n\n");
    printf("=== State Transition Diagram ===\n");
    printf("                ┌─────────────────────────────┐\n");
    printf("                │         OS Kernel            │\n");
    printf(" new process    │                              │\n");
    printf("────────────▶ [NEW]                            │\n");
    printf("                │  admitted                    │\n");
    printf("                ▼                              │\n");
    printf("             [READY] ◀────────────────────┐   │\n");
    printf("                │ dispatcher               │   │\n");
    printf("                │ schedules                │   │\n");
    printf("                ▼                          │   │\n");
    printf("            [RUNNING] ──I/O request──▶ [WAITING]\n");
    printf("                │                          │   │\n");
    printf("                │ exit                     │   │\n");
    printf("                ▼                          │   │\n");
    printf("           [TERMINATED]   I/O done ────────┘   │\n");
    printf("                └─────────────────────────────┘\n");
    return 0;
}
