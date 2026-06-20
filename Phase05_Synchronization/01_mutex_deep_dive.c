/* Phase05/01_mutex_deep_dive.c
 * TOPIC: Mutex internals — futex, spinlock, priority inheritance
 * Compile: gcc -Wall -pthread -o mutex_deep 01_mutex_deep_dive.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

/* ─── Timing utility ─── */
static long get_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000L + ts.tv_nsec;
}

/* ─── Demo 1: Mutex types ─── */
void demo_mutex_types(void) {
    printf("=== Mutex Types ===\n\n");

    /* 1. Default (PTHREAD_MUTEX_DEFAULT) */
    pthread_mutex_t m_default = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&m_default);
    /* pthread_mutex_lock(&m_default);  ← DEADLOCK! Default mutex not recursive */
    pthread_mutex_unlock(&m_default);
    printf("DEFAULT mutex: fast, undefined behavior on double-lock\n");

    /* 2. Error-checking mutex */
    pthread_mutex_t m_errcheck;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&m_errcheck, &attr);

    pthread_mutex_lock(&m_errcheck);
    int ret = pthread_mutex_lock(&m_errcheck);  /* Try to double-lock */
    printf("ERRORCHECK mutex: double-lock returns: %s (EDEADLK=%d)\n",
           strerror(ret), EDEADLK);
    pthread_mutex_unlock(&m_errcheck);
    pthread_mutex_destroy(&m_errcheck);

    /* 3. Recursive mutex */
    pthread_mutex_t m_rec;
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_rec, &attr);

    pthread_mutex_lock(&m_rec);
    pthread_mutex_lock(&m_rec);   /* OK! Recursive allows same thread to re-lock */
    pthread_mutex_lock(&m_rec);
    printf("RECURSIVE mutex: locked 3 times by same thread — allowed!\n");
    pthread_mutex_unlock(&m_rec);
    pthread_mutex_unlock(&m_rec);
    pthread_mutex_unlock(&m_rec);  /* Must unlock same number of times */
    pthread_mutex_destroy(&m_rec);
    pthread_mutexattr_destroy(&attr);
}

/* ─── Demo 2: Lock contention timing ─── */
pthread_mutex_t bench_mtx = PTHREAD_MUTEX_INITIALIZER;
volatile long shared_counter = 0;

void *contention_thread(void *arg) {
    int iters = *(int*)arg;
    for (int i = 0; i < iters; i++) {
        pthread_mutex_lock(&bench_mtx);
        shared_counter++;
        pthread_mutex_unlock(&bench_mtx);
    }
    return NULL;
}

void demo_mutex_performance(void) {
    printf("\n=== Mutex Performance (contention benchmark) ===\n");
    int iters = 500000;

    /* 1 thread — no contention */
    shared_counter = 0;
    long t0 = get_ns();
    pthread_t t;
    pthread_create(&t, NULL, contention_thread, &iters);
    pthread_join(t, NULL);
    long t1 = get_ns();
    printf("1 thread  (no contention):    %ldms  (%ld ns/lock)\n",
           (t1-t0)/1000000, (t1-t0)/iters);

    /* 4 threads — high contention */
    shared_counter = 0;
    t0 = get_ns();
    pthread_t ts[4];
    for (int i=0;i<4;i++) pthread_create(&ts[i],NULL,contention_thread,&iters);
    for (int i=0;i<4;i++) pthread_join(ts[i],NULL);
    t1 = get_ns();
    printf("4 threads (high contention):  %ldms  (%ld ns/lock per thread)\n",
           (t1-t0)/1000000, (t1-t0)/(4*iters));
    printf("Counter: %ld (expected %d)\n\n", shared_counter, 4*iters);
    printf("Observation: Contention makes mutex ~5-10x slower!\n");
    printf("Solution: Reduce critical section size, use lock-free structures.\n");
}

/* ─── Demo 3: trylock — non-blocking lock attempt ─── */
pthread_mutex_t try_mtx = PTHREAD_MUTEX_INITIALIZER;

