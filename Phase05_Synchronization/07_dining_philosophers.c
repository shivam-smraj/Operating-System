/* Phase05/07_dining_philosophers.c
 * Dining Philosophers — correct solution via resource ordering
 * Compile: gcc -Wall -pthread -o dining 07_dining_philosophers.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 5
pthread_mutex_t chopstick[N];
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void plog(int id, const char *msg) {
    pthread_mutex_lock(&print_lock);
    printf("Philosopher %d: %s\n", id, msg);
    pthread_mutex_unlock(&print_lock);
}

/* CORRECT SOLUTION: resource ordering — always pick lower-numbered chopstick first */
void *philosopher(void *arg) {
    int id    = *(int*)arg;
    int left  = id;
    int right = (id + 1) % N;
    int first  = (left < right) ? left : right;
    int second = (left < right) ? right : left;

    for (int meal = 0; meal < 3; meal++) {
        plog(id, "thinking...");
        usleep((rand() % 200 + 100) * 1000);

        plog(id, "hungry — waiting for lower chopstick");
        pthread_mutex_lock(&chopstick[first]);
        usleep(10000);
        pthread_mutex_lock(&chopstick[second]);

        plog(id, "EATING!");
        usleep((rand() % 300 + 100) * 1000);

        pthread_mutex_unlock(&chopstick[second]);
        pthread_mutex_unlock(&chopstick[first]);
        plog(id, "done eating");
    }
    return NULL;
}

int main(void) {
    printf("=== Dining Philosophers — Resource Ordering Solution ===\n");
    printf("Rule: Always acquire lower-numbered chopstick first.\n");
    printf("This breaks circular wait → no deadlock!\n\n");

    for (int i = 0; i < N; i++)
        pthread_mutex_init(&chopstick[i], NULL);

    pthread_t t[N];
    int ids[N];
    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&t[i], NULL, philosopher, &ids[i]);
    }
    for (int i = 0; i < N; i++) pthread_join(t[i], NULL);

    printf("\nAll %d philosophers ate 3 times each — no deadlock!\n", N);
    return 0;
}
