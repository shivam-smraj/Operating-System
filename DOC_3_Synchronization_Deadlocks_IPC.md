# DOC 3 — SYNCHRONIZATION, DEADLOCKS & IPC
## Complete OS Interview Preparation | SDE-2 / SDE-3 / FAANG Level
### Version 1.0 | June 2026

---

# ═══════════════════════════════════════════════════════
# CHAPTER 1: THE SYNCHRONIZATION PROBLEM
# ═══════════════════════════════════════════════════════

## 1.1 Race Conditions — Why Concurrency is Hard

### THEORY BLOCK

A **race condition** occurs when the correctness of a program depends on the relative timing of events in concurrent threads or processes. The outcome "races" between different orderings of execution.

**Why race conditions are insidious:** They appear non-deterministically. Your program might work correctly 999 times out of 1000, then silently corrupt data once. This makes them extremely hard to debug.

### Real-World Disasters Caused by Race Conditions

1. **Therac-25 (1985-1987):** A radiation therapy machine. A race condition between the operator typing commands and the machine resetting allowed a high-power X-ray mode to fire without the beam spreader in place. **5 patients died, 5 were severely injured.** The bug was in a shared variable not properly synchronized.

2. **Northeast Blackout (2003):** A race condition in GE Energy's alarm management system caused alarms to be silently dropped. Operators didn't know about cascading failures until the blackout had spread to 55 million people.

3. **Mars Pathfinder (1997):** Priority inversion (a form of synchronization bug) caused the rover's computer to reset repeatedly. NASA fixed it remotely by enabling priority inheritance in VxWorks.

---

### Demonstrating a Race Condition in C

```c
/* Without synchronization — BUGGY */
#include <stdio.h>
#include <pthread.h>

long counter = 0;   /* SHARED — both threads access this */

void *increment(void *arg) {
    for (int i = 0; i < 1000000; i++) {
        counter++;   /* This is NOT atomic! It's 3 instructions:
                      * LOAD  r1, counter    ← thread can be preempted here!
                      * ADD   r1, 1          ← or here!
                      * STORE counter, r1    ← or here!
                      */
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment, NULL);
    pthread_create(&t2, NULL, increment, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Counter: %ld (expected 2000000)\n", counter);
    /* Output might be: 1,237,845 — different every run! */
}

/* WHY: Interleaving where both threads lose an increment:
 * Thread 1 LOADS counter=100
 * Thread 2 LOADS counter=100   ← BOTH read same old value!
 * Thread 1 ADDS: r1=101
 * Thread 2 ADDS: r2=101
 * Thread 1 STORES counter=101
 * Thread 2 STORES counter=101  ← Overwrites Thread 1's write!
 * Net result: counter went from 100 → 101, NOT 102. Lost an increment!
 */
```

---

### The Critical Section Problem

A **critical section** is any code that accesses shared resources that must not be executed concurrently.

```
Thread structure with critical section:
┌─────────────────────────────────────────┐
│         REMAINDER SECTION               │  ← Non-critical code (can run concurrently)
│  ... do_other_work() ...                │
├─────────────────────────────────────────┤
│         ENTRY SECTION                   │  ← Request permission to enter CS
│  lock(&mutex)                           │
├─────────────────────────────────────────┤
│         CRITICAL SECTION                │  ← Access shared resource
│  counter++;                             │
│  shared_queue.push(item);               │
├─────────────────────────────────────────┤
│         EXIT SECTION                    │  ← Signal done with CS
│  unlock(&mutex)                         │
├─────────────────────────────────────────┤
│         REMAINDER SECTION               │
└─────────────────────────────────────────┘
```

### Three Requirements for Correct CS Solution (MEMORIZE!)

1. **Mutual Exclusion:** Only ONE process/thread in CS at a time. Period.

2. **Progress:** If no process is in the CS and some process WANTS to enter, the decision about who enters MUST be made in finite time. The selection cannot be postponed indefinitely by processes not interested in entering.

3. **Bounded Waiting:** After a process requests to enter the CS, there is a BOUND on how many times OTHER processes can enter before it gets its turn. No starvation.

---

## 1.2 Peterson's Algorithm (Software Solution for 2 Processes)

```c
/* Peterson's Algorithm — software-only mutual exclusion for 2 threads */
int turn;           /* Whose turn is it? */
int flag[2];        /* flag[i] = TRUE means process i WANTS to enter CS */

/* Process i's entry/exit: */
flag[i] = TRUE;     /* Announce intention to enter CS */
turn = j;           /* Give turn to OTHER process (polite!) */
while (flag[j] && turn == j)
    ;               /* Wait if j WANTS CS and it's j's turn */
/* CRITICAL SECTION */
flag[i] = FALSE;    /* Done, announce exit */
```

**Proof of correctness:**
- **Mutual exclusion:** Processes can only BOTH be in CS simultaneously if `turn == i` AND `turn == j` — impossible (turn is a single variable)!
- **Progress:** If j doesn't want CS (flag[j]=FALSE), i enters immediately.
- **Bounded waiting:** If both want CS, whichever sets `turn` LAST gives way. The OTHER enters first. After j exits and sets flag[j]=FALSE, i enters. So j can't enter again before i.

**Why it fails on modern hardware:** CPUs and compilers REORDER instructions for performance. The stores to `flag[i]` and `turn` might be reordered, breaking Peterson's assumptions. You need **memory barriers** (mfence, __sync_synchronize) to prevent reordering.

---

## 1.3 Hardware Support — Atomic Operations

Modern CPUs provide atomic instructions:

### Test-and-Set (TAS)

