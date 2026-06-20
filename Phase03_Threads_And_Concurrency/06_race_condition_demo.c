/* Phase03/06_race_condition_demo.c
 * Demonstrates race condition and the fix with mutex
 * Compile: gcc -Wall -pthread -o race 06_race_condition_demo.c
 */
#include <stdio.h>
#include <pthread.h>
#define THREADS 4
#define ITERS   500000

long counter_unsafe = 0;
long counter_safe   = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void *unsafe_increment(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERS; i++)
        counter_unsafe++;  /* RACE CONDITION! */
    return NULL;
}

void *safe_increment(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERS; i++) {
        pthread_mutex_lock(&mtx);
        counter_safe++;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

void run_test(const char *label, void *(*fn)(void*), long *counter) {
    *counter = 0;
    pthread_t t[THREADS];
    for (int i = 0; i < THREADS; i++) pthread_create(&t[i], NULL, fn, NULL);
    for (int i = 0; i < THREADS; i++) pthread_join(t[i], NULL);
    long expected = (long)THREADS * ITERS;
    printf("%-22s: got %8ld, expected %8ld — %s\n",
           label, *counter, expected,
           *counter == expected ? "CORRECT" : "WRONG (race!)");
}

int main(void) {
    printf("=== Race Condition Demo ===\n");
    printf("%d threads x %d iterations = %ld expected\n\n",
           THREADS, ITERS, (long)THREADS*ITERS);
    run_test("Unsafe (no mutex)", unsafe_increment, &counter_unsafe);
    run_test("Safe (with mutex)", safe_increment, &counter_safe);
    printf("\nRun multiple times to see unsafe result vary!\n");
    return 0;
}
