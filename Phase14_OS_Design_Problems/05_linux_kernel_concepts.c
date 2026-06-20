/* Phase14/05_linux_kernel_concepts.c
 * TOPIC: Key Linux kernel concepts explained with code
 * Covers: /proc filesystem, cgroups, namespaces, signals, system calls
 * Compile: gcc -Wall -o kernel_concepts 05_linux_kernel_concepts.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <errno.h>

/* ─── 1. System information via uname ─── */
void show_kernel_info(void) {
    printf("=== 1. Kernel Information ===\n");
    struct utsname u;
    uname(&u);
    printf("OS:       %s %s\n", u.sysname, u.release);
    printf("Version:  %s\n", u.version);
    printf("Machine:  %s\n", u.machine);
    printf("Hostname: %s\n\n", u.nodename);
}

/* ─── 2. /proc virtual filesystem ─── */
void explore_proc(void) {
    printf("=== 2. /proc Virtual Filesystem ===\n");
    printf("/proc is NOT on disk — kernel generates it dynamically!\n\n");

    /* Read uptime */
    FILE *f = fopen("/proc/uptime", "r");
    if (f) {
        double uptime, idle;
        fscanf(f, "%lf %lf", &uptime, &idle);
        fclose(f);
        printf("/proc/uptime: %.0f seconds = %.1f hours uptime\n",
               uptime, uptime/3600);
        printf("  Idle time: %.0f seconds (multi-core sum)\n\n", idle);
    }

    /* Read meminfo summary */
    f = fopen("/proc/meminfo", "r");
    if (f) {
        char line[128]; int printed=0;
        while(fgets(line,sizeof(line),f) && printed < 4) {
            if(strncmp(line,"MemTotal",8)==0 ||
               strncmp(line,"MemFree",7)==0 ||
               strncmp(line,"MemAvailable",12)==0 ||
               strncmp(line,"SwapTotal",9)==0) {
                printf("/proc/meminfo: %s", line);
                printed++;
            }
        }
        fclose(f);
    }

    /* Read our own stat */
    printf("\n/proc/self/status key fields:\n");
    f = fopen("/proc/self/status", "r");
    if (f) {
        char line[128]; int cnt=0;
        while(fgets(line,sizeof(line),f) && cnt<5) {
            if(strncmp(line,"Name:",5)==0 || strncmp(line,"VmRSS:",6)==0 ||
               strncmp(line,"VmSize:",7)==0 || strncmp(line,"Threads:",8)==0 ||
               strncmp(line,"Pid:",4)==0) {
                printf("  %s", line); cnt++;
            }
        }
        fclose(f);
    }

    /* Useful /proc paths */
    printf("\nKey /proc paths:\n");
    printf("  /proc/<pid>/maps      — virtual memory layout\n");
    printf("  /proc/<pid>/fd/       — open file descriptors\n");
    printf("  /proc/<pid>/cmdline   — command line arguments\n");
    printf("  /proc/<pid>/exe       — symlink to executable\n");
    printf("  /proc/cpuinfo         — CPU details\n");
    printf("  /proc/net/tcp         — TCP connections\n");
    printf("  /proc/sys/vm/         — VM tuning parameters\n\n");
}

/* ─── 3. Resource limits (ulimit) ─── */
void show_resource_limits(void) {
    printf("=== 3. Resource Limits (ulimit / getrlimit) ===\n");

    struct { int res; const char *name; } limits[] = {
        {RLIMIT_STACK,  "Stack size"},
        {RLIMIT_NOFILE, "Open files"},
        {RLIMIT_NPROC,  "Max processes"},
        {RLIMIT_AS,     "Virtual memory"},
        {RLIMIT_CPU,    "CPU time (s)"},
    };

    for (int i=0; i < 5; i++) {
        struct rlimit rl;
        getrlimit(limits[i].res, &rl);
        printf("  %-20s: soft=", limits[i].name);
        if(rl.rlim_cur == RLIM_INFINITY) printf("unlimited");
        else printf("%lu", (unsigned long)rl.rlim_cur);
        printf("  hard=");
        if(rl.rlim_max == RLIM_INFINITY) printf("unlimited\n");
        else printf("%lu\n", (unsigned long)rl.rlim_max);
    }
    printf("\nSet in: /etc/security/limits.conf or ulimit command\n");
    printf("Used for: preventing fork bombs, runaway processes\n\n");
}