```c
/* Hardware atomic instruction — cannot be interrupted */
bool TestAndSet(bool *target) {
    bool old = *target;   /* Read old value */
    *target = TRUE;       /* Set to TRUE */
    return old;           /* Return old value */
    /* Above 3 operations happen ATOMICALLY — no interrupt between them */
}

/* Spinlock using TAS: */
bool lock = FALSE;   /* FALSE = unlocked */

void acquire(bool *lock) {
    while (TestAndSet(lock) == TRUE)
        ;   /* Spin while lock is TRUE (held by someone) */
    /* When TestAndSet returns FALSE: old value was FALSE (unlocked),
     * we SET it to TRUE (now we hold the lock). */
}

void release(bool *lock) {
    *lock = FALSE;  /* Simply clear the lock */
}
```

**Problem:** TAS spinlock is NOT fair — no bounded waiting. A thread can be starved if others keep acquiring.

### Compare-and-Swap (CAS)

More powerful than TAS. Used in lock-free data structures.

```c
/* Hardware atomic instruction */
int CAS(int *mem, int expected, int new_val) {
    int old = *mem;
    if (old == expected)
        *mem = new_val;
    return old;
}

/* Lock-free counter increment using CAS: */
void atomic_increment(int *counter) {
    int old, new;
    do {
        old = *counter;      /* Read current value */
        new = old + 1;       /* Compute new value */
    } while (CAS(counter, old, new) != old);
    /* If another thread changed counter between read and CAS:
     * CAS fails (returns current value ≠ old), we retry! */
}
```

**ABA Problem with CAS:**
```
Thread 1: reads counter = A
Thread 2: changes counter A → B → A  (goes to B and back to A!)
Thread 1: CAS(counter, A, new) SUCCEEDS! (counter is A again)
          BUT the world changed — B happened in between!
          
Fix: Use tagged pointers (ABA counter): CAS on (value, version_counter)
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 2: SYNCHRONIZATION PRIMITIVES
# ═══════════════════════════════════════════════════════

## 2.1 Mutex (Mutual Exclusion Lock)

A **mutex** is a binary synchronization primitive that provides mutual exclusion.

```
Mutex states:
  UNLOCKED: nobody holds it, any thread can acquire it
  LOCKED:   one thread holds it, others must wait

Properties of a mutex:
  ✓ Mutual exclusion: only one thread holds it at a time
  ✓ Ownership: only the thread that LOCKED it can UNLOCK it
  ✓ Reentrant (if recursive mutex): same thread can lock multiple times
```

### POSIX Mutex API

```c
#include <pthread.h>

/* Declaration and initialization */
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
/* OR: */
pthread_mutex_t mtx;
pthread_mutex_init(&mtx, NULL);   /* attr=NULL uses defaults */

/* Locking */
pthread_mutex_lock(&mtx);    /* Block until acquired */
pthread_mutex_trylock(&mtx); /* Return immediately: 0=got it, EBUSY=couldn't */
pthread_mutex_timedlock(&mtx, &timeout); /* Give up after timeout */

/* Unlocking */
pthread_mutex_unlock(&mtx);  /* MUST be called by same thread that locked! */

/* Cleanup */
pthread_mutex_destroy(&mtx); /* Free resources (must be unlocked) */
```

### Mutex Internals: The futex (Fast Userspace Mutex)

Linux mutexes use **futex** (fast userspace mutex):

```
Fast path (NO contention — common case):
  pthread_mutex_lock() uses atomic CAS to set lock word
  If CAS succeeds (lock was free): lock acquired!
  NO system call! Entire lock acquisition in user space.
  Cost: ~5ns (a few atomic instructions)

