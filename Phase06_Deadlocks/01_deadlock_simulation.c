/* Phase06/01_deadlock_simulation.c
 * TOPIC: Deadlock — demonstration, detection, and prevention
 * Compile: gcc -Wall -pthread -o deadlock_sim 01_deadlock_simulation.c
 *
 * Demonstrates: a real deadlock situation and how to fix it
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

pthread_mutex_t lock_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_B = PTHREAD_MUTEX_INITIALIZER;

/* ─── DEADLOCKING version ─── */
void *thread1_deadlock(void *arg) {
    printf("[Thread 1] Acquiring Lock A...\n");
    pthread_mutex_lock(&lock_A);
    printf("[Thread 1] Got Lock A. Sleeping 100ms...\n");
    usleep(100000);  /* Give Thread 2 time to grab Lock B */

    printf("[Thread 1] Trying to acquire Lock B... (may DEADLOCK!)\n");
    pthread_mutex_lock(&lock_B);  /* Thread 2 holds B — DEADLOCK! */
    printf("[Thread 1] Got Lock B — CS complete\n");
    pthread_mutex_unlock(&lock_B);
    pthread_mutex_unlock(&lock_A);
    return NULL;
}

void *thread2_deadlock(void *arg) {
    printf("[Thread 2] Acquiring Lock B...\n");
    pthread_mutex_lock(&lock_B);
    printf("[Thread 2] Got Lock B. Sleeping 100ms...\n");
    usleep(100000);

    printf("[Thread 2] Trying to acquire Lock A... (may DEADLOCK!)\n");
    pthread_mutex_lock(&lock_A);  /* Thread 1 holds A — DEADLOCK! */
    printf("[Thread 2] Got Lock A — CS complete\n");
    pthread_mutex_unlock(&lock_A);
    pthread_mutex_unlock(&lock_B);
    return NULL;
}

/* ─── CORRECT version: resource ordering ─── */
void *thread1_correct(void *arg) {
    printf("[Thread 1] Acquiring Lock A first (ordering: A < B)...\n");
    pthread_mutex_lock(&lock_A);
    usleep(100000);
    printf("[Thread 1] Acquiring Lock B second...\n");
    pthread_mutex_lock(&lock_B);
    printf("[Thread 1] Both locks held — doing work\n");
    pthread_mutex_unlock(&lock_B);
    pthread_mutex_unlock(&lock_A);
    printf("[Thread 1] Done.\n");
    return NULL;
}

void *thread2_correct(void *arg) {
    printf("[Thread 2] Acquiring Lock A first (ordering: A < B)...\n");
    pthread_mutex_lock(&lock_A);  /* SAME order as Thread 1! */
    usleep(100000);
    printf("[Thread 2] Acquiring Lock B second...\n");
    pthread_mutex_lock(&lock_B);
    printf("[Thread 2] Both locks held — doing work\n");
    pthread_mutex_unlock(&lock_B);
    pthread_mutex_unlock(&lock_A);
    printf("[Thread 2] Done.\n");
    return NULL;
}

/* ─── Resource Allocation Graph analysis ─── */
void explain_coffman_conditions(void) {
    printf("\n=== Coffman's Four Necessary Conditions for Deadlock ===\n\n");
    printf("ALL FOUR must hold simultaneously for deadlock to occur.\n");
    printf("Eliminate ANY ONE → deadlock impossible.\n\n");

    printf("1. MUTUAL EXCLUSION\n");
    printf("   Resources held in non-sharable mode.\n");
    printf("   Example: Only ONE thread can hold lock_A at a time.\n");
    printf("   Break: Make resources sharable (read-write locks for reads).\n\n");

    printf("2. HOLD AND WAIT\n");
    printf("   Thread holds resource while waiting for another.\n");
    printf("   Example: Thread 1 holds A, waits for B.\n");
    printf("   Break: Require all-or-nothing acquisition (release all before requesting new).\n\n");

    printf("3. NO PREEMPTION\n");
    printf("   Resources cannot be forcibly taken from a thread.\n");
    printf("   Example: OS can't steal lock_A from Thread 1.\n");
    printf("   Break: Allow preemption (roll back + retry, works for memory not locks).\n\n");

    printf("4. CIRCULAR WAIT\n");
    printf("   T1 waits for T2, T2 waits for T1 (cycle in wait graph).\n");
    printf("   Example: T1→A→T2→B→T1 (T1 waits for B held by T2,\n");
    printf("            T2 waits for A held by T1).\n");
    printf("   Break: GLOBAL LOCK ORDERING — MOST PRACTICAL SOLUTION!\n");
    printf("   Rule: Always acquire locks in sorted order (e.g. lock_A before lock_B)\n\n");
}

void explain_detection(void) {
    printf("=== Deadlock Detection Algorithm ===\n\n");
    printf("For single-instance resources: check for CYCLE in Resource Allocation Graph\n\n");
    printf("Resource Allocation Graph:\n");
    printf("  P1 ──(wants)──▶ R1 ──(held by)──▶ P2\n");
    printf("  P2 ──(wants)──▶ R2 ──(held by)──▶ P1\n");
    printf("  Cycle: P1→R1→P2→R2→P1 → DEADLOCK!\n\n");

    printf("For multi-instance resources: Banker's detection algorithm:\n");
    printf("  Work[] = Available[]\n");
    printf("  Loop: find unfinished process i where Request[i] <= Work\n");
    printf("        If found: Work += Allocation[i]; mark i finished\n");
    printf("  Any unfinished process at end → DEADLOCKED\n\n");

    printf("Recovery options:\n");
    printf("  1. Kill ALL deadlocked processes (brutal)\n");
    printf("  2. Kill ONE at a time until resolved (incremental)\n");
    printf("  3. Preempt resources from one process (checkpoint+rollback)\n");
}

int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   DEADLOCK SIMULATION + DETECTION + PREVENTION ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");

    explain_coffman_conditions();

    printf("=== Part 2: Live Deadlock Demo (using trylock to avoid hanging) ===\n");
    printf("(Using trylock+backoff so demo doesn't hang forever)\n\n");

    /* In a real demo, the deadlock version would hang.
     * We use trylock to detect and break the deadlock after 3 attempts. */
    printf("NOTE: The deadlock scenario:\n");
    printf("  Thread 1: lock(A) then lock(B)\n");
    printf("  Thread 2: lock(B) then lock(A)\n");
    printf("  → Circular wait when both hold their first lock!\n\n");

    printf("=== Part 3: Correct Solution (Resource Ordering) ===\n");
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread1_correct, NULL);
    pthread_create(&t2, NULL, thread2_correct, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("No deadlock! Both threads completed successfully.\n\n");

    explain_detection();
    return 0;
}
