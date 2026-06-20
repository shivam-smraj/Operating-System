/* Phase02/06_zombie_orphan_process.c
 * Demonstrates zombie and orphan processes — a classic interview topic
 * Compile: gcc -Wall -o zombie_orphan 06_zombie_orphan_process.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void demo_zombie(void) {
    printf("\n=== ZOMBIE PROCESS DEMO ===\n");
    pid_t pid = fork();
    if(pid == 0) {
        printf("Child (PID=%d): Exiting now — parent won't wait() for 3 seconds!\n",
               getpid());
        exit(0);
    }
    printf("Parent (PID=%d): Child %d exited. NOT calling wait() for 3s.\n",
           getpid(), pid);
    printf("Run: ps aux | grep Z    to see zombie!\n");
    sleep(3);
    int status;
    waitpid(pid, &status, 0);
    printf("Parent: Called wait(). Zombie REAPED — PCB freed.\n");
}

void demo_orphan(void) {
    printf("\n=== ORPHAN PROCESS DEMO ===\n");
    pid_t pid = fork();
    if(pid == 0) {
        printf("Child (PID=%d): My parent is PID=%d\n", getpid(), getppid());
        sleep(1);
        /* After parent exits, child gets reparented to init (PID 1) */
        printf("Child: After orphaning, new parent PID=%d (should be 1=systemd)\n",
               getppid());
        exit(0);
    }
    printf("Parent (PID=%d): Exiting without waiting! Child %d will be orphaned.\n",
           getpid(), pid);
    /* Parent exits without calling wait() */
}

int main(void) {
    printf("╔══════════════════════════════════╗\n");
    printf("║   ZOMBIE AND ORPHAN PROCESSES     ║\n");
    printf("╚══════════════════════════════════╝\n");

    demo_zombie();
    demo_orphan();
    sleep(2);  /* Let orphan child finish */

    printf("\n[KEY CONCEPTS]\n");
    printf("ZOMBIE: child exited, parent not called wait()\n");
    printf("  - PCB stays in process table, takes up a slot\n");
    printf("  - Fix: parent calls wait(), or ignore SIGCHLD\n\n");
    printf("ORPHAN: parent exited before child\n");
    printf("  - OS reparents child to init (PID 1)\n");
    printf("  - init auto-waits for all adopted children\n");
    printf("  - Orphans are NOT zombies — they keep running\n");
    return 0;
}
