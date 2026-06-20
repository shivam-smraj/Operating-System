/* Phase02/02_fork_exec_wait.c
 * TOPIC: fork(), exec(), wait() — The Process Creation Trinity
 * Compile: gcc -Wall -o fork_exec_wait 02_fork_exec_wait.c
 *
 * KEY CONCEPT:
 *   fork()  → creates a COPY of the calling process
 *   exec()  → REPLACES the process image with a new program
 *   wait()  → parent WAITS for child to finish, reaps exit status
 *
 * These three calls together implement: "run a program"
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

/* ─── Demo 1: Basic fork ─── */
void demo_basic_fork(void) {
    printf("\n=== Demo 1: Basic fork() ===\n");

    pid_t pid = fork();   /* Returns TWICE! */

    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        /* Child process */
        printf("CHILD:  pid=%d, ppid=%d\n", getpid(), getppid());
        exit(0);
    } else {
        /* Parent process */
        printf("PARENT: pid=%d, child_pid=%d\n", getpid(), pid);
        int status;
        waitpid(pid, &status, 0);   /* Reap the child */
        if (WIFEXITED(status))
            printf("PARENT: child exited with code %d\n", WEXITSTATUS(status));
    }
}

/* ─── Demo 2: fork + exec ─── */
void demo_fork_exec(void) {
    printf("\n=== Demo 2: fork() + exec() ===\n");
    printf("Parent will fork, child will exec 'ls -l /tmp'\n");

    pid_t pid = fork();
    if (pid == 0) {
        /* Child: replace itself with 'ls' */
        printf("Child (PID=%d): calling execl('ls')...\n", getpid());
        execl("/bin/ls", "ls", "-l", "/tmp", NULL);
        /* If execl returns → error! */
        perror("execl failed");
        exit(1);
    }
    /* Parent waits */
    int status;
    wait(&status);
    printf("Parent: ls command finished with status %d\n", WEXITSTATUS(status));
}

/* ─── Demo 3: exec variants ─── */
void demo_exec_variants(void) {
    printf("\n=== Demo 3: exec() Variants ===\n");
    printf("exec family: execl, execlp, execle, execv, execvp, execve\n\n");

    pid_t pid = fork();
    if (pid == 0) {
        /* execvp: search PATH, use array for args */
        char *args[] = {"echo", "Hello from execvp!", NULL};
        execvp("echo", args);
        perror("execvp"); exit(1);
    }
    wait(NULL);

    pid = fork();
    if (pid == 0) {
        /* execlp: search PATH, use variadic args */
        execlp("date", "date", "+%Y-%m-%d %H:%M:%S", NULL);
        perror("execlp"); exit(1);
    }
    wait(NULL);
}

/* ─── Demo 4: Exit status codes ─── */
void demo_exit_status(void) {
    printf("\n=== Demo 4: Exit Status Codes ===\n");

    /* Normal exit */
    pid_t pid = fork();
    if (pid == 0) { exit(42); }  /* Exit with code 42 */
    int status;
    waitpid(pid, &status, 0);
    printf("WIFEXITED=%d  WEXITSTATUS=%d\n",
           WIFEXITED(status), WEXITSTATUS(status));

    /* Signal termination */
    pid = fork();
    if (pid == 0) {
        raise(SIGSEGV);   /* Simulate crash (segfault) */
        exit(0);
    }
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status))
        printf("Child killed by signal %d (%s)\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));

    printf("\nStatus macros:\n");
    printf("  WIFEXITED(status)    → true if normal exit\n");
    printf("  WEXITSTATUS(status)  → exit code (0-255)\n");
    printf("  WIFSIGNALED(status)  → true if killed by signal\n");
    printf("  WTERMSIG(status)     → which signal killed it\n");
    printf("  WIFSTOPPED(status)   → true if stopped by SIGSTOP\n");
}

/* ─── Demo 5: fork bomb (safe - limited) ─── */
void explain_fork_bomb(void) {
    printf("\n=== Demo 5: Fork Bomb (EXPLAINED, not run!) ===\n");
    printf("The most dangerous shell one-liner: :(){ :|:& };:\n\n");
    printf("This is a shell function ':' that:\n");
    printf("  - Calls itself recursively: :|:\n");
    printf("  - Runs the call in background: &\n");
    printf("  - Result: exponential process creation → system freeze\n\n");
    printf("Prevention:\n");
    printf("  ulimit -u 100     (limit to 100 processes per user)\n");
    printf("  /etc/security/limits.conf: student hard nproc 100\n");
    printf("  cgroups: echo 100 > /sys/fs/cgroup/pids/user.slice/pids.max\n\n");
    printf("Try: ulimit -u  (shows your current process limit)\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════╗\n");
    printf("║   fork() + exec() + wait() — Deep Dive   ║\n");
    printf("╚══════════════════════════════════════════╝\n");

    demo_basic_fork();
    demo_fork_exec();
    demo_exec_variants();
    demo_exit_status();
    explain_fork_bomb();

    printf("\n[SUMMARY]\n");
    printf("fork():  Create child as copy of parent (returns twice)\n");
    printf("exec():  Replace process image — no return on success\n");
    printf("wait():  Reap child exit status — prevents zombies\n");
    printf("Shell implementation: for each command, fork+exec+wait\n");
    return 0;
}
