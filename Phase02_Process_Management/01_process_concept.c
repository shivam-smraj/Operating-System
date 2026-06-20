/* Phase02/01_process_concept.c
 * TOPIC: What is a Process? Memory layout, process anatomy
 * Compile: gcc -Wall -o process_concept 01_process_concept.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ── Global/Static variables — stored in DATA SEGMENT ── */
int global_initialized = 100;      /* Initialized data segment (.data) */
int global_uninitialized;          /* BSS segment (zero-initialized) */
static char static_str[] = "hello";  /* .data segment */

/* ── Text segment: this function's code is in TEXT segment ── */
void demonstrate_segments(void) {
    /* ── Stack variables: local to this function, on STACK ── */
    int stack_var = 42;
    char stack_array[100];

    /* ── Heap allocation ── */
    int *heap_ptr = malloc(sizeof(int) * 10);  /* Heap segment */
    if (!heap_ptr) { perror("malloc"); return; }
    heap_ptr[0] = 999;

    printf("\n=== Process Memory Layout ===\n");
    printf("(Addresses increase upward, stack at top grows down)\n\n");

    /* Print addresses to visualize the memory layout */
    printf("TEXT segment (code):  %p  [function addresses]\n",
           (void*)demonstrate_segments);
    printf("DATA segment (init):  %p  global_initialized=%d\n",
           (void*)&global_initialized, global_initialized);
    printf("BSS segment (uninit): %p  global_uninitialized=%d\n",
           (void*)&global_uninitialized, global_uninitialized);
    printf("HEAP:                 %p  heap_ptr[0]=%d\n",
           (void*)heap_ptr, heap_ptr[0]);
    printf("STACK:                %p  stack_var=%d\n",
           (void*)&stack_var, stack_var);

    printf("\nObserve: HEAP address < STACK address (heap grows up, stack grows down)\n");

    free(heap_ptr);  /* Always free heap allocations! */
}

/* ── Demonstrate process identity ── */
void demonstrate_process_identity(void) {
    printf("\n=== Process Identity ===\n");
    printf("PID  (Process ID):         %d\n", getpid());
    printf("PPID (Parent Process ID):  %d\n", getppid());
    printf("UID  (User ID):            %d\n", getuid());
    printf("EUID (Effective User ID):  %d\n", geteuid());
    printf("GID  (Group ID):           %d\n", getgid());

    /* Process image: what program is running */
    char exe_path[256];
    ssize_t n = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
    if (n > 0) {
        exe_path[n] = 0;
        printf("Executable path: %s\n", exe_path);
    }
}

/* ── Demonstrate environment ── */
void demonstrate_environment(void) {
    printf("\n=== Environment Variables (first 5) ===\n");
    /* environ[] is a global array of "KEY=VALUE" strings */
    extern char **environ;
    for (int i = 0; environ[i] && i < 5; i++) {
        printf("  %s\n", environ[i]);
    }
    printf("  ...\n");

    /* Look up specific env var */
    const char *path = getenv("PATH");
    if (path) printf("PATH = %s\n", path);
}

int main(int argc, char *argv[]) {
    printf("╔═══════════════════════════════════════╗\n");
    printf("║         PROCESS CONCEPT DEMO           ║\n");
    printf("╚═══════════════════════════════════════╝\n");

    demonstrate_segments();
    demonstrate_process_identity();
    demonstrate_environment();

    printf("\n[KEY CONCEPTS]\n");
    printf("1. Process = Program + Execution State + Memory Space\n");
    printf("2. Memory layout: Text → Data → BSS → Heap ↑ ... ↓ Stack\n");
    printf("3. Each process has unique PID, inherits environment from parent\n");
    printf("4. Process can read its own /proc/self/ for self-introspection\n");

    return 0;
}