void *try_lock_thread(void *arg) {
    int id = *(int*)arg;
    /* Try to acquire lock without blocking */
    int ret = pthread_mutex_trylock(&try_mtx);
    if (ret == 0) {
        printf("Thread %d: GOT the lock (trylock succeeded)\n", id);
        usleep(200000);  /* Hold it for 200ms */
        pthread_mutex_unlock(&try_mtx);
    } else if (ret == EBUSY) {
        printf("Thread %d: BUSY — lock held by another thread (trylock failed)\n", id);
        printf("Thread %d: doing alternative work instead of blocking\n", id);
    }
    return NULL;
}

void demo_trylock(void) {
    printf("\n=== trylock — Non-blocking Mutex Acquisition ===\n");
    pthread_mutex_lock(&try_mtx);  /* Main holds the lock */

    pthread_t t1, t2;
    int ids[] = {1, 2};
    pthread_create(&t1, NULL, try_lock_thread, &ids[0]);
    usleep(50000);
    pthread_create(&t2, NULL, try_lock_thread, &ids[1]);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_unlock(&try_mtx);

    printf("\ntrylock use case: work-stealing thread pools\n");
    printf("  Worker checks: is this task available? If not, move to next.\n");
}

/* ─── Demo 4: Read-Write Lock ─── */
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
int shared_data = 0;

void *reader(void *arg) {
    int id = *(int*)arg;
    pthread_rwlock_rdlock(&rwlock);  /* Multiple readers can hold simultaneously */
    printf("Reader %d: reading shared_data=%d (concurrent with other readers!)\n",
           id, shared_data);
    usleep(100000);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void *writer(void *arg) {
    int id = *(int*)arg;
    pthread_rwlock_wrlock(&rwlock);  /* Exclusive — blocks all readers/writers */
    shared_data++;
    printf("Writer %d: updated shared_data=%d (exclusive access)\n", id, shared_data);
    usleep(50000);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void demo_rwlock(void) {
    printf("\n=== Read-Write Lock (Multiple Readers / Single Writer) ===\n");

    /* Launch 3 readers simultaneously */
    pthread_t rt[3], wt;
    int rids[3] = {1,2,3}, wid = 1;

    printf("Starting 3 readers simultaneously:\n");
    for(int i=0;i<3;i++) pthread_create(&rt[i], NULL, reader, &rids[i]);
    for(int i=0;i<3;i++) pthread_join(rt[i], NULL);

    printf("\nNow a writer (exclusive):\n");
    pthread_create(&wt, NULL, writer, &wid); pthread_join(wt, NULL);

    printf("\n[rwlock vs mutex]\n");
    printf("Use rwlock when: reads >> writes (read-mostly shared data)\n");
    printf("Use mutex when:  writes are frequent or reads are very short\n");
    printf("rwlock overhead > mutex — only worth it for long reads!\n");
}

int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║      MUTEX DEEP DIVE — Types, Performance      ║\n");
    printf("╚═══════════════════════════════════════════════╝\n\n");

    demo_mutex_types();
    demo_mutex_performance();
    demo_trylock();
    demo_rwlock();

    printf("\n[MUTEX INTERNALS — futex implementation]\n");
    printf("Uncontested path (fast path):\n");
    printf("  1. CAS(lock_word, 0, 1) succeeds → lock acquired!\n");
    printf("  2. No system call. Cost: ~5-20ns\n\n");
    printf("Contested path (slow path):\n");
    printf("  1. CAS fails → lock held by another thread\n");
    printf("  2. Set lock_word = 2 (held + waiters)\n");
    printf("  3. Call futex(FUTEX_WAIT) → thread sleeps in kernel\n");
    printf("  4. On unlock: futex(FUTEX_WAKE) wakes one waiter\n");
    printf("  5. Cost: ~1-10 microseconds (system call overhead)\n");
    return 0;
}
