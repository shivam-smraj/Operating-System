/* Phase01/01_what_is_os.c
 * TOPIC: What is an Operating System?
 * Compile: gcc -Wall -o what_is_os 01_what_is_os.c
 * Run: ./what_is_os
 *
 * This program demonstrates how we interact with the OS through system calls
 * and shows basic OS information retrieval.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <time.h>

void print_separator(const char *title) {
    printf("\n============================\n");
    printf("  %s\n", title);
    printf("============================\n");
}

int main(void) {
    print_separator("OS INFORMATION");

    /* Get OS details via uname() system call */
    struct utsname u;
    if (uname(&u) == 0) {
        printf("System Name : %s\n", u.sysname);
        printf("Node Name   : %s\n", u.nodename);
        printf("Release     : %s\n", u.release);
        printf("Version     : %s\n", u.version);
        printf("Machine     : %s\n", u.machine);
    }

    print_separator("PROCESS INFORMATION");
    printf("My PID      : %d\n", getpid());
    printf("Parent PID  : %d\n", getppid());
    printf("User ID     : %d\n", getuid());
    printf("Group ID    : %d\n", getgid());
    printf("Am I root?  : %s\n", getuid() == 0 ? "YES" : "NO");

    print_separator("RESOURCE LIMITS");
    struct rlimit rl;
    /* RLIMIT_NOFILE: max number of open file descriptors */
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
        printf("Max open files: current=%lu, max=%lu\n",
               (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
    /* RLIMIT_STACK: max stack size */
    if (getrlimit(RLIMIT_STACK, &rl) == 0)
        printf("Max stack size: %lu MB\n",
               (unsigned long)(rl.rlim_cur / (1024*1024)));

    print_separator("SYSTEM CALLS DEMO");
    /* gettimeofday is often implemented as VDSO (no real syscall!) */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("Uptime-like monotonic clock: %ld.%09ld seconds\n",
           ts.tv_sec, ts.tv_nsec);

    printf("\n[CONCEPTS COVERED]\n");
    printf("1. OS provides hardware abstraction (uname, getpid, getrlimit)\n");
    printf("2. Every process interaction with hardware goes through OS\n");
    printf("3. OS enforces resource limits (RLIMIT_NOFILE, RLIMIT_STACK)\n");
    printf("4. System calls are the interface between user programs and OS\n");

    return 0;
}