Slow path (WITH contention — rare):
  CAS fails → someone else holds the lock
  pthread_mutex_lock() calls futex(FUTEX_WAIT) system call
  Kernel puts this thread on a wait queue
  Thread SLEEPS (not spinning — doesn't waste CPU)
  When lock is released: futex(FUTEX_WAKE) wakes one waiter
  Cost: ~1000-10000ns (system call + context switch)
```

**Why futex is brilliant:** In uncontested cases (the common case in well-designed programs), the mutex is essentially free — no kernel involvement at all!

---

## 2.2 Semaphores — Dijkstra's Invention (1965)

A **semaphore** is an integer variable with two atomic operations:
- **wait() / P() / down():** Decrement. Block if the result would be negative.
- **signal() / V() / up():** Increment. Wake one blocked thread if any.

```
Semaphore S:
  S ≥ 0: represents available resources
  
wait(S):  if S > 0: S-- (resource acquired)
          if S = 0: BLOCK (no resource available)

signal(S): S++ (release resource)
           if (any thread blocked on S): WAKE one of them

Binary semaphore (S ∈ {0,1}): Used like a mutex
Counting semaphore (S ∈ {0..N}): Used to count N available resources
```

### POSIX Semaphores

```c
#include <semaphore.h>

/* Unnamed semaphore (within same process or shared memory) */
sem_t sem;
sem_init(&sem, 0, 3);      /* pshared=0 (thread), initial value=3 */
sem_wait(&sem);            /* P() — decrement, block if zero */
sem_trywait(&sem);         /* Non-blocking wait */
sem_post(&sem);            /* V() — increment, wake waiter */
sem_getvalue(&sem, &val);  /* Read current value */
sem_destroy(&sem);

/* Named semaphore (survives process exit, identified by name) */
sem_t *s = sem_open("/mysem", O_CREAT, 0644, 1);
sem_wait(s);
/* ... critical section ... */
sem_post(s);
sem_close(s);
sem_unlink("/mysem");  /* Remove from filesystem */
```

### Mutex vs Semaphore — The KEY Differences

| Feature | Mutex | Semaphore |
|---------|-------|-----------|
| **Value range** | Binary (locked/unlocked) | Integer ≥ 0 |
| **Ownership** | YES — only locker can unlock | NO — any thread can signal |
| **Purpose** | Mutual exclusion | Signaling + mutual exclusion |
| **Initial value** | 1 (unlocked) | Any value N |
| **Use case** | Protecting shared data | Counting resources, signaling |
| **Priority inheritance** | Usually supported | Usually NOT |

**When to use which:**
- Mutex: protect a critical section (write to shared data structure)
- Semaphore: signal one thread to proceed, count available resources (connection pool)

---

## 2.3 Condition Variables

A **condition variable** allows threads to wait for a specific CONDITION to become true, while atomically releasing a mutex.

**Always used WITH a mutex — they are a pair!**

### POSIX Condition Variables

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
int condition_met = 0;

/* Waiter thread: */
pthread_mutex_lock(&mutex);
while (!condition_met) {        /* MUST use while, NOT if! (spurious wakeups!) */
    pthread_cond_wait(&cond, &mutex);  /* Atomically: release mutex + sleep */
}
/* condition_met is now TRUE, and we hold the mutex */
do_work_with_shared_data();
pthread_mutex_unlock(&mutex);

/* Signaler thread: */
pthread_mutex_lock(&mutex);
condition_met = 1;
pthread_cond_signal(&cond);     /* Wake ONE waiting thread */
/* OR: pthread_cond_broadcast(&cond); to wake ALL waiters */
pthread_mutex_unlock(&mutex);
```

### Why WHILE and not IF?

```
SPURIOUS WAKEUPS: pthread_cond_wait() can return even when no signal was sent!
(This is allowed by POSIX for implementation efficiency reasons)

If you use IF:
  Thread A waits: IF(!condition) cond_wait()
  Spurious wakeup occurs
  Thread A proceeds as if condition is true — BUG!

If you use WHILE:
  Thread A waits: WHILE(!condition) cond_wait()
  Spurious wakeup occurs → thread re-checks condition → goes back to sleep
  Correct behavior!
```

### Condition Variable Internals

```c
pthread_cond_wait(&cond, &mutex):
  1. Add this thread to cond's wait queue
  2. ATOMICALLY release mutex (so signaler can proceed)
  3. Sleep (block this thread)
  
  [Later: signaler calls pthread_cond_signal(&cond)]
  
  4. Wake up (removed from wait queue)
  5. Re-acquire mutex (may block if another thread holds it)
  6. Return to caller (caller HOLDS mutex)
```

---

## 2.4 Classic Synchronization Problems

### Producer-Consumer (Bounded Buffer)

**Problem:** Producers create items, consumers consume them. Shared buffer of size N.

```c
/* Producer-Consumer with semaphores */
#define N 10
sem_t empty;    /* Counts empty slots (initially N) */
sem_t full;     /* Counts full slots (initially 0) */
sem_t mutex;    /* Binary semaphore for buffer access (initially 1) */

int buffer[N];
int in = 0, out = 0;   /* buffer insertion and removal positions */

void producer(void) {
    while (1) {
        int item = produce_item();
        sem_wait(&empty);      /* Wait for empty slot (decrement empty count) */
        sem_wait(&mutex);      /* Enter critical section */
        buffer[in] = item;     /* Add item to buffer */
        in = (in + 1) % N;    /* Circular buffer */
        sem_post(&mutex);      /* Exit critical section */
        sem_post(&full);       /* Signal: one more full slot */
    }
}

void consumer(void) {
    while (1) {
        sem_wait(&full);       /* Wait for full slot */
        sem_wait(&mutex);      /* Enter critical section */
        int item = buffer[out]; /* Remove item */
        out = (out + 1) % N;
        sem_post(&mutex);      /* Exit critical section */
        sem_post(&empty);      /* Signal: one more empty slot */
        consume_item(item);
    }
}

/* INITIALIZATION */
void init() {
    sem_init(&empty, 0, N);   /* N empty slots initially */
    sem_init(&full,  0, 0);   /* 0 full slots initially */
    sem_init(&mutex, 0, 1);   /* Binary semaphore for CS */
}

/* COMMON BUG: swapping sem_wait(&empty) and sem_wait(&mutex) in producer:
 * Producer: sem_wait(&mutex) first, then sem_wait(&empty)
 * Consumer: sem_wait(&full) first, then sem_wait(&mutex)
 * If buffer is full: producer holds mutex, waits for empty.
 * Consumer can't acquire mutex → DEADLOCK! */
```

### Readers-Writers Problem

**Problem:** Multiple readers can read simultaneously. A writer needs exclusive access.

```c
/* First Readers-Writers (Readers Preference) */
int read_count = 0;           /* Number of active readers */
pthread_mutex_t mutex;        /* Protects read_count */
pthread_mutex_t write_lock;   /* Exclusive lock for writers */

void reader(void) {
    pthread_mutex_lock(&mutex);
    read_count++;
    if (read_count == 1)                    /* First reader? */
        pthread_mutex_lock(&write_lock);     /* Block writers */
    pthread_mutex_unlock(&mutex);
    
    /* ── READ CRITICAL SECTION ── */
    do_read();
    
    pthread_mutex_lock(&mutex);
    read_count--;
    if (read_count == 0)                     /* Last reader? */
        pthread_mutex_unlock(&write_lock);   /* Allow writers */
    pthread_mutex_unlock(&mutex);
}

void writer(void) {
    pthread_mutex_lock(&write_lock);   /* Need exclusive access */
    /* ── WRITE CRITICAL SECTION ── */
    do_write();
    pthread_mutex_unlock(&write_lock);
}

/* BUG: Writers can STARVE if readers continuously arrive!
 * First reader blocks writer. While writer waits, more readers arrive.
 * Last reader unblocks writer but new readers arrive again. Writer waits forever!
 * FIX: Use writers-preference or fair (queue-based) solution. */
```

### Dining Philosophers Problem

**Problem:** 5 philosophers, 5 chopsticks (between each pair), alternate between thinking and eating. Need both chopsticks to eat.

```c
/* Solution 3: Resource Hierarchy (break circular wait) */
#define N 5
pthread_mutex_t chopstick[N];

void philosopher(int i) {
    /* Always pick up LOWER-NUMBERED chopstick first! */
    int left  = i;
    int right = (i + 1) % N;
    
    int first  = (left < right) ? left : right;  /* Lower number */
    int second = (left < right) ? right : left;  /* Higher number */
    
    while (1) {
        think();
        
        pthread_mutex_lock(&chopstick[first]);   /* Always lower first */
        pthread_mutex_lock(&chopstick[second]);  /* Then higher */
        
        eat();
        
        pthread_mutex_unlock(&chopstick[second]);
        pthread_mutex_unlock(&chopstick[first]);
    }
}

/* Why this works: Cannot have circular wait!
 * Circular wait needs: P0 waits for P1, P1 waits for P2, ..., P4 waits for P0
 * With resource hierarchy: everyone picks lower chopstick first.
 * The philosopher with chopstick 4 and 0: picks 0 first (0 < 4).
 * Philosopher i picks i first. Philosopher (i+1)%5 picks i or (i+1)%5 first.
 * No circular wait is possible → no deadlock! */
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 3: DEADLOCKS — COMPLETE COVERAGE
# ═══════════════════════════════════════════════════════

## 3.1 What is a Deadlock?

A **deadlock** is a situation where a set of processes are ALL blocked, each waiting for a resource held by another process in the set. Nobody can proceed.

```
Deadlock visualization:
  Process A: holds Resource 1, waiting for Resource 2
  Process B: holds Resource 2, waiting for Resource 1
  
  A ──(holds)──▶ R1 ◀──(waits for)── B
  A ──(waits for)──▶ R2 ◀──(holds)── B
  
  Neither A nor B can proceed!
```

**Real-world example:** Two trains approaching each other on a single track. Neither will back up. Both halt forever.

---

## 3.2 Coffman's Four Necessary Conditions (MEMORIZE ALL FOUR!)

ALL FOUR conditions must hold simultaneously for a deadlock to occur. To prevent deadlock, eliminate at least ONE.

### 1. Mutual Exclusion
At least one resource must be held in a non-sharable mode (only one process at a time). If multiple processes could use a resource simultaneously, no deadlock.

### 2. Hold and Wait
A process must be holding at least one resource AND waiting to acquire additional resources held by other processes.

### 3. No Preemption
Resources cannot be forcibly taken away from a process. A process releases resources only voluntarily (when done).

### 4. Circular Wait
There exists a set of processes {P0, P1, ..., Pn} such that P0 is waiting for a resource held by P1, P1 is waiting for a resource held by P2, ..., Pn is waiting for a resource held by P0.

```
Circular wait of 4 processes:
    P1 → waits for resource held by → P2
    P2 → waits for resource held by → P3
    P3 → waits for resource held by → P4
    P4 → waits for resource held by → P1
    (circular!)
```

---

## 3.3 Resource Allocation Graph (RAG)

The RAG is a directed graph for visualizing resource allocation state.

```
Notation:
  ○ Circle = Process
  ■ Rectangle with dots = Resource type (each dot = one instance)
  
  Process → Resource: REQUEST edge (process wants this resource)
  Resource → Process: ASSIGNMENT edge (resource held by this process)

EXAMPLE 1 — Deadlock (single-instance resources):

    P1 ──────▶ R1 ──────▶ P2
    ▲                     │
    │                     │
    R2 ◀───────────────── ┘

  P1 holds R2, wants R1.
  P2 holds R1, wants R2.
  CYCLE DETECTED = DEADLOCK!

EXAMPLE 2 — No Deadlock (multi-instance resources):

  R1 has 2 instances: ■■
  
    P1 ──▶ R1 (instance 1) ──▶ P2
           R1 (instance 2) ──▶ P3
    P2 ──▶ R2 ──▶ P4

  P1 is waiting for R1. But R1 has instance 2 free... 
  Wait, both instances are held by P2 and P3.
  But P3 holds only R1. If P3 finishes: releases R1 → P1 can run.
  No deadlock! (Even though there's a cycle P1→R1→P2→R2→P4...
                this particular graph doesn't guarantee deadlock for multi-instance)
```

**RULE:** 
- Single-instance resources: **Cycle in RAG = Deadlock (necessary AND sufficient)**
- Multi-instance resources: **Cycle in RAG = Deadlock POSSIBLE but not certain (necessary but not sufficient)**

---

## 3.4 Banker's Algorithm — COMPLETE WITH NUMERICALS

The **Banker's Algorithm** (Dijkstra, 1965) is a deadlock avoidance algorithm. Named after a bank that never gives a loan unless it can guarantee stability for all customers.

### Data Structures

```
N processes, M resource types.

Available[M]:
  Available[j] = number of instances of resource j currently available

Max[N][M]:
  Max[i][j] = maximum demand of process i for resource j

Allocation[N][M]:
  Allocation[i][j] = instances of resource j currently allocated to process i

Need[N][M]:
  Need[i][j] = Max[i][j] - Allocation[i][j]
  (how many MORE instances process i may request)

Work[M]:     (working vector, starts as Available)
Finish[N]:   (initially FALSE for all processes)
```

### Safety Algorithm

```
Given the current state, is there a SAFE SEQUENCE?

Algorithm:
  Work = Available
  Finish[i] = FALSE for all i

  LOOP:
    Find i such that: Finish[i] = FALSE AND Need[i] ≤ Work
    If found:
      Work += Allocation[i]  (process i finishes, releases its resources)
      Finish[i] = TRUE
      Go to LOOP
    
  If all Finish[i] = TRUE: SAFE STATE (safe sequence exists)
  Otherwise: UNSAFE STATE
```

### Resource-Request Algorithm

```
Process i requests Resources[M] (a request vector):

Step 1: Check if Request[i] ≤ Need[i]
        If not: error (exceeds maximum claim)

Step 2: Check if Request[i] ≤ Available
        If not: process must wait (resources not available)

Step 3: PRETEND to allocate:
        Available -= Request[i]
        Allocation[i] += Request[i]
        Need[i] -= Request[i]

Step 4: Run Safety Algorithm on the new state
        If SAFE: grant the request (state is safe)
        If UNSAFE: DENY the request, restore old state:
                   Available += Request[i]
                   Allocation[i] -= Request[i]
                   Need[i] += Request[i]
```

### WORKED EXAMPLE 1 — Classic 5-Process, 3-Resource

```
5 processes: P0, P1, P2, P3, P4
3 resource types: A (10), B (5), C (7)

Current state:
           Allocation    Max          Need (=Max-Alloc)   Available
           A  B  C      A  B  C      A  B  C             A  B  C
P0:        0  1  0      7  5  3      7  4  3              3  3  2
P1:        2  0  0      3  2  2      1  2  2
P2:        3  0  2      9  0  2      6  0  0
P3:        2  1  1      2  2  2      0  1  1
P4:        0  0  2      4  3  3      4  3  1

Verify: Total Allocated = (0+2+3+2+0, 1+0+0+1+0, 0+0+2+1+2) = (7,2,5)
        Total = (10,5,7). Available = (10-7, 5-2, 7-5) = (3,3,2) ✓

SAFETY ALGORITHM:
Work = [3,3,2], Finish = [F,F,F,F,F]

Round 1: Find i where Need[i] ≤ Work=[3,3,2]:
  P0: Need=[7,4,3]. 7>3 → NO
  P1: Need=[1,2,2]. 1≤3, 2≤3, 2≤2 → YES!
  P1 can complete! Work = Work + Alloc[P1] = [3,3,2]+[2,0,0] = [5,3,2]
  Finish[P1] = TRUE

Round 2: Find i where Need[i] ≤ Work=[5,3,2]:
  P0: Need=[7,4,3]. 7>5 → NO
  P2: Need=[6,0,0]. 6>5 → NO
  P3: Need=[0,1,1]. 0≤5, 1≤3, 1≤2 → YES!
  P3 finishes. Work = [5,3,2]+[2,1,1] = [7,4,3]. Finish[P3]=TRUE

Round 3: Work=[7,4,3]:
  P0: Need=[7,4,3]. 7≤7, 4≤4, 3≤3 → YES!
  P0 finishes. Work = [7,4,3]+[0,1,0] = [7,5,3]. Finish[P0]=TRUE

Round 4: Work=[7,5,3]:
  P2: Need=[6,0,0]. 6≤7 → YES!
  P2 finishes. Work = [7,5,3]+[3,0,2] = [10,5,5]. Finish[P2]=TRUE

Round 5: Work=[10,5,5]:
  P4: Need=[4,3,1]. 4≤10, 3≤5, 1≤5 → YES!
  P4 finishes. Work = [10,5,5]+[0,0,2] = [10,5,7]. Finish[P4]=TRUE

All Finish = TRUE → SAFE STATE!
Safe sequence: P1 → P3 → P0 → P2 → P4
```

### WORKED EXAMPLE 2 — Resource Request

```
(Using same state as above)
P1 requests [1,0,2]:

Step 1: Request[1]=[1,0,2] ≤ Need[1]=[1,2,2]? YES ✓
Step 2: Request[1]=[1,0,2] ≤ Available=[3,3,2]? YES ✓
Step 3: Pretend allocation:
  Available = [3,3,2]-[1,0,2] = [2,3,0]
  Alloc[P1] = [2,0,0]+[1,0,2] = [3,0,2]
  Need[P1] = [1,2,2]-[1,0,2] = [0,2,0]

Step 4: Safety check with new state:
  Work = [2,3,0]
  
  P1: Need=[0,2,0]. 0≤2, 2≤3, 0≤0 → YES! Work=[2,3,0]+[3,0,2]=[5,3,2]. Finish[P1]=T
  P3: Need=[0,1,1]. 0≤5, 1≤3, 1≤2 → YES! Work=[5,3,2]+[2,1,1]=[7,4,3]. Finish[P3]=T
  P0: Need=[7,4,3]. YES! Work=[7,4,3]+[0,1,0]=[7,5,3]. Finish[P0]=T
  P2: Need=[6,0,0]. YES! Work=[7,5,3]+[3,0,2]=[10,5,5]. Finish[P2]=T
  P4: Need=[4,3,1]. YES! Work=[10,5,5]+[0,0,2]=[10,5,7]. Finish[P4]=T
  
  SAFE! → GRANT the request to P1.
```

---

## 3.5 Deadlock Prevention

### Attack Each Coffman Condition

#### 1. Attack Mutual Exclusion
Make resources SHARABLE: read-only data, immutable objects. Not always possible (printers, locks must be exclusive).

#### 2. Attack Hold and Wait
**Method A:** Require processes to request ALL resources at once before starting.
```
Problems:
- Low utilization: P holds printer for 2 hours but only uses it 5 minutes
- Starvation: P needs {scanner, printer}. If printer is always busy, P never starts
- Must know all needs in advance (impossible for interactive programs)
```

**Method B:** Process releases all resources before requesting new ones.
```
Problems:
- Loss of progress: P does work with Resource A, then needs B.
  P releases A, requests {A, B}. Now must redo work to get A back!
- Only works for resources whose state can be saved
```

#### 3. Attack No Preemption
Allow OS to forcibly take resources:
- Works for: memory (swap out), CPU (preemptive scheduling)
- DOESN'T work for: printers (can't print half a document), locks (state lost)

**Approach:** If P holds some resources and requests another that isn't available:
1. Check if resources P needs are held by waiting processes
2. If yes: preempt those resources and add to P's resource list
3. If no: P waits. P's own resources may be preempted by others.

#### 4. Attack Circular Wait (MOST PRACTICAL!)
**Total ordering of resources:** Assign a global number to each resource type. Processes must request resources in INCREASING order of numbers.

```c
/* Example: Resources numbered: Mutex_A=1, Mutex_B=2, Mutex_C=3 */

/* CORRECT — always lock in increasing order: */
pthread_mutex_lock(&mutex_A);   /* 1 first */
pthread_mutex_lock(&mutex_B);   /* 2 second */
/* ... critical section ... */
pthread_mutex_unlock(&mutex_B);
pthread_mutex_unlock(&mutex_A);

/* If ALL code follows this rule: circular wait is IMPOSSIBLE!
 * Circular wait needs: P holds A, waits for B, AND P holds B, waits for A
 * With ordering: if P holds A(=1) and wants B(=2), P has gone 1→2 ✓
 *               if Q holds B(=2) and wants A(=1), Q is trying to go 2→1 ✗ FORBIDDEN
 * Therefore Q cannot hold B and wait for A → no circular wait! */
```

**How to implement in large codebase:**
- Maintain a lock ordering table (documentation)
- Use static analysis tools to detect ordering violations
- Use dynamic lock order checkers (Valgrind's Helgrind, Google's sanitizers)

---

## 3.6 Deadlock Detection and Recovery

### Detection Algorithm (Multi-Instance Resources)

Similar to Banker's Safety Algorithm but determines if current state IS deadlocked:

```
Work = Available
Finish[i] = FALSE if Allocation[i] != 0 (process holds some resource)
           = TRUE  if Allocation[i] == 0 (process has no resources)

LOOP:
  Find i: Finish[i]=FALSE AND Request[i] ≤ Work
  If found: Work += Allocation[i]; Finish[i]=TRUE; goto LOOP

If any Finish[i]=FALSE: DEADLOCK! Process i is deadlocked.
```

### Recovery from Deadlock

#### Method 1: Process Termination
- **Kill ALL deadlocked processes:** Simple but lots of lost work.
- **Kill one at a time:** Until deadlock is broken. Which to kill?
  - Process with lowest priority
  - Process that has computed the least (minimize work lost)
  - Process that uses fewest resources (easier to restart)
  - Process that is NOT interactive (less disruptive to user)

#### Method 2: Resource Preemption
Select a process, preempt its resources, roll back to a safe state, restart.

**Rollback:** Requires **checkpointing** — periodic snapshots of process state.

**Starvation prevention:** Keep track of how many times a process has been preempted. If too many times: always pick a different victim.

---

# ═══════════════════════════════════════════════════════
# CHAPTER 4: INTER-PROCESS COMMUNICATION (IPC)
# ═══════════════════════════════════════════════════════

## 4.1 IPC Overview

Processes are ISOLATED — by design, one process cannot access another's memory. IPC mechanisms provide controlled communication channels.

```
IPC Taxonomy:
                        ┌──────────────────────────────┐
                        │          IPC MECHANISMS       │
                        └──────────────┬───────────────┘
                   ┌────────────────────┼──────────────────┐
                   ▼                    ▼                   ▼
          ┌─────────────┐    ┌─────────────────┐  ┌─────────────┐
          │  Data Flow  │    │  Shared State   │  │  Signaling  │
          │  (streaming)│    │  (shared memory)│  │  (events)   │
          └──────┬──────┘    └────────┬────────┘  └──────┬──────┘
                 │                    │                   │
         ┌───────┴──────┐   ┌────────┴──────┐    ┌──────┴──────┐
         │ Pipes (anon) │   │ Shared Memory │    │  Signals    │
         │ Named Pipes  │   │ Memory-mapped │    │             │
         │ Message Queue│   │ files (mmap)  │    │             │
         │ Sockets      │   └───────────────┘    └─────────────┘
         └──────────────┘

Performance (fastest to slowest):
  Shared Memory (no copy!) > Pipes > Message Queues > Sockets
```

---

## 4.2 Pipes

```c
/* Anonymous Pipe: parent ↔ child only */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

int main(void) {
    int pipefd[2];  /* pipefd[0] = read end, pipefd[1] = write end */
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        /* CHILD: reads from pipe */
        close(pipefd[1]);  /* Close unused write end */
        
        char buf[100];
        ssize_t n = read(pipefd[0], buf, sizeof(buf)-1);
        buf[n] = '\0';
        printf("Child received: '%s'\n", buf);
        
        close(pipefd[0]);
        return 0;
        
    } else {
        /* PARENT: writes to pipe */
        close(pipefd[0]);  /* Close unused read end */
        
        const char *msg = "Hello from parent!";
        write(pipefd[1], msg, strlen(msg));
        
        close(pipefd[1]);  /* Close write end so child gets EOF */
        wait(NULL);
    }
    return 0;
}
/* Output: Child received: 'Hello from parent!' */
```

### Named Pipes (FIFOs)

Named pipes exist in the filesystem. ANY process can open them by name (not just parent-child).

```c
/* Server creates FIFO */
#include <sys/stat.h>
#include <fcntl.h>

mkfifo("/tmp/myfifo", 0666);    /* Create FIFO file */

/* Server reads: */
int fd = open("/tmp/myfifo", O_RDONLY);  /* BLOCKS until writer opens! */
read(fd, buffer, sizeof(buffer));
close(fd);

/* Client writes (separate process): */
int fd = open("/tmp/myfifo", O_WRONLY);  /* BLOCKS until reader opens! */
write(fd, "Hello!", 6);
close(fd);
unlink("/tmp/myfifo");           /* Remove FIFO file when done */
```

**Pipe vs FIFO differences:**

| Feature | Pipe | FIFO |
|---------|------|------|
| **Who can use** | Only related processes (parent-child after fork) | ANY processes |
| **Named in filesystem** | No | Yes (/tmp/myfifo) |
| **Survives process exit** | No | Yes (until unlink()) |
| **Opening** | Already open after pipe() | Must call open() explicitly |

---

## 4.3 Shared Memory

**Fastest IPC!** Both processes map the same physical memory into their address spaces. No copying — direct access.

```c
/* POSIX Shared Memory */
#include <sys/mman.h>
#include <fcntl.h>

/* Process A (creator): */
int fd = shm_open("/myshm", O_CREAT|O_RDWR, 0666);
ftruncate(fd, sizeof(SharedData));           /* Set size */

SharedData *data = mmap(NULL, sizeof(SharedData),
                        PROT_READ|PROT_WRITE,
                        MAP_SHARED, fd, 0);  /* Map into address space */
close(fd);

data->value = 42;              /* Direct write! No system call for this */
strncpy(data->message, "hello", sizeof(data->message));

/* Process B (user): */
int fd = shm_open("/myshm", O_RDWR, 0666);  /* No O_CREAT — must already exist */

SharedData *data = mmap(NULL, sizeof(SharedData),
                        PROT_READ|PROT_WRITE,
                        MAP_SHARED, fd, 0);
close(fd);

printf("Value: %d, Message: %s\n", data->value, data->message);
/* Output: Value: 42, Message: hello */

munmap(data, sizeof(SharedData));  /* Unmap when done */
shm_unlink("/myshm");              /* Remove (by last user) */
```

**CRITICAL:** Shared memory needs EXTERNAL SYNCHRONIZATION! Use semaphores or mutexes stored in the shared memory itself.

**Dangerous gotcha:** Pointers inside shared memory are INVALID across processes! Virtual addresses differ between processes. Use **offsets** instead:

```c
/* WRONG: */
data->ptr = &data->buffer[5];  /* This is a virtual address, invalid in other process! */

/* CORRECT: */
data->offset = 5;              /* Offset within shared region — valid! */
char *item = (char*)data + data->offset;  /* Each process computes its own pointer */
```

---

## 4.4 Message Queues

Message queues allow processes to send and receive **discrete messages** with associated priorities.

```c
/* POSIX Message Queues */
#include <mqueue.h>
#include <string.h>

/* Sender: */
mqd_t mq = mq_open("/myqueue", O_CREAT|O_WRONLY, 0666, NULL);

char msg[] = "Task: compute sum of 1-1000";
mq_send(mq, msg, strlen(msg)+1, 0);  /* priority=0 */
mq_close(mq);

/* Receiver: */
mqd_t mq = mq_open("/myqueue", O_RDONLY, 0666, NULL);

struct mq_attr attr;
mq_getattr(mq, &attr);

char buffer[attr.mq_msgsize];
unsigned int priority;
mq_receive(mq, buffer, sizeof(buffer), &priority);  /* Blocks if empty */
printf("Received: '%s' with priority %u\n", buffer, priority);

mq_close(mq);
mq_unlink("/myqueue");   /* Remove the queue */
```

**Message Queue vs Pipe differences:**
- Messages have **type/priority** (received in priority order)
- Messages have **boundaries** (not byte streams like pipes)
- Message queue **persists** after sender exits (until unlinked)
- Multiple senders and receivers supported

---

## 4.5 Signals as IPC (Limited)

Signals can send very limited information (effectively just a number from 0-31, or with sigqueue: one integer + one pointer for real-time signals).

```c
/* Sending a signal with data (POSIX real-time signals) */
union sigval sv;
sv.sival_int = 42;                        /* Data to send */
sigqueue(target_pid, SIGRTMIN, sv);       /* Send to process */

/* Receiving signal with data */
struct sigaction sa;
sa.sa_flags = SA_SIGINFO;                 /* Use sa_sigaction, not sa_handler */
sa.sa_sigaction = signal_handler;
sigaction(SIGRTMIN, &sa, NULL);

void signal_handler(int sig, siginfo_t *info, void *context) {
    printf("Received signal %d with value %d from PID %d\n",
           sig, info->si_value.sival_int, info->si_pid);
}
```

---

## MCQ Bank — Synchronization and Deadlocks

**Q1.** Which of the following is NOT a necessary condition for deadlock? [EASY]
- A) Mutual exclusion
- B) Hold and wait
- C) Circular wait
- D) Preemption ✓ (CORRECT — it's NO PREEMPTION that's required, not preemption!)

*Explanation:* The four conditions are: mutual exclusion, hold and wait, NO preemption, and circular wait. Preemption itself would PREVENT deadlock!

**Q2.** The Banker's Algorithm is used for: [MEDIUM]
- A) Deadlock detection
- B) Deadlock prevention
- C) Deadlock avoidance ✓ (CORRECT)
- D) Deadlock recovery

*Explanation:* Banker's Algorithm is deadlock AVOIDANCE — it doesn't prevent deadlock from being possible, but it only grants resource requests that keep the system in a safe state.

**Q3.** Which of these is the fastest IPC mechanism? [MEDIUM]
- A) TCP sockets
- B) Pipes
- C) Message queues
- D) Shared memory ✓ (CORRECT — no copying, direct memory access)

**Q4.** A semaphore with initial value 1 is used as: [EASY]
- A) A counting semaphore
- B) A binary semaphore (mutex) ✓ (CORRECT)
- C) A read-write lock
- D) A condition variable

**Q5.** In the Producer-Consumer problem, what happens when a producer tries to produce into a FULL buffer? [MEDIUM]
- A) The item is lost
- B) The producer gets an error
- C) The producer blocks (waits) ✓ (CORRECT)
- D) The oldest item is overwritten

**Q6.** Peterson's Algorithm satisfies: [HARD]
- A) Mutual exclusion only
- B) Mutual exclusion and progress only
- C) All three requirements: mutual exclusion, progress, bounded waiting ✓ (CORRECT)
- D) None of the requirements

**Q7.** A cycle in a Resource Allocation Graph with SINGLE instance resources indicates: [MEDIUM]
- A) Possible deadlock
- B) Definite deadlock ✓ (CORRECT)
- C) No deadlock
- D) Livelock

**Q8.** The difference between livelock and deadlock: [HARD]
- A) In livelock, processes are blocked. In deadlock, they're running.
- B) In livelock, processes keep changing state but make no progress. In deadlock, processes are stuck. ✓ (CORRECT)
- C) Livelock involves more processes than deadlock.
- D) Livelock is not a real OS problem.

**Q9.** Breaking which Coffman condition is most practical in real systems? [HARD]
- A) Mutual exclusion
- B) Hold and wait
- C) No preemption
- D) Circular wait ✓ (CORRECT — lock ordering is the most practical prevention strategy)

**Q10.** Spurious wakeups in condition variables are handled by: [MEDIUM]
- A) Using signal() instead of wait()
- B) Using a while loop instead of if ✓ (CORRECT)
- C) Using a mutex instead of a condition variable
- D) Spurious wakeups cannot occur

---

## Interview Questions — Synchronization and Deadlocks

### Q1: "Explain the difference between a mutex and a semaphore."

**Answer:**
> "Both mutex and semaphore are synchronization primitives, but they have different semantics:
>
> A **mutex** is a binary lock with OWNERSHIP. Only the thread that acquired the mutex can release it. It's used for mutual exclusion — protecting a critical section. Mutexes typically support priority inheritance (for real-time systems). Implementation: pthread_mutex_t.
>
> A **semaphore** is an integer counter without ownership. ANY thread can call signal() (increment), regardless of which thread called wait() (decrement). This makes semaphores suitable for SIGNALING — 'producer signals consumer that data is ready.' Semaphores can also count multiple resources (counting semaphore with initial value N means N resources available).
>
> **When to use mutex:** When you need to protect shared data from concurrent modification. Use mutex to ensure only one thread executes a critical section.
>
> **When to use semaphore:** When you need to signal another thread that some event occurred, or when you need to limit access to N copies of a resource (connection pool of size 10: semaphore initialized to 10).
>
> **Common pitfall:** Using a mutex for signaling can cause deadlock. Thread A acquires mutex and waits for Thread B to signal. But Thread B can't acquire mutex to do signaling. Use a condition variable (with mutex) for signaling instead."

### Q2: "Prove that the Banker's Algorithm is correct."

**Answer:**
> "The Banker's Algorithm is correct because it only grants requests that maintain the system in a SAFE STATE, defined as: there exists at least one safe sequence — an ordering of all processes such that each process can complete using available + previously-allocated resources.
>
> **Proof sketch:**
> - If the system is in a safe state with sequence P1, P2, ..., Pn: P1 can complete using available resources. When P1 finishes, it releases resources. P2 can now complete (has resources left by P1 + original available), etc.
> - No deadlock occurs because each process in the safe sequence CAN complete.
> - The Banker's Algorithm ensures that granting any request doesn't eliminate ALL safe sequences.
> - If the new state after granting has no safe sequence → unsafe → we deny the request.
> - Since we only ever move to safe states, and a safe state has at least one completion order, we guarantee no deadlock.
>
> **Why Banker's is not used in practice:** (1) Must know maximum resource needs in advance — impossible for interactive programs. (2) Fixed number of processes and resources assumed — unrealistic in dynamic systems. (3) Overhead of running safety algorithm on every request."

### Q3: "How does the Linux kernel prevent deadlocks?"

**Answer:**
> "Linux uses several strategies:
>
> 1. **Lock ordering:** The kernel documents and enforces lock ordering hierarchies. For example: if you hold a spinlock, you cannot acquire a sleeping lock (mutex). Within spinlocks, there are documented ordering rules (e.g., interrupt lock before device lock).
>
> 2. **lockdep:** Linux's dynamic lock dependency checker (enabled in debug kernels). It builds a graph of lock acquisition orders at runtime and warns if a potential deadlock is detected (circular dependency in the graph). This is how new kernel bugs are found!
>
> 3. **Lock primitives with timeouts:** trylock variants allow code to detect failure and handle it rather than blocking forever.
>
> 4. **RCU (Read-Copy-Update):** For read-mostly data structures, RCU allows readers to proceed WITHOUT any locks. Eliminates the chance of reader-writer deadlock.
>
> 5. **Preemptible kernel (PREEMPT_RT):** With RT patch, nearly all kernel locks are preemptible, reducing the chance of priority inversion and related deadlock-like situations."

---

*End of DOC_3 — Synchronization, Deadlocks, and IPC*
*Coverage: Race conditions, critical section, Peterson's algorithm, atomic ops, mutex, semaphore, condition variables, producer-consumer, readers-writers, dining philosophers, deadlock conditions, RAG, Banker's algorithm, deadlock prevention/detection/recovery, pipes, shared memory, message queues, signals*
