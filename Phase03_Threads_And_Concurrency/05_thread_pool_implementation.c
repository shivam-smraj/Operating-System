/* Phase03/05_thread_pool_implementation.c
 * Full thread pool with task queue and worker threads
 * Compile: gcc -Wall -pthread -o thread_pool 05_thread_pool_implementation.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_QUEUE_SIZE 100
#define NUM_WORKERS    4

typedef void (*task_fn)(void *);
typedef struct { task_fn fn; void *arg; } Task;

typedef struct {
    Task            queue[MAX_QUEUE_SIZE];
    int             head, tail, count, shutdown;
    pthread_mutex_t lock;
    pthread_cond_t  not_empty, not_full;
    pthread_t       workers[NUM_WORKERS];
} ThreadPool;

void *worker(void *arg) {
    ThreadPool *pool = (ThreadPool*)arg;
    while (1) {
        pthread_mutex_lock(&pool->lock);
        while (pool->count == 0 && !pool->shutdown)
            pthread_cond_wait(&pool->not_empty, &pool->lock);
        if (pool->shutdown && pool->count == 0) {
            pthread_mutex_unlock(&pool->lock);
            return NULL;
        }
        Task t = pool->queue[pool->head];
        pool->head = (pool->head + 1) % MAX_QUEUE_SIZE;
        pool->count--;
        pthread_cond_signal(&pool->not_full);
        pthread_mutex_unlock(&pool->lock);
        t.fn(t.arg);
    }
}

ThreadPool *pool_create(void) {
    ThreadPool *p = calloc(1, sizeof(ThreadPool));
    pthread_mutex_init(&p->lock, NULL);
    pthread_cond_init(&p->not_empty, NULL);
    pthread_cond_init(&p->not_full, NULL);
    for (int i = 0; i < NUM_WORKERS; i++)
        pthread_create(&p->workers[i], NULL, worker, p);
    return p;
}

void pool_submit(ThreadPool *p, task_fn fn, void *arg) {
    pthread_mutex_lock(&p->lock);
    while (p->count == MAX_QUEUE_SIZE)
        pthread_cond_wait(&p->not_full, &p->lock);
    p->queue[p->tail] = (Task){fn, arg};
    p->tail = (p->tail + 1) % MAX_QUEUE_SIZE;
    p->count++;
    pthread_cond_signal(&p->not_empty);
    pthread_mutex_unlock(&p->lock);
}

void pool_shutdown(ThreadPool *p) {
    pthread_mutex_lock(&p->lock);
    p->shutdown = 1;
    pthread_cond_broadcast(&p->not_empty);
    pthread_mutex_unlock(&p->lock);
    for (int i = 0; i < NUM_WORKERS; i++)
        pthread_join(p->workers[i], NULL);
    free(p);
}

void compute_task(void *arg) {
    int n = *(int*)arg;
    long sum = 0;
    for (int i = 0; i <= n; i++) sum += i;
    printf("[Task] sum(1..%d) = %ld (thread %lu)\n",
           n, sum, pthread_self() % 10000);
    free(arg);
}

int main(void) {
    printf("=== Thread Pool Demo ===\n");
    printf("Pool: %d workers, queue size: %d\n\n", NUM_WORKERS, MAX_QUEUE_SIZE);
    ThreadPool *pool = pool_create();
    for (int i = 1; i <= 12; i++) {
        int *n = malloc(sizeof(int));
        *n = i * 100;
        pool_submit(pool, compute_task, n);
    }
    sleep(1);
    pool_shutdown(pool);
    printf("All workers done.\n");
    return 0;
}