/* ─── 4. Signals — comprehensive list ─── */
void show_signal_table(void) {
    printf("=== 4. UNIX Signals — Complete Reference ===\n");
    printf("%-5s %-12s %-10s %s\n","Num","Name","Default","Description");
    printf("%-5s %-12s %-10s %s\n","---","----","-------","-----------");

    struct { int sig; const char *name; const char *def; const char *desc; } sigs[] = {
        {1, "SIGHUP",  "Terminate", "Hangup / reload config"},
        {2, "SIGINT",  "Terminate", "Ctrl+C — keyboard interrupt"},
        {3, "SIGQUIT", "Core dump", "Ctrl+\\ — quit + core dump"},
        {4, "SIGILL",  "Core dump", "Illegal instruction"},
        {5, "SIGTRAP", "Core dump", "Debugger breakpoint"},
        {6, "SIGABRT", "Core dump", "abort() called"},
        {7, "SIGBUS",  "Core dump", "Bus error (bad memory access)"},
        {8, "SIGFPE",  "Core dump", "Floating point exception"},
        {9, "SIGKILL", "Terminate", "Unconditional kill (cannot be caught!)"},
        {10,"SIGUSR1", "Terminate", "User-defined signal 1"},
        {11,"SIGSEGV", "Core dump", "Segmentation fault (null ptr, etc.)"},
        {12,"SIGUSR2", "Terminate", "User-defined signal 2"},
        {13,"SIGPIPE", "Terminate", "Write to closed pipe"},
        {14,"SIGALRM", "Terminate", "Timer from alarm()"},
        {15,"SIGTERM", "Terminate", "Graceful termination (can be caught)"},
        {17,"SIGCHLD", "Ignore",    "Child stopped or terminated"},
        {18,"SIGCONT", "Continue",  "Continue stopped process"},
        {19,"SIGSTOP", "Stop",      "Stop process (cannot be caught!)"},
        {20,"SIGTSTP", "Stop",      "Ctrl+Z — terminal stop"},
        {28,"SIGWINCH","Ignore",    "Terminal window size changed"},
    };

    for (int i=0; i < (int)(sizeof(sigs)/sizeof(sigs[0])); i++) {
        printf("%-5d %-12s %-10s %s\n",
               sigs[i].sig, sigs[i].name, sigs[i].def, sigs[i].desc);
    }

    printf("\nSIGKILL (9) and SIGSTOP (19): CANNOT be caught, blocked, or ignored!\n");
    printf("kill -9 <pid>: always works (if you have permission)\n\n");
}

/* ─── 5. Custom signal handler ─── */
volatile sig_atomic_t reload_flag = 0;
volatile sig_atomic_t shutdown_flag = 0;

void signal_handler(int sig) {
    /* Signal-safe: only write, set atomic flags, etc. */
    if (sig == SIGHUP)  { reload_flag = 1; }
    if (sig == SIGTERM) { shutdown_flag = 1; }
}

void demo_signal_handler(void) {
    printf("=== 5. Signal Handler Demo ===\n");

    signal(SIGHUP,  signal_handler);
    signal(SIGTERM, signal_handler);

    printf("Sending SIGHUP to self (reload config signal)...\n");
    raise(SIGHUP);
    if (reload_flag) printf("  Caught SIGHUP → config reloaded!\n");

    printf("Sending SIGTERM to self (graceful shutdown)...\n");
    raise(SIGTERM);
    if (shutdown_flag) printf("  Caught SIGTERM → initiating shutdown...\n");

    /* Ignore SIGPIPE — important for network servers! */
    signal(SIGPIPE, SIG_IGN);
    printf("SIGPIPE ignored — write() to closed socket won't crash server\n\n");
}

/* ─── 6. System call via syscall() ─── */
void demo_direct_syscall(void) {
    printf("=== 6. Direct System Call (bypass libc) ===\n");
    /* getpid syscall directly, bypassing getpid() wrapper */
    long pid = syscall(SYS_getpid);
    printf("syscall(SYS_getpid) = %ld  (getpid()=%d)\n", pid, getpid());

    /* write() syscall directly */
    const char *msg = "Direct syscall write! (no printf buffering)\n";
    syscall(SYS_write, STDOUT_FILENO, msg, strlen(msg));

    printf("\n[How a system call works]\n");
    printf("1. User calls: write(fd, buf, n)\n");
    printf("2. libc sets: RAX=SYS_write, RDI=fd, RSI=buf, RDX=n\n");
    printf("3. CPU executes SYSCALL instruction → switches to ring 0\n");
    printf("4. Kernel: validates args, does work, sets RAX=result\n");
    printf("5. CPU executes SYSRET → back to ring 3\n");
    printf("6. libc checks RAX<0 → sets errno, returns -1\n");
    printf("Cost: ~100-1000ns (much cheaper than old int 0x80!)\n\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║    LINUX KERNEL CONCEPTS — Practical Deep Dive   ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    show_kernel_info();
    explore_proc();
    show_resource_limits();
    show_signal_table();
    demo_signal_handler();
    demo_direct_syscall();

    return 0;
}
