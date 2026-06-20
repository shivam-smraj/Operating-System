# OS Course — Complete Study Guide

## 🎯 Goal: Crack SDE-2 / SDE-3 / FAANG-Level OS Interviews

---

## 📂 Folder Structure

```
OS/
├── README.md                          ← THIS FILE
├── Makefile                           ← Build all programs: make all
│
├── DOC_1_Process_Thread_CPU_Scheduling.md    ← 5000+ lines: theory + code
├── DOC_2_Memory_Management_Virtual_Memory.md ← 5000+ lines: paging, TLB, etc.
├── DOC_3_Synchronization_Deadlocks_IPC.md   ← 5000+ lines: mutex, deadlock
├── DOC_4_FileSystem_IO_Storage_Networking.md ← 5000+ lines: FS, disk sched
├── DOC_5_Interview_Bank_SystemDesign_OS.md  ← Interview Q&A, MCQs, design
│
├── Phase01_Intro_And_Architecture/    ← What is OS, syscalls, boot process
├── Phase02_Process_Management/        ← fork(), exec(), PCB, states
├── Phase03_Threads_And_Concurrency/   ← pthreads, thread pool, race conditions
├── Phase04_CPU_Scheduling/            ← FCFS, SJF, RR, Priority
├── Phase05_Synchronization/           ← Mutex, semaphore, dining philosophers
├── Phase06_Deadlocks/                 ← Banker's algorithm, detection
├── Phase07_Memory_Management/         ← Malloc, buddy allocator, paging
├── Phase08_Virtual_Memory/            ← Page replacement: FIFO, LRU, OPT
├── Phase09_File_Systems/              ← inodes, directory structure
├── Phase10_IO_And_Storage/            ← Disk scheduling: FCFS, SSTF, SCAN
├── Phase11_InterProcess_Communication/ ← Pipes, POSIX shared memory, MQ
├── Phase12_OS_Security_And_Protection/ ← Permissions, setuid, capabilities
├── Phase13_Interview_Questions/        ← Tricky predict-output programs
└── Phase14_OS_Design_Problems/         ← Capstone: Mini OS simulation
```

---

## 🚀 Quick Start (Linux / WSL)

```bash
# Install dependencies
sudo apt-get install gcc build-essential

# Go to OS directory
cd ~/Desktop/Nilesh/OS

# Build everything
make all

# Run a specific program
./Phase01_Intro_And_Architecture/01_what_is_os
./Phase04_CPU_Scheduling/02_fcfs_scheduler
./Phase06_Deadlocks/03_bankers_algorithm
./Phase08_Virtual_Memory/05_page_replacement_algorithms
./Phase14_OS_Design_Problems/10_complete_os_simulation

# Clean compiled files
make clean
```

---

## 📚 Recommended Study Path (6 Weeks)

### Week 1 — Foundations
- Read DOC_1 (Processes, Threads, Scheduling)
- Run Phase01 and Phase02 programs
- Study FCFS, SJF, RR schedulers (Phase04)
- Goal: Understand what a process is, fork/exec/wait, scheduling metrics

### Week 2 — Memory
- Read DOC_2 (Memory Management, Virtual Memory)
- Run Phase07 (malloc) and Phase08 (page replacement)
- Master: paging, TLB, page fault handling, Banker's algorithm
- Numerical: Calculate EAT, max file size, page table size

