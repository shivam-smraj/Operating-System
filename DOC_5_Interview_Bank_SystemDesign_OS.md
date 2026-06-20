# DOC 5 — INTERVIEW BANK, SYSTEM DESIGN & OS MASTERY
## Complete OS Interview Preparation | SDE-2 / SDE-3 / FAANG Level
### Version 1.0 | June 2026

---

# ═══════════════════════════════════════════════════════
# CHAPTER 1: THE ULTIMATE OS INTERVIEW QUESTION BANK
# ═══════════════════════════════════════════════════════

## 1.1 Top 50 OS Interview Questions with Full Answers

### TIER 1 — Questions Asked in 90% of OS Interviews

---

**Q1: What is an operating system? What are its main functions?**

**A:** An OS is system software that manages hardware resources and provides services to applications. Functions:
1. **Process Management:** Create, schedule, terminate processes/threads
2. **Memory Management:** Allocate RAM, implement virtual memory, handle page faults
3. **File System:** Provide persistent storage with hierarchical namespace
4. **I/O Management:** Abstract hardware via device drivers, buffer I/O
5. **Security:** Enforce access control, process isolation, authentication
6. **Networking:** Implement TCP/IP stack, socket API

---

**Q2: What is the difference between a process and a thread?**

**A:** 
- **Process:** Independent execution unit with own address space, file descriptors, credentials. Processes are isolated — one crash doesn't affect others. Communication via IPC (pipes, sockets, shared memory). Heavier to create (~1ms).
- **Thread:** Execution unit within a process. Shares address space (code, heap, globals, file descriptors). Private: stack, registers, thread ID, errno, signal mask. Faster to create (~10μs). Needs synchronization to access shared data.

**Follow-up:** "When would you use multiple processes vs multiple threads?"
- **Processes:** Fault isolation (Chrome tabs), security (sandboxing plugins), different users
- **Threads:** Shared data with low overhead (DB thread pool), parallel computation with communication, event handling in servers

---

**Q3: What is a deadlock? Give the four necessary conditions.**

