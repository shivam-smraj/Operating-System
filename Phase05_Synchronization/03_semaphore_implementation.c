/* Phase05/03_semaphore_implementation.c
 * Custom semaphore + POSIX semaphore + producer-consumer
 * Compile: gcc -Wall -pthread -o sem_impl 03_semaphore_implementation.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

/* ─── Custom semaphore built from mutex + condvar ─── */
typedef struct {
    int             value;
    pthread_mutex_t lock;
    pthread_cond_t  not_zero;
} MySem;

void mysem_init(MySem *s, int val) {
    s->value = val;
    pthread_mutex_init(&s->lock, NULL);
    pthread_cond_init(&s->not_zero, NULL);
}

/* P() — decrement, block if zero */
void mysem_wait(MySem *s) {
    pthread_mutex_lock(&s->lock);
    while (s->value <= 0)
        pthread_cond_wait(&s->not_zero, &s->lock);
    s->value--;
    pthread_mutex_unlock(&s->lock);
}

/* V() — increment, wake one waiter */
void mysem_post(MySem *s) {
    pthread_mutex_lock(&s->lock);
    s->value++;
    pthread_cond_signal(&s->not_zero);
    pthread_mutex_unlock(&s->lock);
}

int mysem_getvalue(MySem *s) {
    pthread_mutex_lock(&s->lock);
    int v = s->value;
    pthread_mutex_unlock(&s->lock);
    return v;
}

/* ─── Bounded Producer-Consumer with custom semaphores ─── */
#define BUF_SIZE  5
int buf[BUF_SIZE];
int in_pos=0, out_pos=0;
MySem sem_empty, sem_full;
pthread_mutex_t buf_lock = PTHREAD_MUTEX_INITIALIZER;

void *producer(void *arg) {
    int id = *(int*)arg;
    for (int item = id*10 + 1; item <= id*10 + 5; item++) {
        mysem_wait(&sem_empty);           /* Wait for empty slot */
        pthread_mutex_lock(&buf_lock);
        buf[in_pos] = item;
        printf("[P%d] PRODUCE %2d → buf[%d] (full_slots=%d)\n",
               id, item, in_pos, mysem_getvalue(&sem_full)+1);
        in_pos = (in_pos + 1) % BUF_SIZE;
        pthread_mutex_unlock(&buf_lock);
        mysem_post(&sem_full);            /* Signal: item ready */
        usleep(80000);
    }
    return NULL;
}

void *consumer(void *arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 5; i++) {
        mysem_wait(&sem_full);            /* Wait for item */
        pthread_mutex_lock(&buf_lock);
        int item = buf[out_pos];
        printf("      [C%d] CONSUME %2d ← buf[%d] (empty_slots=%d)\n",
               id, item, out_pos, mysem_getvalue(&sem_empty)+1);
        out_pos = (out_pos + 1) % BUF_SIZE;
        pthread_mutex_unlock(&buf_lock);
        mysem_post(&sem_empty);           /* Signal: slot freed */
        usleep(150000);
    }
    return NULL;
}

void demo_custom_semaphore(void) {
    printf("=== Custom Semaphore (built from mutex+condvar) ===\n");
    printf("Buffer size: %d  |  2 producers  |  2 consumers\n\n", BUF_SIZE);

    mysem_init(&sem_empty, BUF_SIZE);  /* BUF_SIZE empty slots */
    mysem_init(&sem_full,  0);         /* 0 full slots */

    pthread_t p[2], c[2];
    int pids[2]={1,2}, cids[2]={1,2};
    pthread_create(&p[0],NULL,producer,&pids[0]);
    pthread_create(&p[1],NULL,producer,&pids[1]);
    pthread_create(&c[0],NULL,consumer,&cids[0]);
    pthread_create(&c[1],NULL,consumer,&cids[1]);

    for(int i=0;i<2;i++){pthread_join(p[i],NULL);pthread_join(c[i],NULL);}
    printf("\nAll done. Buffer empty: %s\n",
           mysem_getvalue(&sem_full)==0 ? "YES" : "NO");
}

/* ─── POSIX named semaphore demo ─── */
void demo_posix_semaphore(void) {
    printf("\n=== POSIX Unnamed Semaphore ===\n");
    sem_t s;
    sem_init(&s, 0, 3);  /* pshared=0 (threads), initial=3 */

    int val;
    sem_getvalue(&s, &val);
    printf("Initial value: %d\n", val);

    sem_wait(&s); sem_getvalue(&s,&val); printf("After wait(): %d\n", val);
    sem_wait(&s); sem_getvalue(&s,&val); printf("After wait(): %d\n", val);
    sem_wait(&s); sem_getvalue(&s,&val); printf("After wait(): %d\n", val);
    /* sem_wait(&s) now would BLOCK — value is 0 */

    printf("Trying trywait() when value=0: ");
    if (sem_trywait(&s) == -1)
        printf("WOULD_BLOCK (returns EAGAIN) — non-blocking check\n");

    sem_post(&s); sem_getvalue(&s,&val); printf("After post(): %d\n", val);
    sem_destroy(&s);

    printf("\n[Semaphore vs Mutex]\n");
    printf("Semaphore:\n");
    printf("  - Integer value (0..N)\n");
    printf("  - NO ownership — any thread can post()\n");
    printf("  - Used for: signaling, resource counting\n");
    printf("  - Binary semaphore (init=1) ≈ mutex but no ownership!\n");
    printf("Mutex:\n");
    printf("  - Binary (locked/unlocked)\n");
    printf("  - Ownership — ONLY locker can unlock\n");
    printf("  - Supports priority inheritance (futex)\n");
    printf("  - Used for: protecting critical sections\n");
}

int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   SEMAPHORE IMPLEMENTATION + POSIX SEMAPHORE   ║\n");
    printf("╚═══════════════════════════════════════════════╝\n\n");
    demo_custom_semaphore();
    demo_posix_semaphore();
    return 0;
}
