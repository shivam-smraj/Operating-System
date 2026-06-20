/* Phase01/02_kernel_vs_userspace.c
 * TOPIC: Kernel vs User Space — The fundamental boundary
 * Compile: gcc -Wall -o kernel_vs_user 02_kernel_vs_userspace.c
 *
 * This program demonstrates the user/kernel space boundary by:
 * 1. Showing that user-space code runs in Ring 3
 * 2. Demonstrating that OS services require crossing to Ring 0 (via syscall)
 * 3. Measuring the cost of a system call vs a regular function call
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>  /* SYS_write, SYS_getpid, etc. */
#include <string.h>

/* ─── A regular function call (entirely in user space, Ring 3) ─── */
static inline long user_space_add(long a, long b) {
    return a + b;   /* No kernel involvement whatsoever */
}

/* ─── Measure time using CLOCK_MONOTONIC ─── */
static long get_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000L + ts.tv_nsec;
}

int main(void) {
    printf("=== Kernel vs User Space Demo ===\n\n");

    /* ── Part 1: Demonstrate that normal code is user-space only ── */
    printf("[User Space] Performing 1,000,000 integer additions...\n");
    long start = get_ns();
    volatile long sum = 0;
    for (int i = 0; i < 1000000; i++)
        sum += user_space_add(i, i+1);  /* Pure user-space, no kernel */
    long user_time = get_ns() - start;
    printf("  Result: %ld\n", sum);
    printf("  Time: %ld ns (%.3f ns per add) — NO kernel involvement!\n\n",
           user_time, (double)user_time / 1000000);

    /* ── Part 2: System call performance ── */
    printf("[Kernel Space] Performing 10,000 getpid() system calls...\n");
    start = get_ns();
    volatile pid_t pid = 0;
    for (int i = 0; i < 10000; i++)
        pid = getpid();    /* Ring 3 → syscall instruction → Ring 0 → Ring 3 */
    long syscall_time = get_ns() - start;
    printf("  PID: %d\n", pid);
    printf("  Time: %ld ns (%.0f ns per syscall)\n\n",
           syscall_time, (double)syscall_time / 10000);

    printf("[ANALYSIS]\n");
    printf("  Avg user-space op : %.3f ns\n", (double)user_time / 1000000);
    printf("  Avg syscall cost  : %.0f ns\n", (double)syscall_time / 10000);
    printf("  Syscall overhead  : ~%.0fx slower than user-space op\n\n",
           ((double)syscall_time / 10000) / ((double)user_time / 1000000));

    /* ── Part 3: Raw system call without libc wrapper ── */
    printf("[Raw Syscall] Writing using raw syscall (no libc):\n");
    const char msg[] = "Hello from raw syscall(SYS_write)!\n";
    /* syscall(SYS_write, fd, buf, len) bypasses printf/libc entirely */
    syscall(SYS_write, STDOUT_FILENO, msg, strlen(msg));

    /* ── Part 4: What happens in the CPU during a system call ── */
    printf("\n[HOW SYSCALL WORKS — x86-64 Linux]:\n");
    printf("  1. User puts syscall number in %%rax (e.g., 1 for write)\n");
    printf("  2. User puts args in %%rdi, %%rsi, %%rdx, %%r10, %%r8, %%r9\n");
    printf("  3. SYSCALL instruction: CPU saves %%rip to %%rcx, %%rflags to %%r11\n");
    printf("  4. CPU atomically changes CPL (Current Privilege Level) 3→0\n");
    printf("  5. CPU jumps to address in MSR_LSTAR (kernel's entry point)\n");
    printf("  6. Kernel executes sys_write() (or whatever was requested)\n");
    printf("  7. SYSRET: CPU restores %%rip from %%rcx, CPL 0→3\n");
    printf("  8. User program continues from instruction after SYSCALL\n");

    return 0;
}
/* Expected output (times vary by machine):
 * === Kernel vs User Space Demo ===
 * [User Space] 1,000,000 additions...
 *   Time: 800000 ns (0.800 ns per add) — NO kernel involvement!
 * [Kernel Space] 10,000 getpid() system calls...
 *   Time: 2500000 ns (250 ns per syscall)
 * [ANALYSIS]
 *   Avg user-space op : 0.800 ns
 *   Avg syscall cost  : 250 ns
 *   Syscall overhead  : ~312x slower than user-space op
 */
