/* Phase03/01_pthread_basics.c
 * TOPIC: POSIX Threads — Creation, Joining, Attributes
 * Compile: gcc -Wall -pthread -o pthread_basics 01_pthread_basics.c
 *
 * pthreads is the standard threading API on Linux/macOS.
 * These are the core functions every programmer must know.
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* ─── Thread function signature: void* fn(void*) ─── */
void *worker_thread(void *arg) {
    int thread_num = *(int*)arg;
    printf("Thread %d: started, TID=%lu\n",
           thread_num, (unsigned long)pthread_self() % 100000);

    /* Simulate work */
    usleep(thread_num * 100000);  /* 100ms × thread_num */
    printf("Thread %d: finished work\n", thread_num);

    /* Return value via heap (IMPORTANT: don't return stack address!) */
    int *result = malloc(sizeof(int));
    *result = thread_num * thread_num;  /* Return thread_num squared */
    return (void*)result;
}

void demo_basic_threads(void) {
    printf("\n=== Demo 1: Basic Thread Creation and Joining ===\n");
    #define N_THREADS 4

    pthread_t tids[N_THREADS];
    int       args[N_THREADS];

    /* Create N threads */
    for (int i = 0; i < N_THREADS; i++) {
        args[i] = i + 1;
        int ret = pthread_create(&tids[i], NULL, worker_thread, &args[i]);
        if (ret != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(ret));
            exit(1);
        }
        printf("Main: created thread %d (TID=%lu)\n",
               i+1, (unsigned long)tids[i] % 100000);
    }

    /* Join (wait for) each thread and collect return value */
    for (int i = 0; i < N_THREADS; i++) {
        void *retval;
        pthread_join(tids[i], &retval);  /* Block until thread finishes */
        int result = *(int*)retval;
        printf("Main: thread %d returned %d (=%d²)\n", i+1, result, i+1);
        free(retval);  /* We malloc'd it in thread, free it here */
    }
    printf("All threads joined.\n");
}

void demo_detached_thread(void) {
    printf("\n=== Demo 2: Detached Thread (fire and forget) ===\n");

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    /* Detached: auto-cleaned up when done, can't be joined */

    int arg = 99;
    pthread_t tid;
    pthread_create(&tid, &attr, worker_thread, &arg);
    pthread_attr_destroy(&attr);

    printf("Detached thread launched — main won't wait for it.\n");
    printf("Detached threads are used for background tasks (log writer, etc.)\n");
    sleep(1);  /* Give detached thread time to run */
}

void demo_thread_attributes(void) {
    printf("\n=== Demo 3: Thread Attributes ===\n");

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    /* Stack size */
    size_t stacksize;
    pthread_attr_getstacksize(&attr, &stacksize);
    printf("Default stack size: %zu KB\n", stacksize / 1024);

    /* Set custom stack size: 1MB instead of default 8MB */
    pthread_attr_setstacksize(&attr, 1024 * 1024);
    pthread_attr_getstacksize(&attr, &stacksize);
    printf("Custom stack size:  %zu KB\n", stacksize / 1024);

    /* Scheduling policy */
    int policy;
    pthread_attr_getschedpolicy(&attr, &policy);
    printf("Scheduling policy: %s\n",
           policy == SCHED_OTHER ? "SCHED_OTHER (CFS)" :
           policy == SCHED_FIFO  ? "SCHED_FIFO (RT)"  :
           policy == SCHED_RR    ? "SCHED_RR (RT)"    : "unknown");

    pthread_attr_destroy(&attr);
}

void demo_thread_local_storage(void) {
    printf("\n=== Demo 4: Thread-Local Storage ===\n");

    /* __thread: each thread gets its OWN copy of this variable */
    /* Used for errno, locale settings, per-thread caches */
    __thread int per_thread_counter = 0;

    per_thread_counter = 42;
    printf("Main thread: per_thread_counter = %d\n", per_thread_counter);
    /* Another thread would see its own counter starting at 0 */

    printf("TLS is how errno works:\n");
    printf("  errno is __thread int errno — each thread has its own!\n");
    printf("  errno = %d (this thread's errno)\n", errno);
}

int main(void) {
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║   POSIX Threads (pthreads) — Deep Dive    ║\n");
    printf("╚═══════════════════════════════════════════╝\n");

    demo_basic_threads();
    demo_detached_thread();
    demo_thread_attributes();
    demo_thread_local_storage();

    printf("\n[THREAD vs PROCESS — Memory View]\n");
    printf("Shared between threads of same process:\n");
    printf("  ✓ Code segment (text)\n");
    printf("  ✓ Global variables, static variables\n");
    printf("  ✓ Heap (malloc'd memory)\n");
    printf("  ✓ File descriptors\n");
    printf("  ✓ Signal handlers, signal dispositions\n");
    printf("Private per thread:\n");
    printf("  • Stack (local variables, function call frames)\n");
    printf("  • CPU registers (saved/restored on context switch)\n");
    printf("  • Thread ID (pthread_t)\n");
    printf("  • errno (thread-local storage)\n");
    printf("  • Signal mask (which signals are blocked)\n");
    printf("  • Thread-local storage (__thread variables)\n");
    return 0;
}