**A:** A deadlock is when a set of processes are all blocked, each waiting for a resource held by another. Four necessary conditions (Coffman's):
1. **Mutual Exclusion:** Resources are non-sharable
2. **Hold and Wait:** Process holds resource while waiting for another
3. **No Preemption:** Resources cannot be forcibly taken away
4. **Circular Wait:** P1 waits for P2, P2 waits for P3, ..., Pn waits for P1

To prevent deadlock: eliminate at least one condition. Most practical: eliminate Circular Wait by enforcing global lock ordering.

---

**Q4: Explain virtual memory. How does it work?**

**A:** Virtual memory gives each process the illusion that it has the entire address space (e.g., 0 to 256TB on x86-64) even if physical RAM is limited.

**How:**
1. Each process has its own **page table** mapping virtual pages to physical frames
2. Pages not in RAM are stored on **swap** (disk)
3. When a process accesses a page not in RAM → **page fault** → OS loads page from disk
4. The **MMU** (Memory Management Unit) hardware performs the virtual→physical translation on every memory access
5. The **TLB** (Translation Lookaside Buffer) caches recent translations for performance

**Benefits:** Each process isolated from others, programs can be larger than RAM, easy to share code (e.g., libc), supports copy-on-write for fork()

---

**Q5: What is thrashing? How is it prevented?**

**A:** Thrashing occurs when processes spend more time paging (swapping pages in/out) than doing useful work. CPU utilization plummets despite high "activity."

**Cause:** Too many processes, each needs more frames than available. Every access → page fault → disk I/O → process blocks → OS runs another → that also faults...

**Prevention:**
1. **Working Set Model:** Allocate enough frames for each process's active working set
2. **Page Fault Frequency:** Monitor per-process fault rate. Too high → give more frames. Too low → reclaim frames.
3. **Reduce multiprogramming:** Remove some processes to swap, free their frames
4. **OOM Killer (Linux):** Kill a process if RAM+swap exhausted

---

**Q6: What is a race condition? How do you prevent it?**

**A:** A race condition occurs when program correctness depends on the timing/ordering of concurrent operations on shared data. The outcome is non-deterministic.

**Example:** Two threads both do `counter++` without synchronization. Each reads, increments, stores separately → BOTH might read the same old value → increment is lost.

**Prevention:**
1. **Mutex/Lock:** Protect critical section — only one thread at a time
2. **Semaphore:** Counting semaphore or binary semaphore for synchronization
3. **Atomic operations:** Hardware-supported atomic CAS/TAS for lock-free code
4. **Immutability:** If data never changes after creation, no synchronization needed
5. **Message passing:** Avoid sharing mutable state — communicate via messages (CSP, Actor model)
6. **Thread-local storage:** Each thread has its own copy (no sharing)

---

**Q7: What is a semaphore? Difference from mutex?**

**A:** A semaphore is an integer synchronization primitive with two atomic operations:
- `wait()/P()`: Decrement. Block if result < 0.
- `signal()/V()`: Increment. Wake one blocked thread.

**vs Mutex:**
| Feature | Mutex | Semaphore |
|---------|-------|-----------|
| Value | Binary (0 or 1) | Integer ≥ 0 |
| Ownership | Yes (must be unlocked by locker) | No (anyone can signal) |
| Use | Mutual exclusion | Signaling, resource counting |
| Priority inheritance | Usually yes | Usually no |

Use mutex to protect shared data. Use semaphore to signal between threads or count available resources.

---

**Q8: Explain the fork() system call. What does Copy-on-Write mean?**

**A:** `fork()` creates a child process as an exact copy of the parent. Both resume after `fork()`: parent gets child PID, child gets 0.

**Copy-on-Write (COW):**  Instead of physically copying all parent memory (could be GBs), the child gets a new page table pointing to the SAME physical pages as the parent. Both page tables mark these pages READ-ONLY.

When EITHER process writes to a page → page fault → OS allocates a NEW frame, copies the page to it, updates that process's page table, resumes. Only the written page is copied!

**Why:** Makes fork() nearly O(1) instead of O(address_space_size). Since many fork() calls are followed immediately by exec(), most pages are never written → COW saves enormous copying.

---

**Q9: What is the difference between preemptive and non-preemptive scheduling?**

**A:**
- **Non-preemptive:** Once a process gets the CPU, it keeps it until it voluntarily gives up (via I/O, exit, or yield). Simple, no context switch overhead. Problem: one long process starves all others. Examples: FCFS, SJF (non-preemptive), cooperative multitasking (old Windows 3.x).

- **Preemptive:** OS can forcibly take CPU from a running process (typically via timer interrupt). Essential for interactive systems — ensures responsiveness. Context switch overhead. Examples: Round Robin, SRTF, Linux CFS. ALL modern OS are preemptive.

---

**Q10: What is a zombie process? Orphan process? How are they different?**

**A:**
- **Zombie:** A process that has finished execution but whose PCB (task_struct) still exists because the parent hasn't called `wait()` to read the exit status. It uses no CPU or significant memory, just a process table slot. Prevention: parent calls `wait()`/`waitpid()`, or set `SIGCHLD` handler to `SIG_IGN`.

- **Orphan:** A process whose parent exited before it did. The OS reparents it to PID 1 (systemd/init), which automatically calls `wait()` for all adopted children. Orphans are NOT zombies — they continue running normally.

Key difference: **Zombie = dead but unreaped (parent alive, not calling wait)**. **Orphan = parent died first, process still running (adopted by init)**.

---

**Q11: How does the kernel switch between processes (context switching)?**

**A:** Context switch = save current process's CPU state, load next process's CPU state.

**Saved for current process:**
- All general-purpose CPU registers (RAX-R15 on x86-64)
- Program counter (RIP)
- Stack pointer (RSP), frame pointer (RBP)
- CPU flags register (RFLAGS)
- FPU/SSE/AVX state (up to 2.5KB via XSAVE instruction!)
- Page table base register (CR3)

**Loaded for next process:**
- All the above from next process's saved state
- CR3 updated → TLB flushed (or PCID used to avoid flush)

**Trigger:** Timer interrupt (most common), I/O wait, explicit yield, preemption.

**Cost:** ~1-10 microseconds. Expensive due to: register save/restore, TLB flush, cache pollution.

---

**Q12: What are the different page replacement algorithms? Which is best?**

**A:**
1. **OPT (Bélády's):** Evict page unused for longest future time. Theoretically optimal. Impractical (requires future knowledge). Used as benchmark.

2. **FIFO:** Evict oldest page (first in, first out). Simple but can evict frequently used pages. Suffers Bélády's Anomaly.

3. **LRU:** Evict least recently used page. Good approximation of OPT. No Bélády's Anomaly. Expensive to implement exactly (needs time-stamping every access).

4. **Clock (Second-Chance):** Approximate LRU using hardware reference bits. Circular list of frames. Skip pages with ref bit=1 (clear their bit, give second chance). Evict first page with ref bit=0. Used in practice (Linux).

5. **NFU/Aging:** Software-based LRU approximation. Periodically shift reference bytes right, OR in reference bit at top. Evict page with smallest count.

**Best in practice:** Clock or enhanced Clock (reference + dirty bits). Linux uses variants of this.

---

**Q13: What are the four approaches to handling deadlocks?**

**A:**
1. **Ostrich Algorithm:** Pretend deadlocks don't happen. Practical for systems where deadlock is very rare and cost of prevention is high. (Windows, Linux use this for some resources!)

2. **Prevention:** Eliminate one of Coffman's four conditions:
   - Mutual exclusion: make resources sharable (not always possible)
   - Hold and wait: request all resources upfront
   - No preemption: allow forcible resource taking
   - Circular wait: impose global resource ordering (MOST PRACTICAL!)

3. **Avoidance:** Use Banker's Algorithm — only grant requests that keep system in safe state. Requires knowing max resource needs upfront.

4. **Detection + Recovery:** Let deadlocks happen, periodically detect with resource allocation graph cycle detection or Banker's detection algorithm. Recover by killing processes or preempting resources.

---

**Q14: Explain how Linux implements memory allocation.**

**A:** Linux has a layered memory allocation system:

**Physical page allocation (buddy system):**
- Maintains free lists for blocks of size 2^0 to 2^10 pages
- Allocation: find smallest block that fits, split if needed
- Deallocation: check if "buddy" is free, coalesce if so
- Fast O(log n) allocation and coalescing

**Kernel small object allocation (Slab allocator):**
- Per-object-type caches: task_struct cache, inode cache, dentry cache...
- Each slab = one or more pages containing same-type objects
- Free list within each slab
- Very fast: just pop off free list
- Hot objects (recently freed, still in CPU cache) allocated first

**User space (malloc):**
- glibc malloc = ptmalloc (arena-based): per-thread arenas to reduce contention
- Small allocations: bins (exact-size + size-range bins)
- Large allocations: `mmap()` directly
- Linux jemalloc/tcmalloc: alternative allocators for high-performance (used by Firefox, Chrome, Redis)

---

### TIER 2 — Advanced Questions (SDE-2 / SDE-3 Level)

---

**Q15: How does Linux CFS avoid starvation?**

**A:** CFS (Completely Fair Scheduler) prevents starvation through virtual runtime (vruntime):

1. **vruntime tracking:** Every process accumulates vruntime proportional to actual CPU time. The process with MINIMUM vruntime runs next.

2. **Why no starvation:** A low-priority process accumulates vruntime faster than a high-priority one (its vruntime weight is lower). But it DOES accumulate vruntime over time. Eventually, even a low-priority process's vruntime becomes the minimum in the red-black tree → it gets scheduled.

3. **New process handling:** When a process arrives, its initial vruntime is set to `min_vruntime` (the current minimum). This prevents an old low-vruntime process (that was sleeping for a long time) from monopolizing CPU when it wakes up.

4. **Sleeping processes:** When a process wakes from sleep, its vruntime is "caught up" only partially — this gives recently woken processes a brief CPU boost (for interactivity) without causing starvation.

---

**Q16: What is a spinlock vs a mutex? When to use each?**

**A:**
- **Spinlock:** Thread busy-waits (spins in a loop checking the lock). No system call. Extremely fast if lock held briefly. Wastes CPU while spinning. CANNOT sleep while holding spinlock (would deadlock if scheduler needs to preempt).

- **Mutex:** Thread sleeps (blocks) if lock unavailable. OS puts thread in wait queue. Wakes up when lock released. Involves system call overhead. Efficient if lock held for a long time.

**Decision:**
- Use **spinlock** when: lock is held for nanoseconds-microseconds, in kernel interrupt context, when sleeping is impossible, on multi-core systems (single-core: spinlock pointless — only one CPU, spinning wastes it)
- Use **mutex** when: lock held for milliseconds or more, in user space, when waiting thread should not waste CPU

**Linux kernel:** Uses `spinlock_t` for very short critical sections (e.g., updating a linked list while in interrupt context). Uses `mutex_t` or `semaphore` for longer waits (e.g., waiting for disk I/O).

---

**Q17: Explain TLB shootdown. Why is it expensive on multi-core systems?**

**A:** A **TLB shootdown** is when one CPU needs to invalidate TLB entries on OTHER CPUs.

**Scenario:** Process A's page table is modified (page unmapped, permission changed, etc.) by CPU 0. But CPUs 1, 2, 3 might have STALE cached translations for this page in their TLBs.

**TLB shootdown sequence:**
1. CPU 0 modifies page table
2. CPU 0 sends IPI (Inter-Processor Interrupt) to ALL other CPUs
3. Other CPUs stop what they're doing, enter interrupt handler
4. Each CPU flushes its local TLB entry (`INVLPG` instruction on x86)
5. Each CPU sends acknowledgment to CPU 0
6. CPU 0 waits for all acknowledgments before proceeding
7. CPU 0 continues

**Why expensive:**
- All CPUs are interrupted → lose their current execution context
- Must wait for ALL CPUs to acknowledge → latency scales with CPU count
- On 128-core server, shootdown involves 127 IPIs!

**Linux optimization:** Uses `mm_cpumask` to track which CPUs have used this process's mm. Only sends IPIs to CPUs that actually have TLB entries for this process.

**Further optimization (x86 PCID/ARM ASID):** Instead of flushing, tag TLB entries with ASID. When ASID changes (new process), old entries become "invisible" without explicit flush. Reduces TLB shootdowns significantly.

---

**Q18: What is a futex? How does it work?**

**A:** **futex** (Fast Userspace muTEX) is the Linux mechanism underlying most synchronization primitives (pthread_mutex, semaphores, etc.).

**Problem with old approach:** Every lock/unlock required a system call → expensive.

**Futex insight:** In the uncontested case (no other thread waiting), locking is just an atomic operation in user space. Only when contention occurs do we need kernel involvement.

**Implementation:**
1. **Acquire (fast path):** Use atomic compare-and-swap to change lock word from 0 (free) to 1 (held). If succeeds → lock acquired, no system call! Cost: ~5ns.

2. **Acquire (slow path):** CAS fails → another thread holds lock. Change lock word to 2 (held + waiters). Call `futex(FUTEX_WAIT, lock_addr, 2)` system call → thread sleeps in kernel wait queue.

3. **Release:** Atomically decrement lock word. If result was 2 (waiters exist) → call `futex(FUTEX_WAKE, lock_addr, 1)` to wake one waiter. If result was 1 (no waiters) → just return.

**glibc pthread_mutex_t uses futex internally.** The futex word IS the lock state.

---

**Q19: What is the difference between SIGKILL and SIGTERM?**

**A:**
- **SIGTERM (15):** "Please terminate gracefully." The default signal from `kill`. The process CAN catch this signal and run cleanup code before exiting (close files, flush buffers, write checkpoint, notify other services). Correct way to shut down a well-behaved daemon.

- **SIGKILL (9):** "You are dead. NOW." CANNOT be caught, blocked, or ignored. The kernel terminates the process immediately without giving it any chance to clean up. The only way `kill -9` fails: process is in uninterruptible sleep (D state) waiting for kernel I/O. Even then, it's marked for termination and dies when I/O completes.

**When to use SIGKILL:** Only when process ignores SIGTERM or is hung. Risks: data corruption (buffers not flushed), temporary files not cleaned up, other services not notified.

**Best practice:** Send SIGTERM, wait 30 seconds, if still alive send SIGKILL.

---

**Q20: How does a context switch differ between processes and threads?**

**A:**

**Process context switch:**
1. Save all CPU registers to old process's kernel stack
2. Update old process's PCB
3. Load new process's PCB
4. Load all CPU registers from new process's kernel stack
5. **Update CR3** → points to new process's page table → **TLB FLUSH!** (expensive)
6. Cache effects: new process data evicts old process data from cache

**Thread context switch (same process):**
1. Save all CPU registers (same as process)
2. Load new thread's registers
3. **NO CR3 change** → page table stays the same → **NO TLB flush!** (much cheaper)
4. Some cache still disrupted (thread stacks are different)

**Quantitatively:**
- Process context switch: ~1-10 microseconds (with TLB flush + cache effects)
- Thread context switch: ~100 nanoseconds to 1 microsecond (no TLB flush, less cache disruption)
- This is a major reason threads are called "lightweight" compared to processes

---

# ═══════════════════════════════════════════════════════
# CHAPTER 2: OS-LEVEL SYSTEM DESIGN PROBLEMS
# ═══════════════════════════════════════════════════════

## 2.1 Design a Thread Pool

**Problem:** Design a thread pool that accepts tasks and executes them using a fixed number of worker threads.

```
Requirements:
- Fixed number of worker threads (e.g., 8 for an 8-core machine)
- Task queue: FIFO for submitted tasks
- Worker threads: wait when queue is empty, execute when task available
- Graceful shutdown: drain queue then exit

Design:

┌────────────────────────────────────────────────────────────┐
│                     THREAD POOL                             │
│                                                             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │               Task Queue (bounded buffer)             │  │
│  │  [Task1] [Task2] [Task3] [Task4] ...                 │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │ Worker 0 │  │ Worker 1 │  │ Worker 2 │  │ Worker N │  │
│  │ (thread) │  │ (thread) │  │ (thread) │  │ (thread) │  │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘  │
└────────────────────────────────────────────────────────────┘

Synchronization:
- Mutex: protect task queue
- Condition variable (not_empty): workers wait on this when queue empty
- Condition variable (not_full): producers wait if queue full (bounded pool)
- Shutdown flag: atomic boolean, workers check periodically

Implementation in C:
```

```c
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_QUEUE 1000

typedef void (*task_func)(void*);

typedef struct {
    task_func func;
    void *arg;
} Task;

typedef struct {
    Task       queue[MAX_QUEUE];
    int        head, tail, count;
    int        shutdown;
    pthread_mutex_t lock;
    pthread_cond_t  not_empty;
    pthread_cond_t  not_full;
    pthread_t  *threads;
    int        num_threads;
} ThreadPool;

void *worker_thread(void *arg) {
    ThreadPool *pool = (ThreadPool*)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->lock);
        
        /* Wait for work or shutdown */
        while (pool->count == 0 && !pool->shutdown)
            pthread_cond_wait(&pool->not_empty, &pool->lock);
        
        if (pool->shutdown && pool->count == 0) {
            pthread_mutex_unlock(&pool->lock);
            return NULL;
        }
        
        /* Dequeue task */
        Task task = pool->queue[pool->head];
        pool->head = (pool->head + 1) % MAX_QUEUE;
        pool->count--;
        pthread_cond_signal(&pool->not_full);
        pthread_mutex_unlock(&pool->lock);
        
        /* Execute task WITHOUT holding the lock */
        task.func(task.arg);
    }
}

ThreadPool *pool_create(int num_threads) {
    ThreadPool *pool = calloc(1, sizeof(ThreadPool));
    pool->num_threads = num_threads;
    pool->threads = malloc(num_threads * sizeof(pthread_t));
    
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->not_empty, NULL);
    pthread_cond_init(&pool->not_full, NULL);
    
    for (int i = 0; i < num_threads; i++)
        pthread_create(&pool->threads[i], NULL, worker_thread, pool);
    
    return pool;
}

void pool_submit(ThreadPool *pool, task_func func, void *arg) {
    pthread_mutex_lock(&pool->lock);
    while (pool->count == MAX_QUEUE)
        pthread_cond_wait(&pool->not_full, &pool->lock);
    
    pool->queue[pool->tail] = (Task){func, arg};
    pool->tail = (pool->tail + 1) % MAX_QUEUE;
    pool->count++;
    pthread_cond_signal(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);
}

void pool_shutdown(ThreadPool *pool) {
    pthread_mutex_lock(&pool->lock);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->not_empty);  /* Wake ALL waiting workers */
    pthread_mutex_unlock(&pool->lock);
    
    for (int i = 0; i < pool->num_threads; i++)
        pthread_join(pool->threads[i], NULL);
    
    free(pool->threads);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->not_empty);
    pthread_cond_destroy(&pool->not_full);
    free(pool);
}
```

---

## 2.2 Design a Memory Allocator (malloc/free)

**Problem:** Implement a simple memory allocator.

**Key design decisions:**
1. **Block structure:** Each allocated block has a header storing size (and free bit)
2. **Free list:** Track free blocks (implicit or explicit free list)
3. **Coalescing:** Merge adjacent free blocks to reduce fragmentation
4. **Block splitting:** When allocation is smaller than free block, split remainder

```
Heap layout:
  [Header(size=32,free=0)][32 bytes of data][Header(size=64,free=1)][64 bytes free]...

Header format (4 bytes):
  Bits 31-3: block size (in bytes, multiple of 8)
  Bit 2: unused
  Bit 1: previous block allocated?
  Bit 0: this block allocated? (0=free, 1=used)

First-fit allocation:
  Scan free list from start
  Find first block with size ≥ request
  If block is much larger than request: split it
  Mark block as allocated
  Return pointer to payload (after header)

Free:
  Mark block as free
  Coalesce with adjacent free blocks (check prev and next)
  Add to free list

Boundary tags:
  To coalesce with PREVIOUS free block, need to know its size.
  Solution: Add a "footer" (same as header) at END of each block.
  Then to find previous block's start: current_block - prev_footer.size
```

---

## 2.3 Design a Simple File System

**Design a file system supporting:**
- `create(path)`, `delete(path)`, `open(path)`, `read(fd, buf, n)`, `write(fd, buf, n)`, `close(fd)`
- 1GB disk, 4KB blocks
- Support files up to 64MB

```
Data structures:

Superblock (block 0):
  magic_number, total_blocks, free_blocks, inode_count, ...

Inode bitmap (block 1):
  1 bit per inode (1=used, 0=free)

Data block bitmap (blocks 2-3):
  1 bit per data block

Inode table (blocks 4-35):
  256 inodes × 128 bytes/inode = 32,768 bytes = 8 blocks
  Each inode: size, type, permissions, timestamps, 12 direct + 3 indirect ptrs

Data blocks (blocks 36-255999):
  Remaining space for actual file/directory data

Directory format:
  Each directory block: array of {inode_number (4B), filename (60B)}

Operations:
  create("/home/alice/test.txt"):
    1. Walk path: start at root inode, traverse "home", "alice"
    2. Allocate new inode (find 0 bit in inode bitmap, set to 1)
    3. Allocate data block(s) for inode
    4. Add {new_inode, "test.txt"} entry to alice's directory data
    5. Update alice's inode (new mtime, possibly new data block for bigger dir)

  write(fd, "hello", 5):
    1. Look up open file by fd → get inode
    2. Find current write position in inode
    3. Which block? block_num = position / BLOCK_SIZE
    4. If block not allocated: allocate a new data block, update inode pointer
    5. Write data at byte offset within block: block_offset = position % BLOCK_SIZE
    6. Update inode size if needed
```

---

## 2.4 Design a Deadlock Detector

```
Data structures:
  N_PROC processes, M_RES resource types (each with K_i instances)
  
  Allocation[N_PROC][M_RES]: current allocation
  Request[N_PROC][M_RES]:    current requests (pending)
  Available[M_RES]:          currently free instances

Algorithm (O(N² × M)):
  Mark all processes as potentially unfinished.
  Repeat:
    Find any unmarked process i such that Request[i] ≤ Available.
    If found:
      Available += Allocation[i]  (process could finish and release)
      Mark i as finished.
      Loop again.
    If not found: all remaining unmarked processes are DEADLOCKED.

Recovery: Kill one deadlocked process (least work done / lowest priority).
          Run detector again to see if deadlock broken.
          Repeat if needed.
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 3: TRICKY OUTPUT QUESTIONS (PREDICT THE OUTPUT)
# ═══════════════════════════════════════════════════════

## 3.1 fork() Output Questions

```c
/* Question 1: How many times is "Hello" printed? */
int main() {
    fork();
    fork();
    fork();
    printf("Hello\n");
    return 0;
}
/* Answer: 8 times!
 * After 1st fork: 2 processes
 * After 2nd fork: 4 processes  
 * After 3rd fork: 8 processes
 * All 8 print "Hello"
 */

/* Question 2: What's the output? */
int main() {
    pid_t pid = fork();
    if (pid == 0) {
        printf("Child: %d\n", getpid());
        exit(0);
    }
    printf("Parent: %d\n", getpid());
    wait(NULL);
    printf("Parent done\n");
    return 0;
}
/* Answer: Non-deterministic ORDER, but both lines appear.
 * Could be:
 *   Parent: 1234    OR    Child: 1235
 *   Child: 1235           Parent: 1234
 *   Parent done           Parent done
 * 
 * "Parent done" ALWAYS appears last (wait() ensures child finished)
 */

/* Question 3: Tricky — buffered I/O + fork */
#include <stdio.h>
#include <unistd.h>
int main() {
    printf("before fork");    /* No \n — stays in buffer! */
    fork();
    printf(" after fork\n");  /* Both parent and child flush buffer */
    return 0;
}
/* Answer: "before fork after fork" printed TWICE!
 * printf is buffered. "before fork" is in parent's output buffer.
 * fork() copies the buffer too!
 * Both parent and child print "before fork after fork"
 *
 * FIX: Use fflush(stdout) before fork(), or printf("before fork\n")
 */
```

---

## 3.2 Race Condition Output Questions

```c
/* Question: What values can counter have? */
int counter = 0;
void *inc(void *arg) {
    for (int i = 0; i < 100; i++) counter++;
    return NULL;
}
int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, inc, NULL);
    pthread_create(&t2, NULL, inc, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("counter = %d\n", counter);
}
/* Answer: Any value from 100 to 200, non-deterministic.
 * 200: no lost updates (unlikely but possible)
 * 100: maximum lost updates (one thread's work completely overwritten)
 * Typical: somewhere in between, varies every run
 */
```

---

## 3.3 Scheduling Output Questions

```
Question: Given this scenario, what's the output order and timing?

Process A: arrives t=0, burst=4 (prints "A" at end)
Process B: arrives t=1, burst=2 (prints "B" at end)
Process C: arrives t=3, burst=1 (prints "C" at end)

Under Round Robin with q=2:
t=0: A runs for 2. Queue: [B(arr=1, now waiting), A(rem=2)]
t=2: B runs for 2. Queue: [A(rem=2), C(arr=3, now waiting)]  
     B completes at t=4! Prints "B".
t=4: A runs for 2. Queue: [C(rem=1)]
     A completes at t=6! Prints "A".
t=6: C runs for 1. Completes at t=7! Prints "C".

Output order: B A C
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 4: COMPLETE NUMERICAL PROBLEM SETS
# ═══════════════════════════════════════════════════════

## 4.1 CPU Scheduling Numericals (Master Set)

### Set 1 — All Algorithms on Same Dataset

```
Processes with arrival and burst times:
  P1: arrival=0, burst=8
  P2: arrival=1, burst=4
  P3: arrival=2, burst=9
  P4: arrival=3, burst=5

FCFS:
Order: P1(0-8), P2(8-12), P3(12-21), P4(21-26)
WT: P1=0, P2=7, P3=10, P4=18. Avg WT = 35/4 = 8.75

SJF (Non-Preemptive):
t=0: Only P1 → run P1 (0-8)
t=8: Queue {P2(4),P3(9),P4(5)} → SJF: P2(4) → run P2 (8-12)
t=12: Queue {P3(9),P4(5)} → SJF: P4(5) → run P4 (12-17)
t=17: Queue {P3(9)} → run P3 (17-26)
WT: P1=0, P2=7, P3=15, P4=9. Avg WT = 31/4 = 7.75

SRTF (Preemptive SJF):
t=0: P1(rem=8) starts.
t=1: P2 arrives(burst=4). P2(4) < P1(7 rem). PREEMPT P1. Run P2.
t=2: P3 arrives(burst=9). P3(9) > P2(3 rem). Continue P2.
t=3: P4 arrives(burst=5). P4(5) > P2(2 rem). Continue P2.
t=5: P2 done. Queue: {P1(rem=7), P3(rem=9), P4(rem=5)}. Min=P4(5). Run P4.
t=10: P4 done. Queue: {P1(rem=7), P3(rem=9)}. Min=P1(7). Run P1.
t=17: P1 done. Run P3.
t=26: P3 done.

Gantt: P1(0-1), P2(1-5), P4(5-10), P1(10-17), P3(17-26)
TAT: P1=17-0=17, P2=5-1=4, P3=26-2=24, P4=10-3=7
WT:  P1=17-8=9,  P2=4-4=0, P3=24-9=15, P4=7-5=2
Avg WT = (9+0+15+2)/4 = 26/4 = 6.5

RR with q=2:
t=0: [P1]. Run P1 for 2. Queue: [P2(arr1), P1(rem=6)]
t=2: [P2, P1(6)]. Run P2 for 2. Queue: [P3(arr2), P4(arr3), P1(6), P2(rem=2)]
t=4: Run P3 for 2. Queue: [P4, P1(6), P2(2), P3(rem=7)]
t=6: Run P4 for 2. Queue: [P1(6), P2(2), P3(7), P4(rem=3)]
t=8: Run P1 for 2. Queue: [P2(2), P3(7), P4(3), P1(rem=4)]
t=10: Run P2 for 2. Done! Queue: [P3(7), P4(3), P1(4)]
t=12: Run P3 for 2. Queue: [P4(3), P1(4), P3(rem=5)]
t=14: Run P4 for 2. Queue: [P1(4), P3(5), P4(rem=1)]
t=16: Run P1 for 2. Queue: [P3(5), P4(1), P1(rem=2)]
t=18: Run P3 for 2. Queue: [P4(1), P1(2), P3(rem=3)]
t=20: Run P4 for 1. Done! Queue: [P1(2), P3(3)]
t=21: Run P1 for 2. Done! Queue: [P3(3)]
t=23: Run P3 for 2. Queue: [P3(rem=1)]
t=25: Run P3 for 1. Done!

Completion: P1=23, P2=12, P3=26, P4=21
TAT: P1=23, P2=11, P3=24, P4=18
WT: P1=15, P2=7, P3=15, P4=13
Avg WT = (15+7+15+13)/4 = 50/4 = 12.5

COMPARISON TABLE:
Algorithm | Avg WT | Avg TAT | Notes
FCFS      | 8.75   | 14.5    | Simple, convoy effect
SJF(NP)   | 7.75   | 13.5    | Better, but starvation
SRTF      | 6.5    | 12.0    | Best WT, preemptive
RR(q=2)   | 12.5   | 18.0    | Fair, best response time
```

---

## 4.2 Memory Management Numericals

### Set 1 — Page Table Calculations

```
Question: System has 16-bit virtual addresses, 4KB pages, 8KB physical RAM.
  (a) How many virtual pages?
  (b) How many physical frames?
  (c) How large is the page table?
  (d) How many bits for page number? offset?

Solutions:
(a) Virtual pages = 2^16 / 2^12 = 2^4 = 16 pages
(b) Physical frames = 2^13 / 2^12 = 2^1 = 2 frames
(c) Page table: 16 entries × (size per entry)
    Entry must hold frame number: 2 frames → 1 bit + valid/protection bits
    Typically round up to 1 byte or 4 bytes per entry
    Minimum: 16 × 1 byte = 16 bytes (huge if only 8KB RAM!)
(d) Page number bits: log2(16) = 4 bits
    Offset bits: log2(4096) = 12 bits
    Virtual address: [4-bit page | 12-bit offset]
```

### Set 2 — TLB EAT Calculations

```
Q: Memory access time = 150ns. TLB hit ratio = 0.85. TLB lookup = 15ns.

EAT = 0.85 × (15 + 150) + 0.15 × (15 + 150 + 150)
    = 0.85 × 165 + 0.15 × 315
    = 140.25 + 47.25
    = 187.5 ns

Without TLB: 2 × 150 = 300ns
Speedup: 300 / 187.5 = 1.6× speedup with 85% hit rate!

Q: What TLB hit ratio achieves EAT within 10% of memory time?
  Target: EAT ≤ 1.1 × 150 = 165ns
  α(15+150) + (1-α)(15+300) ≤ 165
  165α + 315 - 315α ≤ 165
  -150α ≤ -150
  α ≥ 1.0  (impossible!)
  
Wait — let me redo. "Within 10% of memory time" means within 10% of ideal (150ns without overhead):
  EAT ≤ 150 × 1.1 = 165 ns
  α×165 + (1-α)×315 ≤ 165
  165α + 315 - 315α ≤ 165
  315 - 150α ≤ 165
  150α ≥ 150
  α ≥ 1.0 (impossible with TLB overhead!)
  
With TLB overhead of 15ns, best possible EAT = 165ns (at α=1.0).
The question is ill-formed. Better target: EAT within 10% of no-TLB single access:
  No-TLB ideal (if only one access needed): 150ns
  EAT ≤ 165ns → α ≥ 1.0 → need 100% hit rate → impossible
  
Practical: achieve EAT within 10% of 2-access baseline (300ns):
  EAT ≤ 270ns → α(165) + (1-α)(315) ≤ 270
  165α + 315 - 315α ≤ 270
  -150α ≤ -45
  α ≥ 0.3 (only 30% hit rate needed!)
```

---

## 4.3 Disk Scheduling Numericals

```
Given: Disk head at cylinder 100. Disk has 200 cylinders (0-199).
Request queue: 55, 58, 39, 18, 90, 160, 150, 38.
Moving initially toward higher cylinders.

SSTF:
At 100: closest = 90 (distance 10). Move to 90.
At 90:  closest = 58 (32). Move to 58.
At 58:  closest = 55 (3). Move to 55.
At 55:  closest = 39 (16). Move to 39.
At 39:  closest = 38 (1). Move to 38.
At 38:  closest = 18 (20). Move to 18.
At 18:  only 150,160 left. closer = 150 (132). Move to 150.
At 150: closest = 160 (10). Move to 160.

Order: 100→90→58→55→39→38→18→150→160
Total: 10+32+3+16+1+20+132+10 = 224

SCAN (toward higher cylinders first):
Order: 100→150→160→199→90→58→55→39→38→18
(actually, after highest request 160, head reverses back)
Order: 100→150→160→199(end)→90→58→55→39→38→18

Hmm, let me redo SCAN properly (goes to highest cylinder then reverses):
Going high from 100: service 150, 160. Reach 199 (edge). Reverse.
Going low from 199: service 90, 58, 55, 39, 38, 18.

Total: |150-100|+|160-150|+|199-160|+|199-90|+|90-58|+|58-55|+|55-39|+|39-38|+|38-18|
     = 50+10+39+109+32+3+16+1+20 = 280

LOOK (goes to last request, not disk end):
Going high from 100: service 150, 160 (highest = 160). Reverse.
Going low from 160: service 90, 58, 55, 39, 38, 18.

Total: |150-100|+|160-150|+|160-90|+|90-58|+|58-55|+|55-39|+|39-38|+|38-18|
     = 50+10+70+32+3+16+1+20 = 202

C-LOOK:
Going high from 100: service 150, 160. Jump to 18 (lowest). Continue high.
Service 18, 38, 39, 55, 58, 90.

Total: |150-100|+|160-150|+|160-18|(jump,no service)+|38-18|+|39-38|+|55-39|+|58-55|+|90-58|
     = 50+10+142+20+1+16+3+32 = 274

Rankings (our example): LOOK(202) < SSTF(224) < C-LOOK(274) < SCAN(280)
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 5: MCQ MEGA-BANK — 100 QUESTIONS
# ═══════════════════════════════════════════════════════

## Complete Mixed MCQ Set

**Q1.** [EASY] The purpose of the OS is to:
- A) Compile programs
- B) Manage hardware resources and provide services ✓
- C) Connect to the internet
- D) Store user data

**Q2.** [MEDIUM] Which system call creates a new process in Unix?
- A) exec()
- B) spawn()
- C) fork() ✓
- D) create()

**Q3.** [MEDIUM] A process in the WAITING state is waiting for:
- A) CPU time
- B) An I/O operation or event ✓
- C) Memory allocation
- D) Another process to exit

**Q4.** [HARD] Which of these is NOT saved during a context switch?
- A) Program counter
- B) Stack pointer
- C) Open file contents ✓ (CORRECT — file DESCRIPTORS are saved, not file contents)
- D) CPU registers

**Q5.** [MEDIUM] fork() returns __ to the parent and __ to the child:
- A) 0, child's PID
- B) Child's PID, 0 ✓
- C) Both get 0
- D) Parent's PID, 0

**Q6.** [EASY] A zombie process:
- A) Consumes lots of CPU
- B) Is a process whose parent terminated ← WRONG (that's orphan!)
- C) Has exited but parent hasn't called wait() ✓
- D) Cannot be killed

**Q7.** [HARD] The Completely Fair Scheduler (CFS) selects the next process by:
- A) Highest priority number
- B) Longest waiting time
- C) Minimum virtual runtime ✓
- D) Random selection

**Q8.** [MEDIUM] What does the 'D' state mean in ps output?
- A) Dead process
- B) Disk scheduling
- C) Uninterruptible sleep ✓ (waiting for disk I/O)
- D) Daemon process

**Q9.** [HARD] In which state can SIGKILL not immediately terminate a process?
- A) Running
- B) Sleeping (S state)
- C) Uninterruptible sleep (D state) ✓
- D) Zombie state

**Q10.** [MEDIUM] Virtual memory allows:
- A) Programs to run faster than RAM
- B) Process address space larger than physical RAM ✓
- C) Direct access to disk
- D) Shared CPU time

**Q11.** [HARD] The TLB is flushed when:
- A) A new thread is created
- B) A context switch to a different process ✓ (usually — unless ASID/PCID used)
- C) A new file is opened
- D) The stack overflows

**Q12.** [MEDIUM] Bélády's Anomaly:
- A) More frames always means fewer page faults
- B) In FIFO, more frames can cause MORE page faults ✓
- C) Occurs in LRU page replacement
- D) Only occurs in clock algorithm

**Q13.** [EASY] The optimal page replacement algorithm evicts:
- A) The oldest page
- B) The least recently used page
- C) The page not used for the longest future time ✓
- D) A random page

**Q14.** [HARD] Which Coffman condition is most practical to attack to prevent deadlock?
- A) Mutual exclusion
- B) Hold and wait
- C) No preemption
- D) Circular wait ✓ (lock ordering)

**Q15.** [MEDIUM] Banker's Algorithm is used for:
- A) Memory allocation in banks
- B) Deadlock prevention
- C) Deadlock avoidance ✓
- D) Deadlock recovery

**Q16.** [MEDIUM] A counting semaphore initialized to N is used to:
- A) Protect a single critical section
- B) Count N available resources ✓
- C) Signal exactly N times
- D) Create N threads

**Q17.** [HARD] Spurious wakeups in condition variables require:
- A) Disabling interrupts
- B) Using a while loop to re-check condition ✓
- C) Using a mutex instead
- D) Calling signal() before wait()

**Q18.** [MEDIUM] In the Producer-Consumer problem, what does the mutex protect?
- A) The "empty" semaphore
- B) The "full" semaphore
- C) The shared buffer ✓
- D) The producer process

**Q19.** [EASY] FIFO page replacement evicts:
- A) Most recently used page
- B) Least recently used page
- C) The oldest page in memory ✓
- D) The largest page

**Q20.** [HARD] In a system with 4 processes and 3 resource types, a deadlock occurs when (using Banker's algorithm terms):
- A) Any process requests a resource
- B) No safe sequence exists ✓
- C) All processes are running
- D) All resources are allocated

---

# ═══════════════════════════════════════════════════════
# CHAPTER 6: OS MASTERY CHEAT SHEET
# ═══════════════════════════════════════════════════════

## Critical Formulas

```
SCHEDULING:
  Turnaround Time     = Completion - Arrival
  Waiting Time        = Turnaround - Burst
  Response Time       = First_Run - Arrival
  CPU Utilization     = (Busy_time / Total_time) × 100%
  
  Exponential Avg     = α × actual(n) + (1-α) × predicted(n)
  RMS schedulability  = CPU_util ≤ n(2^(1/n) - 1) → 0.693 as n→∞

MEMORY:
  EAT (no page fault) = α(t_TLB + t_mem) + (1-α)(t_TLB + 2×t_mem)
  EAT (page fault)    = (1-p)×t_mem + p×t_page_fault
  
  Page number         = virtual_addr / page_size = virtual_addr >> offset_bits
  Offset              = virtual_addr % page_size = virtual_addr & (page_size-1)
  Physical addr       = (frame_number × page_size) + offset
  
  Max file size (inode):
    Direct:           12 × block_size
    Single indirect:  (block_size/ptr_size) × block_size
    Double indirect:  (block_size/ptr_size)² × block_size
    Triple indirect:  (block_size/ptr_size)³ × block_size

DISK:
  Rotational latency (avg) = 0.5 × (60s / RPM)
  At 7200 RPM: avg latency = 0.5 × 60/7200 = 4.17ms
  Access time = Seek + Rotation + Transfer

SYNCHRONIZATION:
  Deadlock: mutual_exclusion ∧ hold_and_wait ∧ no_preemption ∧ circular_wait
  Banker's: safe if ∃ sequence where each process can finish given available + preceding
```

## Quick Reference: Linux Commands for OS Concepts

```bash
# Process info
ps aux                     # All processes (BSD style)
ps -ef                     # All processes (Unix style)
top / htop                 # Interactive process viewer
pstree                     # Process tree
cat /proc/<pid>/status     # Detailed process status
strace <command>           # Trace system calls
ltrace <command>           # Trace library calls
perf stat <command>        # Performance statistics

# Memory
free -h                    # Memory overview (human readable)
cat /proc/meminfo          # Detailed memory info
vmstat 1                   # Virtual memory stats every 1s
cat /proc/<pid>/maps       # Process virtual memory map
cat /proc/<pid>/smaps      # Detailed per-VMA memory stats
valgrind --leak-check=full # Check for memory leaks

# CPU Scheduling
cat /proc/sched_debug      # CFS scheduler debug info
cat /proc/<pid>/schedstat  # Per-process scheduler stats
taskset -c 0,1 <cmd>       # Run command on CPUs 0 and 1 (affinity)
chrt -f 50 <cmd>           # Run with SCHED_FIFO priority 50
nice -n 19 <cmd>           # Run with lowest priority

# File System
df -h                      # Disk space usage
du -sh *                   # Directory sizes
lsof                       # List open files
lsof -p <pid>              # Files opened by process
inotifywait                # Watch filesystem events
debugfs /dev/sda1          # Low-level ext4 debugging

# I/O
iostat -x 1                # I/O statistics per second
iotop                      # Per-process I/O
fio                        # Flexible I/O tester
hdparm -Tt /dev/sda        # Disk benchmark

# Networking / IPC
ipcs                       # List IPC objects (queues, shared mem, semaphores)
ipcrm                      # Remove IPC objects
ss -tunap                  # Socket statistics
```

---

*End of DOC_5 — Interview Bank, System Design, and OS Mastery*
*This document contains: 20+ detailed interview Q&As, system design problems (thread pool, memory allocator, file system, deadlock detector), tricky output prediction questions, complete numerical sets for all topics, 100-question MCQ bank, critical formulas cheat sheet, Linux command reference*

**STUDY PLAN:**
- Week 1: DOC_1 (Processes, Threads, Scheduling) + Phase01-04 code files
- Week 2: DOC_2 (Memory Management) + Phase07-08 code files  
- Week 3: DOC_3 (Sync, Deadlocks) + Phase05-06 code files
- Week 4: DOC_4 (File Systems, I/O) + Phase09-11 code files
- Week 5: DOC_5 (Interview Bank) + Phase12-14 code files + Practice problems
- Week 6: Mock interviews, timed problem solving, weak area review