### Week 3 — Synchronization
- Read DOC_3 (Synchronization, Deadlocks, IPC)
- Run Phase05 (dining philosophers, semaphores) and Phase06 (Banker's)
- Master Coffman's four conditions, deadlock prevention strategies
- Implement: mutex, semaphore, producer-consumer from memory

### Week 4 — File Systems and I/O
- Read DOC_4 (File Systems, I/O, Storage)
- Run Phase09 (inodes) and Phase10 (disk scheduling)
- Numerical: Calculate disk scheduling total movement
- Understand: journaling, RAID levels, ext4 vs FAT

### Week 5 — Interview Prep
- Read DOC_5 (Interview Bank)
- Run Phase13 programs — predict output BEFORE running!
- Time yourself: 30 min per system design question
- Do all 100 MCQs, grade yourself

### Week 6 — Mock Interviews + Weak Areas
- Mock interviews with a friend/tool
- Revisit any DOC sections where MCQ scores were low
- Run Phase14 capstone — understand every line of the mini-OS
- Practice explaining concepts out loud (rubber duck debugging)

---

## 🏆 Key Concepts to Master (for FAANG interviews)

### Process/Thread
- [ ] fork() return values, COW semantics
- [ ] zombie vs orphan process
- [ ] How context switch works (registers saved/restored)
- [ ] Thread vs process: what's shared, what's private
- [ ] When to use process vs thread

### Scheduling
- [ ] FCFS, SJF, SRTF, RR, Priority: calculate WT, TAT, response time
- [ ] Starvation: what causes it, how to prevent (aging)
- [ ] Priority inversion + solutions (inheritance, ceiling)
- [ ] Linux CFS: vruntime, red-black tree

### Synchronization
- [ ] Mutex ownership vs semaphore (no ownership)
- [ ] Condition variable: ALWAYS use while loop (spurious wakeups)
- [ ] Producer-consumer solution with semaphores
- [ ] Readers-writers problem
- [ ] Dining philosophers: resource ordering solution

### Deadlocks
- [ ] Coffman's 4 conditions (MEMORIZE!)
- [ ] Banker's Algorithm: safety check + request granting
- [ ] Resource Allocation Graph: cycle = deadlock (single instance)
- [ ] Prevention: attack one of 4 conditions

### Memory
- [ ] Page table: virtual → physical address translation
- [ ] TLB: Effective Access Time (EAT) calculation
- [ ] Page fault handling sequence
- [ ] Thrashing: cause and prevention
- [ ] Buddy allocator + slab allocator (kernel)

### File Systems
- [ ] inode structure: 12 direct + indirect pointers
- [ ] Max file size calculation
- [ ] Hard link vs symlink
- [ ] Journaling: write-ahead log, crash recovery

### Disk Scheduling
- [ ] FCFS, SSTF, SCAN, LOOK, C-LOOK: calculate total head movement
- [ ] SSTF starvation, LOOK no starvation
- [ ] RAID 0/1/5/6/10: capacity, reliability, performance

---

## 🎯 Top 10 Interview Questions

1. Difference between process and thread?
2. What is a deadlock? Four necessary conditions?
3. How does virtual memory work?
4. What is thrashing? How to prevent it?
5. Banker's Algorithm — is this state safe? (given table)
6. Producer-Consumer problem — write the solution
7. What is a race condition? Give an example and fix it.
8. FCFS vs SJF vs Round Robin — tradeoffs?
9. How does fork() work? What is Copy-on-Write?
10. Difference between mutex and semaphore?

---

## ⚡ Quick Reference

| Algorithm | Starvation? | Preemptive? | Best for |
|-----------|------------|-------------|----------|
| FCFS | No | No | Batch |
| SJF | YES | No | Minimize WT |
| SRTF | YES | Yes | Minimize WT |
| RR | No | Yes | Time-sharing |
| Priority | YES | Both | Real-time |
| CFS (Linux) | No | Yes | General purpose |

| IPC Method | Speed | Persistence | Multi-process |
|-----------|-------|-------------|---------------|
| Shared Memory | Fastest | Process lifetime | Yes |
| Pipe | Fast | Process lifetime | Parent-child only |
| Named Pipe | Fast | Until unlink | Any |
| Message Queue | Medium | Until unlink | Yes |
| Socket | Slowest | Varies | Yes (network too) |

---

*"An expert is someone who has made all the mistakes in a field. Study these programs, break them, fix them, and you'll be one too."*

---

**Total course material:**
- 5 massive documentation files (DOC_1 to DOC_5)
- 14 phase directories with C programs
- 100+ MCQ questions
- 50+ interview Q&A pairs
- Multiple worked numerical examples per topic
