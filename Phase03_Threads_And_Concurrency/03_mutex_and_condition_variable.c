/* Phase03/03_mutex_and_condition_variable.c
 * TOPIC: Mutex + Condition Variable — the synchronization pair
 * Compile: gcc -Wall -pthread -o mutex_cond 03_mutex_and_condition_variable.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* ─── Shared state ─── */
pthread_mutex_t mutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  ready  = PTHREAD_COND_INITIALIZER;  /* "data is ready" */
pthread_cond_t  space  = PTHREAD_COND_INITIALIZER;  /* "space available" */

int buffer[5];
int count = 0;          /* Items currently in buffer */
int total_produced = 0;
int total_consumed = 0;
int done = 0;           /* Shutdown flag */

#define ITEMS_TO_PRODUCE 15

void *producer(void *arg) {
    for (int i = 1; i <= ITEMS_TO_PRODUCE; i++) {
        pthread_mutex_lock(&mutex);

        /* Wait while buffer is full */
        while (count == 5 && !done)
            pthread_cond_wait(&space, &mutex);  /* Atomically: unlock + sleep */

        /* Add item to buffer (circular) */
        buffer[total_produced % 5] = i;
        count++;
        total_produced++;
        printf("[Producer] PUT  item %2d  (buffer count: %d)\n", i, count);

        pthread_cond_signal(&ready);   /* Signal: "item available!" */
        pthread_mutex_unlock(&mutex);

        usleep(80000);  /* Simulate production time */
    }

    /* Signal consumers to exit */
    pthread_mutex_lock(&mutex);
    done = 1;
    pthread_cond_broadcast(&ready);  /* Wake ALL waiting consumers */
    pthread_mutex_unlock(&mutex);

    printf("[Producer] Done producing %d items.\n", ITEMS_TO_PRODUCE);
    return NULL;
}

void *consumer(void *arg) {
    int id = *(int*)arg;
    while (1) {
        pthread_mutex_lock(&mutex);

        /* Wait while buffer is empty (and not done) */
        while (count == 0 && !done)
            pthread_cond_wait(&ready, &mutex);  /* Atomically: unlock + sleep */

        if (count == 0 && done) {
            pthread_mutex_unlock(&mutex);
            break;  /* No more items coming, exit */
        }

        /* Remove item from buffer */
        int item = buffer[total_consumed % 5];
        count--;
        total_consumed++;
        printf("[Consumer%d] GET  item %2d  (buffer count: %d)\n", id, item, count);

        pthread_cond_signal(&space);   /* Signal: "space available!" */
        pthread_mutex_unlock(&mutex);

        usleep(150000);  /* Simulate consumption time (slower than producer) */
    }
    printf("[Consumer%d] Exiting.\n", id);
    return NULL;
}

void demo_producer_consumer(void) {
    printf("=== Producer-Consumer with Mutex + Condition Variables ===\n");
    printf("Buffer size: 5  |  Producer: 1  |  Consumers: 2\n");
    printf("Items to produce: %d\n\n", ITEMS_TO_PRODUCE);

    pthread_t prod_tid;
    pthread_t cons_tids[2];
    int ids[2] = {1, 2};

    pthread_create(&prod_tid,    NULL, producer, NULL);
    pthread_create(&cons_tids[0], NULL, consumer, &ids[0]);
    pthread_create(&cons_tids[1], NULL, consumer, &ids[1]);

    pthread_join(prod_tid,    NULL);
    pthread_join(cons_tids[0], NULL);
    pthread_join(cons_tids[1], NULL);

    printf("\nProduced: %d  |  Consumed: %d\n", total_produced, total_consumed);
    printf("Buffer remaining: %d (should be 0)\n", count);
}

/* ─── Barrier: wait for N threads to reach a point ─── */
typedef struct {
    int             count;
    int             total;
    pthread_mutex_t lock;
    pthread_cond_t  all_arrived;
} Barrier;

void barrier_init(Barrier *b, int n) {
    b->count = 0; b->total = n;
    pthread_mutex_init(&b->lock, NULL);
    pthread_cond_init(&b->all_arrived, NULL);
}

void barrier_wait(Barrier *b, int tid) {
    pthread_mutex_lock(&b->lock);
    b->count++;
    printf("  Thread %d arrived at barrier (%d/%d)\n", tid, b->count, b->total);
    if (b->count < b->total) {
        pthread_cond_wait(&b->all_arrived, &b->lock);  /* Wait for others */
    } else {
        b->count = 0;  /* Reset for reuse */
        pthread_cond_broadcast(&b->all_arrived);         /* Wake all */
        printf("  >>> All threads arrived! Barrier released <<<\n");
    }
    pthread_mutex_unlock(&b->lock);
}

Barrier bar;

void *barrier_thread(void *arg) {
    int id = *(int*)arg;
    printf("Thread %d: doing phase 1 work...\n", id);
    usleep((id+1) * 50000);
    barrier_wait(&bar, id);
    printf("Thread %d: doing phase 2 work...\n", id);
    return NULL;
}

void demo_barrier(void) {
    printf("\n=== Barrier Synchronization ===\n");
    printf("4 threads must all complete phase 1 before any starts phase 2\n\n");

    barrier_init(&bar, 4);
    pthread_t t[4];
    int ids[4] = {0,1,2,3};
    for (int i=0; i<4; i++)
        pthread_create(&t[i], NULL, barrier_thread, &ids[i]);
    for (int i=0; i<4; i++)
        pthread_join(t[i], NULL);
    printf("\nAll threads completed both phases.\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  Mutex + Condition Variables — Complete Demo  ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    demo_producer_consumer();
    demo_barrier();

    printf("\n[KEY RULES]\n");
    printf("1. Always use WHILE loop for cond_wait (not if!) — spurious wakeups!\n");
    printf("2. cond_wait atomically: releases mutex + sleeps\n");
    printf("3. On wakeup: cond_wait re-acquires mutex before returning\n");
    printf("4. signal() wakes ONE waiter; broadcast() wakes ALL waiters\n");
    printf("5. Condition variable ALWAYS paired with a mutex\n");
    return 0;
}
