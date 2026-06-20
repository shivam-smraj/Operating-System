# DOC 1 — PROCESSES, THREADS & CPU SCHEDULING
## Complete OS Interview Preparation | SDE-2 / SDE-3 / FAANG Level
### Author: OS Mastery Course | Version 1.0 | June 2026

---

> **HOW TO USE THIS DOCUMENT:**
> Read sequentially. Every concept builds on the previous.
> For each section: read theory → study diagrams → trace code → solve MCQs → solve numericals.
> Time estimate: 40–60 hours for complete mastery.

---

# ═══════════════════════════════════════════════════════
# CHAPTER 1: INTRODUCTION TO OPERATING SYSTEMS
# ═══════════════════════════════════════════════════════

---

## 1.1 What is an Operating System?

### THEORY BLOCK

An **Operating System (OS)** is the most fundamental software running on a computer. It sits between the hardware (CPU, memory, disks, keyboard, screen) and the applications you write and run. Without an OS, every application would need to know exactly how to talk to every possible piece of hardware — an impossibility.

Here are **5 different perspectives** on what an OS is:

#### Perspective 1: Resource Manager
The OS is a **traffic controller** for hardware resources. It decides: which program gets CPU time, how much memory each program gets, which program can read the disk right now. Just like a city government allocates water, electricity, and roads among citizens, the OS allocates CPU, memory, and I/O among processes.

#### Perspective 2: Virtual Machine / Abstraction Layer
The OS creates an **illusion** for each program. Each program thinks it has:
- The entire CPU to itself (virtualized CPU via scheduling)
- The entire memory to itself (virtualized memory via virtual memory)
- Its own private file system view (via VFS)

Without this illusion, every program would have to cooperate manually — chaos!

#### Perspective 3: Government / Law Enforcer
The OS enforces **laws**. It says: "Program A cannot read Program B's memory." "Only privileged programs can access hardware directly." It uses hardware mechanisms (memory protection, privilege levels) to enforce these rules. Violating them causes a segfault or a system crash.

#### Perspective 4: Referee / Arbitrator
When two programs both want to write to the same disk at the same time, who wins? The OS is the **referee**. It queues requests, resolves conflicts, prevents one program from starving others.

#### Perspective 5: Standard Interface (POSIX / WinAPI)
The OS provides a **stable API** so your C program compiled in 2020 still runs on hardware built in 2026. The OS abstracts hardware differences away. `open("file.txt", O_RDONLY)` works whether the file is on an HDD, SSD, NFS share, or RAM disk.

---

### What Would Happen WITHOUT an OS?

Imagine writing a C program that plays a video:
- You would need to write your own disk driver to read the video file
- You would need to write your own GPU driver to display frames
- You would need to write your own network stack to download the video
- You would need to manually manage memory so other programs don't overwrite yours
- You would need to write your own scheduler so music can play while video plays
- You would need to write your own security so another program can't steal your login cookies

This was actually the situation in **1940s–1950s computing**. Programmers would physically wire the machine for their program, run it, then the next programmer would re-wire. The OS was invented to automate this management.

---

### Historical Evolution

```
1940s: No OS — programs run on bare hardware, one at a time
       Operator manually loads punch cards, no multitasking

1950s: Batch Systems — jobs collected, run sequentially
       Resident Monitor: first primitive OS, loads next job automatically
       Problem: CPU idle while waiting for tape/card reader (I/O)

1960s: Multiprogramming — multiple jobs in memory simultaneously
       When Job A waits for I/O, CPU switches to Job B
       IBM OS/360: first major general-purpose OS
       Problem: response time still slow for interactive use

1965: Time-Sharing — multiple users share one machine (CTSS, Multics)
      Each user gets a short time slice, feels like dedicated machine
      UNIX born at Bell Labs in 1969 (simplified Multics)

1980s: Personal Computing — OS for single users (DOS, Mac OS, Windows)
       No memory protection (MS-DOS), then protected mode (Windows 3.x)
       
1991: Linux — Linus Torvalds releases Linux 0.01 (10,000 lines)
      Free, open-source UNIX-like OS for x86 PCs
      
1990s-2000s: SMP, clusters, internet — OS handles multi-CPU,
             networking baked in, security becomes critical

Today: Containerization (Docker/Kubernetes), cloud VMs (KVM/Xen),
       mobile (Android/iOS), IoT (FreeRTOS), real-time systems
```

---

### Types of Operating Systems

| Type | Description | Examples | Key Characteristic |
|------|-------------|----------|--------------------|
| **Batch** | Jobs collected and run in bulk | IBM JES, early UNIX | Throughput optimized, no interaction |
| **Time-Sharing** | Multiple users share CPU | UNIX, Linux, Windows | Fairness, response time |
| **Real-Time** | Must respond within deadline | VxWorks, FreeRTOS, QNX | Determinism, low jitter |
| **Distributed** | Multiple machines act as one | Plan 9, Amoeba, K42 | Transparency, fault tolerance |
| **Embedded** | Tightly constrained hardware | Android, iOS, RTOS | Small footprint, power efficiency |
| **Mobile** | Touch-based, battery-powered | Android (Linux kernel), iOS (XNU) | App sandboxing, power management |
| **Hypervisor** | Manages multiple virtual machines | Xen, KVM, VMware ESXi | VM isolation, resource multiplexing |

---

### ASCII Diagram: OS as Layer Cake

```
┌─────────────────────────────────────────────────────────────────┐
│                    USER APPLICATIONS                             │
│         Firefox, VS Code, Python, MySQL, nginx...               │
├─────────────────────────────────────────────────────────────────┤
│                    STANDARD LIBRARIES                            │
│      glibc (printf, malloc, pthread...), libstdc++, OpenSSL     │
├─────────────────────────────────────────────────────────────────┤
│                  SYSTEM CALL INTERFACE                           │
│    open() read() write() fork() mmap() socket() ioctl()...      │
├───────────────────────────┬─────────────────────────────────────┤
│   PROCESS MANAGEMENT      │   MEMORY MANAGEMENT                 │
│   scheduler, PCB, signals │   paging, VMAs, malloc              │
├───────────────────────────┼─────────────────────────────────────┤
│   VIRTUAL FILE SYSTEM     │   NETWORK STACK                     │
│   ext4, xfs, tmpfs, proc  │   TCP/IP, socket buffer             │
├───────────────────────────┼─────────────────────────────────────┤
│              DEVICE DRIVERS                                      │
│    disk, NIC, GPU, USB, keyboard, mouse drivers                 │
├─────────────────────────────────────────────────────────────────┤
│                    HARDWARE                                      │
│         CPU, RAM, SSD/HDD, NIC, GPU, USB controllers           │
└─────────────────────────────────────────────────────────────────┘
      ↑ Ring 3 (User Mode)    ↓ Ring 0 (Kernel Mode)
      Applications run here   OS kernel runs here
```

---

### INTERVIEW BLOCK

**Q1: "Explain what an OS does in 2 minutes flat."**

**ANSWER (30-second version):**
> "An OS is a program that manages hardware resources and provides services to applications. It virtualizes the CPU through scheduling, virtualizes memory through paging, provides a file system abstraction, handles I/O through device drivers, and enforces isolation between processes. Without an OS, every application would need to directly manage hardware, which is impractical and unsafe."

**ANSWER (Full 2-minute version):**
> "Think of the OS as the management layer of a large office building. The hardware is the building itself — floors, elevators, meeting rooms, electrical outlets. The OS is building management — it decides which teams (processes) get which conference rooms (CPU time), ensures teams don't wander into each other's offices (memory isolation), provides common services like the elevator (file system), phone network (network stack), and security badges (access control).
>
> Technically, the OS does five things: (1) Process management — creates, schedules, and terminates processes; (2) Memory management — allocates RAM, implements virtual memory so each process has its own address space; (3) File system — provides persistent storage with a hierarchical name space; (4) I/O management — abstracts hardware devices through drivers; (5) Security & protection — enforces access controls and isolates processes.
>
> The OS switches between user mode and kernel mode to enforce this separation. Your program runs in user mode (Ring 3). When it needs OS services, it invokes a system call, which switches to kernel mode (Ring 0), executes the kernel code, then switches back."

---

**Q2: "Why does Linux dominate servers while Windows dominates desktops?"**

**ANSWER:**
> - **Servers:** Linux dominates (>90% of top 500 supercomputers, >70% of web servers) because: (1) Free and open-source — no license cost for 1000 VMs; (2) Extremely stable — can run years without reboot; (3) Minimal footprint — can run in 256MB RAM headless; (4) Superior networking — designed for network servers from day one; (5) Scriptable — everything is a file, perfect for automation.
>
> - **Desktops:** Windows dominates because: (1) Commercial software ecosystem (Office, Adobe, games); (2) Hardware vendor driver support historically better; (3) User-friendly UI for non-technical users; (4) Enterprise Active Directory integration; (5) Gaming: DirectX, anti-cheat compatibility.

---

**Q3: "What are the key differences between a monolithic kernel and a microkernel?"**

| Feature | Monolithic Kernel | Microkernel |
|---------|------------------|-------------|
| **Where code runs** | All in kernel space (Ring 0) | Only essential code in kernel; drivers/FS in user space |
| **Performance** | Fast (no IPC overhead) | Slower (IPC for every service) |
| **Reliability** | Bug in driver crashes whole kernel | Driver crash doesn't crash kernel |
| **Examples** | Linux, FreeBSD, Unix | L4, Minix, QNX, GNU Hurd |
| **Code size** | Large (millions of LoC) | Small (thousands of LoC) |
| **Security** | Large attack surface | Smaller attack surface |

**Interview trick:** Linux is monolithic but uses **loadable kernel modules** — you can add/remove drivers without rebooting. This gives some microkernel benefits (modular) with monolithic performance.

---

### MCQ BLOCK

**Q1.** Which of the following is NOT a function of an operating system? [EASY]
- A) Memory management
- B) Process scheduling
- C) Compiling source code ✓ (CORRECT — compilation is done by the compiler, not OS)
- D) File management

*Why A, B, D are wrong:* These are all core OS functions. The OS never compiles code — that's the job of gcc, clang, etc.

**Q2.** The OS switches from user mode to kernel mode when: [MEDIUM]
- A) A process is created
- B) A system call is made ✓ (CORRECT)
- C) A variable is declared
- D) A function returns

*Explanation:* Only system calls (and hardware interrupts, and exceptions like page faults) cause a mode switch. Regular function calls, variable declarations, process creation request from user space all happen IN user mode until the actual syscall is executed.

**Q3.** Which OS type guarantees response within a specific deadline? [EASY]
- A) Batch OS
- B) Time-sharing OS
- C) Real-Time OS ✓ (CORRECT)
- D) Distributed OS

**Q4.** In a monolithic kernel, device drivers run in: [MEDIUM]
- A) User space
- B) Kernel space ✓ (CORRECT)
- C) Hypervisor space
- D) A separate VM

**Q5.** POSIX is: [MEDIUM]
- A) A specific operating system
- B) A hardware architecture
- C) A standard interface specification for OS services ✓ (CORRECT)
- D) A file system type

*Explanation:* POSIX (Portable Operating System Interface) is an IEEE standard that defines the system call interface so that code written for one POSIX-compliant OS (Linux) runs on another (macOS, FreeBSD). It is NOT an OS itself.

**Q6.** The first OS concept that allowed multiple jobs in memory simultaneously was: [HARD]
- A) Time-sharing
- B) Multiprogramming ✓ (CORRECT)
- C) Real-time scheduling
- D) Virtual memory

*Explanation:* Multiprogramming (1960s) allowed multiple jobs in memory simultaneously, switching to another when one does I/O. Time-sharing came AFTER multiprogramming and added the idea of giving each user short time slices.

**Q7.** A hypervisor of Type 1 runs: [MEDIUM]
- A) Inside a host OS
- B) Directly on hardware ✓ (CORRECT)
- C) Inside a container
- D) In user mode

---

### CODING EXERCISE 1.1

**Problem:** Write a C program that prints information about the OS it's running on, including the process ID, parent process ID, and user ID. Also demonstrate calling a raw system call without using libc wrappers.

```c
/* Exercise 1.1: OS Information Demo
 * Compile: gcc -o os_info 01_os_info.c
 * Run: ./os_info
 */
#define _GNU_SOURCE          /* Enable GNU extensions */
#include <stdio.h>
#include <unistd.h>          /* getpid, getppid, getuid */
#include <sys/utsname.h>     /* uname() for OS info */
#include <sys/syscall.h>     /* SYS_write syscall number */

int main(void) {
    struct utsname info;    /* Structure to hold OS info */
    
    /* Get OS information via uname() system call */
    if (uname(&info) != 0) {
        perror("uname failed");
        return 1;
    }
    
    printf("=== Operating System Information ===\n");
    printf("OS Name:    %s\n", info.sysname);   /* e.g., "Linux" */
    printf("Hostname:   %s\n", info.nodename);  /* e.g., "ubuntu-server" */
    printf("Release:    %s\n", info.release);   /* e.g., "5.15.0-76-generic" */
    printf("Version:    %s\n", info.version);   /* Build date/time */
    printf("Machine:    %s\n", info.machine);   /* e.g., "x86_64" */
    
    printf("\n=== Process Information ===\n");
    printf("My PID:     %d\n", getpid());       /* My process ID */
    printf("Parent PID: %d\n", getppid());      /* Shell's PID */
    printf("User ID:    %d\n", getuid());       /* 0=root, else normal user */
    printf("Group ID:   %d\n", getgid());
    
    printf("\n=== Raw System Call Demo ===\n");
    /* Using raw syscall() instead of printf() wrapper
     * write syscall: number=1, args: fd=1(stdout), buf, len */
    const char msg[] = "This uses raw write() syscall directly!\n";
    syscall(SYS_write, 1, msg, sizeof(msg) - 1);
    
    printf("\n=== /proc/self/status excerpt ===\n");
    /* Reading our own process info from /proc filesystem */
    FILE *f = fopen("/proc/self/status", "r");
    if (f) {
        char line[256];
        int count = 0;
        while (fgets(line, sizeof(line), f) && count < 8) {
            printf("%s", line);
            count++;
        }
        fclose(f);
    }
    
    return 0;
}
/* Expected Output (approximate):
 * === Operating System Information ===
 * OS Name:    Linux
 * Hostname:   ubuntu
 * Release:    5.15.0-76-generic
 * Version:    #83-Ubuntu SMP Thu Jun  8 21:35:16 UTC 2023
 * Machine:    x86_64
 *
 * === Process Information ===
 * My PID:     1234
 * Parent PID: 1000
 * User ID:    1001
 * Group ID:   1001
 *
 * === Raw System Call Demo ===
 * This uses raw write() syscall directly!
 */
```

---

## 1.2 OS Architecture — Deep Dive

### Kernel vs User Space

The most important boundary in an OS is the **kernel/user space boundary**. Modern CPUs enforce this using **privilege levels** (Intel calls them "rings"):

```
Ring 0 (KERNEL MODE — Most Privileged)
  ├── Can execute ANY instruction (halt, cli, in/out, lidt, etc.)
  ├── Can access ANY memory address
  ├── Can modify page tables (CR3 register)
  ├── Can handle interrupts
  └── OS kernel runs here

Ring 1, 2 (rarely used today — originally for OS services/drivers)

Ring 3 (USER MODE — Least Privileged)
  ├── Cannot execute privileged instructions
  ├── Cannot access kernel memory (page table enforces this)
  ├── Cannot directly access hardware ports
  └── Your C programs run here
```

**Why this separation?**
- **Security:** A buggy or malicious program in Ring 3 cannot corrupt the kernel.
- **Stability:** A crash in user mode kills only that process, not the whole system.
- **Hardware protection:** Without Ring 0/3 separation, any program could format your disk!

**How does user code get kernel services?**
Via **system calls** — a controlled gate from Ring 3 to Ring 0. The CPU instruction `syscall` (x86-64) or `int 0x80` (older x86 32-bit) triggers the mode switch.

---

### ASCII: Complete OS Architecture

```
╔══════════════════════════════════════════════════════════════╗
║                    USER SPACE (Ring 3)                        ║
║  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    ║
║  │ Process 1│  │ Process 2│  │ Process 3│  │ Process N│    ║
║  │ (nginx)  │  │ (python) │  │ (bash)   │  │ (mysql)  │    ║
║  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘    ║
║       │              │              │              │           ║
║  ┌────┴──────────────┴──────────────┴──────────────┴────┐    ║
║  │              GNU C Library (glibc)                     │    ║
║  │  printf(), malloc(), pthread_create(), fopen()...      │    ║
║  └────────────────────────┬───────────────────────────────┘    ║
╚═══════════════════════════│══════════════════════════════════╝
                            │  syscall instruction
                            │  (crosses Ring 3 → Ring 0 boundary)
                            ▼
╔══════════════════════════════════════════════════════════════╗
║                   KERNEL SPACE (Ring 0)                       ║
║  ┌─────────────────────────────────────────────────────┐     ║
║  │          SYSTEM CALL INTERFACE / TABLE               │     ║
║  │  sys_read sys_write sys_fork sys_mmap sys_open...    │     ║
║  └──────┬──────────┬──────────┬──────────┬─────────────┘     ║
║         │          │          │          │                     ║
║  ┌──────┴──┐ ┌─────┴──┐ ┌────┴────┐ ┌───┴──────┐           ║
║  │ PROCESS │ │MEMORY  │ │  VFS   │ │ NETWORK  │           ║
║  │ MANAGER │ │MANAGER │ │LAYER   │ │  STACK   │           ║
║  │ sched   │ │ paging │ │ext4/xfs│ │ TCP/IP   │           ║
║  │ fork()  │ │ mmap() │ │ tmpfs  │ │ sockets  │           ║
║  └──────┬──┘ └─────┬──┘ └────┬────┘ └───┬──────┘           ║
║         │          │          │           │                    ║
║  ┌──────┴──────────┴──────────┴───────────┴──────────────┐   ║
║  │                   DEVICE DRIVERS                        │   ║
║  │     disk driver  |  NIC driver  |  GPU driver  |...    │   ║
║  └─────────────────────────────────────────────────────────┘   ║
╚══════════════════════════════════════════════════════════════╝
                            │
                            ▼
                     ┌──────────────┐
                     │   HARDWARE    │
                     │ CPU RAM DISK  │
                     └──────────────┘
```

---

### Kernel Types: Detailed Comparison

#### Monolithic Kernel (Linux, FreeBSD, original UNIX)

```
User Space  │  App App App
────────────┼──────────────── syscall boundary ────
Kernel Space│ [Scheduler | MM | VFS | Net | Drivers]
            │         ALL IN ONE ADDRESS SPACE
```

**Pros:**
- Fast: function calls within kernel, no message passing
- Simple interface: direct function calls between subsystems
- Well-understood: decades of optimization

**Cons:**
- Bug anywhere = kernel crash (kernel panic / BSOD)
- Hard to update a single component without kernel reboot
- Large trusted computing base (TCB)

#### Microkernel (L4, Minix 3, QNX, GNU Hurd)

```
User Space  │  App App  FileServer  NetworkServer  DiskDriver
            │  (user processes, each in own address space)
────────────┼──────── IPC message passing ──────────────────
Kernel Space│  [IPC | Memory | Scheduling]  ONLY THESE!
```

**Pros:**
- Driver crash ≠ kernel crash (driver is a user process!)
- Formally verifiable (seL4 is formally proved correct)
- Modular: replace components without rebooting

**Cons:**
- IPC overhead: every driver call = user→kernel→user→kernel (expensive!)
- Historically 10-50% slower than monolithic (though modern microkernels closing gap)
- Complex IPC debugging

#### Hybrid Kernel (Windows NT, macOS XNU)

Reality: "hybrid" is mostly marketing. Windows NT and XNU run most things in kernel mode for performance, but have a cleaner internal architecture than pure monolithic kernels.

**Windows NT Executive** runs in kernel mode:
- Object Manager, Security Monitor, Memory Manager, Process Manager
- I/O Manager, Configuration Manager, Plug and Play Manager

**macOS XNU** = Mach microkernel + BSD subsystem (both in kernel space for performance)

---

### System Calls — The Gate Between Worlds

#### What is a System Call?

A system call is how a user-space program asks the OS kernel to do something privileged. Think of it as ringing the doorbell of the OS — you can't enter directly (Ring 0 protection), but you can request service.

**System call vs library call:**
- `printf()` is a **library call** (in glibc) — runs entirely in Ring 3
- `write()` is a **system call** — transfers to Ring 0 kernel
- Note: `printf()` internally calls `write()` eventually!

#### System Call Mechanism (x86-64 Linux)

```
Step 1: User code calls printf("Hello\n")
        └── glibc printf() does formatting
        └── glibc calls write(1, "Hello\n", 6)
            └── glibc write() wrapper:
                MOV rax, 1      ; system call number for write
                MOV rdi, 1      ; arg1 = fd (stdout)
                MOV rsi, ptr    ; arg2 = buffer pointer
                MOV rdx, 6      ; arg3 = byte count
                SYSCALL         ; ← This instruction crosses Ring 3→Ring 0!

Step 2: CPU executes SYSCALL instruction:
        ├── Saves RIP (return address) in RCX
        ├── Saves RFLAGS in R11
        ├── Switches RSP to kernel stack
        ├── Changes CPL (Current Privilege Level) from 3 to 0
        └── Jumps to address in MSR_LSTAR (Linux's syscall entry point)

Step 3: Linux kernel entry_SYSCALL_64 handler:
        ├── Saves ALL user registers to kernel stack
        ├── Looks up syscall number 1 in sys_call_table[]
        ├── Calls sys_write(fd=1, buf, len=6)
        │   └── Finds file struct for fd 1
        │   └── Calls VFS write → device driver
        │   └── Returns number of bytes written
        ├── Restores user registers from kernel stack
        └── SYSRET: switches back to Ring 3, restores RIP from RCX

Step 4: User code continues after write() returns
```

#### Common Linux System Call Numbers (x86-64)

| Number | Name | Description |
|--------|------|-------------|
| 0 | `read` | Read from file descriptor |
| 1 | `write` | Write to file descriptor |
| 2 | `open` | Open a file |
| 3 | `close` | Close a file descriptor |
| 9 | `mmap` | Map file/memory into address space |
| 11 | `munmap` | Unmap memory |
| 22 | `pipe` | Create a pipe |
| 32 | `dup` | Duplicate file descriptor |
| 39 | `getpid` | Get process ID |
| 57 | `fork` | Create a child process |
| 59 | `execve` | Execute a program |
| 60 | `exit` | Terminate the process |
| 61 | `wait4` | Wait for child process |
| 102 | `getuid` | Get user ID |
| 231 | `exit_group` | Exit all threads |

---

## 1.3 OS Boot Sequence

```
POWER ON
    │
    ▼
BIOS/UEFI (firmware in ROM)
    ├── POST: Power-On Self Test (test RAM, CPU, keyboard, disk)
    ├── Detect bootable device (SSD, USB, network)
    └── Load MBR (first 512 bytes) or UEFI bootloader from EFI partition
    │
    ▼
BOOTLOADER (GRUB2, systemd-boot, rEFInd)
    ├── Stage 1: Small code in MBR, loads Stage 2
    ├── Stage 2: Full bootloader from /boot partition
    ├── Displays boot menu (choose kernel version, OS)
    ├── Loads kernel image (vmlinuz) into RAM
    ├── Loads initrd/initramfs (initial RAM disk)
    └── Passes kernel parameters (root=, quiet, nosplash, ...)
    │
    ▼
KERNEL INITIALIZATION
    ├── head.S: Very first assembly code — sets up GDT, IDT, stack
    ├── start_kernel(): Main C entry point in init/main.c
    ├── cpu_init(): Initialize per-CPU data structures
    ├── mm_init(): Initialize memory management (page tables, zones)
    ├── sched_init(): Initialize scheduler (run queues, etc.)
    ├── vfs_caches_init(): Set up virtual file system
    ├── rest_init(): Creates PID 1 (init) as kernel thread
    └── CPU_idle(): Kernel idle loop (runs when nothing else to run)
    │
    ▼
PID 1: init or systemd
    ├── Mounts root filesystem (/, /proc, /sys, /dev)
    ├── Reads /etc/fstab → mounts additional filesystems
    ├── Starts system services (network, logging, SSH, cron...)
    ├── Starts login manager (getty on TTYs, gdm/lightdm for GUI)
    └── Waits for user login
    │
    ▼
USER LOGIN
    ├── getty: Displays "login:" prompt on terminal
    ├── User types credentials
    ├── PAM: Pluggable Authentication Modules verify password
    ├── Shell (bash/zsh/fish) started for the user
    └── User has a working session!

Total time: typically 5-30 seconds (fast SSD + SSD + minimal services)
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 2: PROCESS MANAGEMENT — COMPLETE COVERAGE
# ═══════════════════════════════════════════════════════

---

## 2.1 What is a Process?

### THEORY BLOCK

A **process** is a **program in execution**. This seemingly simple definition hides enormous complexity.

**Program vs Process — the critical difference:**
```
PROGRAM: A static file on disk (e.g., /usr/bin/firefox)
         It's just bytes — instructions + data, doing nothing
         Like a recipe in a cookbook

PROCESS: The program RUNNING — in CPU, using memory, doing stuff
         It has: its own memory space, CPU registers, stack, heap
         Like a chef COOKING using the recipe
```

One program can have multiple processes:
```bash
$ ps aux | grep firefox
user  1234  ... firefox
user  5678  ... firefox   # Second window = different process!
user  9012  ... firefox   # Extension process
```

---

### Process Memory Layout

Every process has its own **virtual address space** (the illusion that it owns all memory). The layout on x86-64 Linux:

```
High addresses (0xFFFFFFFFFFFFFFFF)
┌─────────────────────────────────────┐
│         KERNEL SPACE                │  ← Inaccessible from user mode
│  (kernel code, page tables, etc.)   │  ← Process cannot read/write here
├─────────────────────────────────────┤ 0x0000 7FFF FFFF FFFF (128TB user limit)
│                                     │
│         [ unused gap ]              │
│                                     │
├─────────────────────────────────────┤
│      Stack (grows downward ↓)       │  ← Local variables, return addresses
│      ████████████████████████       │  ← Each thread has its own stack
│      (each function call pushes)    │
├─────────────────────────────────────┤
│              mmap region            │  ← Shared libraries, mmap() calls
│      (libpthread.so, libc.so...)    │
├─────────────────────────────────────┤
│              Heap (grows upward ↑)  │  ← malloc(), new — dynamic allocation
│      ████████                       │
├─────────────────────────────────────┤
│         BSS Segment                 │  ← Uninitialized global variables
│         (zero-initialized)         │  ← e.g., "int arr[1000];" at global scope
├─────────────────────────────────────┤
│         Data Segment                │  ← Initialized global/static variables
│         (int x = 42;)              │  ← e.g., "int x = 42;" at global scope
├─────────────────────────────────────┤
│         Text Segment (Code)         │  ← Your compiled instructions (READ ONLY)
│         (read-only, executable)    │  ← Protects code from accidental overwrite
├─────────────────────────────────────┤
Low addresses (0x0000000000000000 = NULL)
```

**Key insight for interviews:** The stack grows DOWN, the heap grows UP. They can meet in the middle (stack overflow = stack hits another segment).

---

### Process Control Block (PCB) — The OS's View of a Process

The **Process Control Block (PCB)** is the kernel's data structure representing a process. Every time the OS thinks about a process, it reads/writes the PCB.

In Linux, the PCB is `struct task_struct` defined in `<linux/sched.h>`. It has 500+ fields. Here are the critical ones:

```c
struct task_struct {
    /* ── Identity ──────────────────────────────────── */
    pid_t pid;              /* Process ID (unique) */
    pid_t tgid;            /* Thread Group ID (= pid for main thread) */
    struct task_struct *parent; /* Pointer to parent process */
    struct list_head children;  /* List of children */
    struct list_head sibling;   /* Siblings (same parent) */
    
    /* ── State ─────────────────────────────────────── */
    volatile long state;   /* TASK_RUNNING, TASK_INTERRUPTIBLE, etc. */
    int exit_code;         /* Exit code when process terminates */
    
    /* ── CPU Scheduling ────────────────────────────── */
    int prio;              /* Dynamic priority */
    int static_prio;       /* Static priority (from nice value) */
    int normal_prio;       /* Normal priority */
    unsigned int rt_priority; /* Real-time priority (1-99) */
    const struct sched_class *sched_class; /* Which scheduler? CFS/RT/etc. */
    struct sched_entity se; /* CFS scheduling data (vruntime, etc.) */
    
    /* ── Memory ─────────────────────────────────────── */
    struct mm_struct *mm;   /* Memory descriptor (page tables, VMAs) */
    struct mm_struct *active_mm; /* Current active mm (for kernel threads) */
    
    /* ── File System ────────────────────────────────── */
    struct fs_struct *fs;   /* Filesystem info (cwd, root) */
    struct files_struct *files; /* Open file descriptors table */
    
    /* ── Signals ────────────────────────────────────── */
    sigset_t blocked;       /* Blocked (masked) signals */
    struct sigpending pending; /* Pending signals */
    struct signal_struct *signal; /* Shared signal data for thread group */
    
    /* ── Credentials ────────────────────────────────── */
    const struct cred *cred; /* User/group IDs, capabilities */
    
    /* ── CPU Context (saved on context switch) ──────── */
    struct thread_struct thread; /* CPU registers, stack pointer, etc. */
    
    /* ── Timing ─────────────────────────────────────── */
    u64 utime;              /* User mode CPU time used */
    u64 stime;              /* Kernel mode CPU time used */
    u64 start_time;         /* When process was created */
    
    /* ── ... 450+ more fields ... */
};
```

**ASCII: PCB in kernel memory**
```
Kernel Memory (Ring 0 only)
┌────────────────────────────────────────────────────────────────┐
│                    Process Table                                │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ task_struct[0]  PID=1    (systemd)                        │  │
│  │  state=SLEEPING  prio=20  mm=→[page_table_for_systemd]   │  │
│  │  files=→[fd_table: 0→stdin, 1→stdout, 2→stderr, ...]    │  │
│  │  parent=→NULL   children=→[task2,task3,...]              │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ task_struct[1]  PID=1234 (nginx)                         │  │
│  │  state=RUNNING  prio=25  mm=→[page_table_for_nginx]      │  │
│  │  cpu_context: rip=0x401A20 rsp=0x7FFEE000 rbp=...       │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ task_struct[2]  PID=5678 (bash) — ZOMBIE                 │  │
│  │  state=ZOMBIE  exit_code=0  (waiting for parent wait())  │  │
│  └──────────────────────────────────────────────────────────┘  │
│  [ ... more task_structs ... ]                                  │
└────────────────────────────────────────────────────────────────┘
```

---

## 2.2 Process States and State Machine

### The 5-State Model

```
                    ┌──────────────────────┐
                    │         NEW          │
                    │  (being created)     │
                    └──────────┬───────────┘
                               │ admitted (long-term scheduler)
                               ▼
                    ┌──────────────────────┐
              ┌────▶│       READY          │◀────────────────────┐
              │     │  (waiting for CPU)   │                     │
              │     └──────────┬───────────┘                     │
              │                │ dispatch (short-term scheduler)  │
              │                ▼                                  │
              │     ┌──────────────────────┐  interrupt/       │
              │     │      RUNNING         │  preemption       │
  I/O         │     │  (using CPU right    │───────────────────┘
  completes   │     │   now)               │
              │     └──────────┬───────────┘
              │                │ I/O request / wait for event
              │                ▼
              │     ┌──────────────────────┐
              └─────│      WAITING         │
                    │  (blocked, waiting   │
                    │   for I/O or event)  │
                    └──────────┬───────────┘
                               │ (cannot transition directly to RUNNING!)
                    ┌──────────┴───────────┐
                    │      TERMINATED      │
                    │  (process finished,  │
                    │   PCB still exists   │
                    │   until parent calls │
                    │   wait())            │
                    └──────────────────────┘
```

**State Transitions:**

| From | To | Trigger |
|------|----|---------|
| NEW → READY | Long-term scheduler admits the process |
| READY → RUNNING | Short-term scheduler dispatches to CPU |
| RUNNING → READY | Time quantum expired (preemption) or voluntary yield |
| RUNNING → WAITING | Process calls read(), sleep(), wait(), etc. |
| WAITING → READY | I/O completes, or waited event occurs |
| RUNNING → TERMINATED | Process calls exit() or is killed |

**CRITICAL INTERVIEW POINT:** A WAITING process **NEVER** goes directly to RUNNING. It must go WAITING → READY → RUNNING. This is because the CPU might be busy with another process when the I/O completes.

---

### The 7-State Model (with Suspension)

When physical memory is full, the OS can **swap** a process out to disk:

```
┌──────────────┐     admit      ┌──────────────┐
│     NEW      │───────────────▶│  READY       │
└──────────────┘                └──────┬───────┘
                                       │ dispatch
                    swap out    ┌──────▼───────┐    I/O wait
                ┌───────────────│   RUNNING    │──────────────────┐
                │               └──────┬───────┘                  │
                │               I/O    │ done/                    │
                │               wait   │ event                    ▼
                │               ┌──────▼───────┐         ┌──────────────┐
                │               │   WAITING    │         │   WAITING    │
                │               │  (in memory) │         │  SUSPENDED   │
                ▼               └──────┬───────┘         │  (on disk)   │
        ┌──────────────┐              │                  └──────┬───────┘
        │    READY     │◀─────────────┘ event occurs           │
        │  SUSPENDED   │  swap in                        event │
        │  (on disk)   │◀──────────────────────────────────────┘
        └──────────────┘
```

**Process state codes in `ps` command:**
```
R = RUNNING or RUNNABLE (in run queue)
S = Sleeping (interruptible — can be woken by signal)
D = Disk sleep (UNinterruptible — waiting for I/O, cannot be killed!)
Z = Zombie (terminated, parent hasn't called wait() yet)
T = Stopped (received SIGSTOP or Ctrl+Z)
I = Idle kernel thread
X = Dead (should never appear)
< = High priority (negative nice value)
N = Low priority (positive nice value)
L = Has pages locked in memory
s = Session leader
+ = In foreground process group
```

**How to check:** `ps aux` or `cat /proc/<pid>/status`

---

## 2.3 Process Creation — fork() Internals

### fork() — The Unix Way

`fork()` creates an exact copy of the current process. After `fork()`, there are TWO processes: parent and child. They both resume executing from the exact same point — right after the `fork()` call.

```c
pid_t pid = fork();   // ← BOTH processes resume from here!
// But they get DIFFERENT return values:
//   Parent: pid = child's PID (e.g., 1234)
//   Child:  pid = 0
```

**Why fork() returns 0 to child?**
The child doesn't know its own PID from fork's return value — it uses `getpid()`. The parent needs the child's PID to wait for it or send signals. So: parent gets child PID, child gets 0.

---

### What fork() Does Internally (Step by Step)

```
fork() system call:
│
├─ 1. kernel allocates a new task_struct for child
│     child.pid = alloc_pid()  (unique new PID)
│     child.ppid = parent.pid
│
├─ 2. COPY parent's task_struct fields to child:
│     - credentials (uid, gid, groups) → COPY
│     - signal handlers → COPY
│     - open file descriptors → COPY (both share same file table entries!)
│     - current working directory → COPY
│     - scheduler stats → COPY (but reset some fields)
│
├─ 3. Copy-on-Write (COW) for memory:
│     - Do NOT copy all memory pages (would be slow!)
│     - Instead: child gets NEW page table, pointing to SAME physical pages
│     - Both page tables mark pages as READ-ONLY
│     - When either process WRITES to a page:
│       → Page fault triggered
│       → Kernel creates a COPY of that page for the writer
│       → Both processes now have their own private copies
│     - This makes fork() nearly O(1) instead of O(address space size)!
│
├─ 4. Copy kernel stack for child (new kernel stack)
│
├─ 5. Set child's CPU state to "just returned from fork() with 0"
│
├─ 6. Add child to run queue (state = TASK_RUNNING)
│
└─ 7. Return child's PID to parent (parent also resumes)
```

---

### Code: Complete fork() Example

```c
/* Phase02/04_fork_exec_wait.c
 * Demonstrates: fork, exec, wait system calls
 * Compile: gcc -Wall -o fork_demo 04_fork_exec_wait.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    /* fork(), getpid(), getppid(), execl() */
#include <sys/wait.h>  /* wait(), waitpid(), WIFEXITED(), WEXITSTATUS() */

int main(void) {
    printf("Parent PID=%d starting\n", getpid());
    
    /* ── Create a child process ── */
    pid_t pid = fork();
    
    if (pid < 0) {
        /* fork() failed (out of memory, too many processes) */
        perror("fork failed");
        exit(EXIT_FAILURE);
        
    } else if (pid == 0) {
        /* ─────────── CHILD PROCESS ─────────── */
        printf("Child:  PID=%d, PPID=%d\n", getpid(), getppid());
        
        /* exec: replace this process image with /bin/ls */
        /* After execl(), nothing below it runs — process image is replaced! */
        execl("/bin/ls", "ls", "-la", "/tmp", NULL);
        
        /* If we reach here, exec() failed */
        perror("exec failed");
        exit(EXIT_FAILURE);
        
    } else {
        /* ─────────── PARENT PROCESS ─────────── */
        printf("Parent: PID=%d, created child PID=%d\n", getpid(), pid);
        
        /* Wait for child to complete */
        int status;
        pid_t finished = waitpid(pid, &status, 0);
        
        if (finished == -1) {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
        }
        
        /* Decode the exit status */
        if (WIFEXITED(status)) {
            /* Child called exit() or returned from main() */
            printf("Parent: child %d exited with code %d\n",
                   finished, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            /* Child was killed by a signal */
            printf("Parent: child %d killed by signal %d\n",
                   finished, WTERMSIG(status));
        }
        
        printf("Parent: done, exiting\n");
    }
    
    return 0;
}
/* Output:
 * Parent PID=1234 starting
 * Parent: PID=1234, created child PID=1235
 * Child:  PID=1235, PPID=1234
 * total 0
 * drwxrwxrwt 18 root root 360 Jun 20 12:00 .
 * drwxr-xr-x 19 root root 380 Jun  1 09:00 ..
 * ...
 * Parent: child 1235 exited with code 0
 * Parent: done, exiting
 */
```

---

### Zombie and Orphan Processes

#### Zombie Process
When a child exits, its PCB (task_struct) is NOT immediately freed. Why? Because the parent might want to check the exit status (`wait()`). The child is in state **ZOMBIE** — dead but PCB still exists.

The PCB is freed ONLY when the parent calls `wait()` or `waitpid()`.

```c
/* Creating a zombie deliberately */
#include <stdio.h>
#include <unistd.h>
int main() {
    pid_t pid = fork();
    if (pid == 0) {
        exit(0);        /* Child exits immediately */
    }
    /* Parent sleeps without calling wait() */
    /* Child is now a ZOMBIE! */
    sleep(30);
    /* Run: ps aux | grep Z   to see zombie */
    return 0;
}
```

**How to prevent zombies:**
1. Parent calls `wait()` / `waitpid()` properly
2. Set `signal(SIGCHLD, SIG_IGN)` — tells kernel to auto-reap children
3. Use double-fork trick (child forks grandchild and exits; grandchild adopted by init)

#### Orphan Process
An **orphan** is a process whose parent exits before it does. The OS reparents orphans to **PID 1** (init/systemd), which automatically calls `wait()` for all its children.

```c
/* Creating an orphan deliberately */
#include <stdio.h>
#include <unistd.h>
int main() {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child: parent will exit first */
        sleep(10);      /* Will be reparented to init (PID 1) */
        printf("My new parent: %d\n", getppid()); /* Should print 1 */
        return 0;
    }
    /* Parent exits immediately, orphaning the child */
    printf("Parent exiting, orphaning child %d\n", pid);
    return 0;
}
```

---

## 2.4 Context Switching — The Core of Multitasking

### What is Context Switching?

A **context switch** is saving the complete state of one process (its "context") so that another process can use the CPU, then restoring a different process's context so IT can run.

**The illusion:** Each process thinks it has the CPU to itself. In reality, on a 4-core machine with 100 processes, each process gets the CPU for tiny slices (typically 1–10ms), rapidly switching between them. The illusion of parallel execution is created.

### What Gets Saved/Restored?

```
When process A is preempted and process B runs:

SAVED (from A, to A's kernel stack):
  ├── RIP  (program counter — where to resume)
  ├── RSP  (stack pointer)
  ├── RBP  (frame pointer)
  ├── RAX, RBX, RCX, RDX, RSI, RDI, R8-R15  (general purpose)
  ├── RFLAGS  (CPU flags: zero, carry, overflow, etc.)
  ├── CR3  (page table base — memory map of process A)
  ├── SSE/FPU state  (floating point / SIMD registers — HUGE!)
  └── Segment registers (CS, DS, SS, FS, GS)

LOADED (from B's kernel stack):
  ├── All the same registers, but B's values
  └── CR3 updated to point to B's page table
      └── This flushes the TLB! (expensive — all cached translations gone)
```

### Context Switch Timeline

```
Time ──────────────────────────────────────────────────────────▶

Process A:  ████████████████████│                              │████████
                                 │ preempted (timer interrupt) │ restored
Process B:                       │        ████████████████████ │
                                 │                             │
                             ┌───┴───────────────────────────┐
                             │     CONTEXT SWITCH OVERHEAD    │
                             │  1. Save A's registers          │
                             │  2. Update A's PCB              │
                             │  3. Scheduler picks B           │
                             │  4. Load B's registers          │
                             │  5. Flush TLB (load B's CR3)    │
                             │  Cost: ~1-10 microseconds       │
                             └───────────────────────────────┘
```

**Why context switching is expensive:**
1. **Register save/restore:** ~30 registers × 8 bytes = 240 bytes, but FPU state = 512 bytes (XSAVE)
2. **TLB flush:** When CR3 changes, all cached virtual→physical translations are invalidated. Next accesses = TLB misses = page table walks = slow!
3. **Cache pollution:** Process B's code/data evicts Process A's cache lines. When A resumes, it's cache-cold.

---

## 2.5 Signals — UNIX Process Communication

### What is a Signal?

A **signal** is an asynchronous notification sent to a process. It's like a software interrupt — the process's normal execution is interrupted to handle the signal.

Think of signals as OS-level "push notifications" to processes.

```
Process is happily running...
    
    Executing instruction at RIP=0x401A20
    Executing instruction at RIP=0x401A24
    Executing instruction at RIP=0x401A28
    
    ← SIGNAL DELIVERED (e.g., SIGINT from Ctrl+C) →
    
    CPU jumps to signal handler at address 0x404000
    Signal handler executes...
    Signal handler returns
    
    Resumes at RIP=0x401A2C (where it left off)
```

### Common Signals

| Signal | Number | Default Action | Can Catch/Ignore? | Common Trigger |
|--------|--------|---------------|-------------------|----------------|
| SIGINT | 2 | Terminate | YES | Ctrl+C |
| SIGQUIT | 3 | Core dump | YES | Ctrl+\ |
| SIGILL | 4 | Core dump | YES | Illegal instruction |
| SIGTRAP | 5 | Core dump | YES | Breakpoint (debugger) |
| SIGABRT | 6 | Core dump | YES | assert() failure |
| SIGFPE | 8 | Core dump | YES | Floating point exception |
| SIGKILL | **9** | Terminate | **NO** | kill -9, force kill |
| SIGSEGV | 11 | Core dump | YES | Segmentation fault |
| SIGPIPE | 13 | Terminate | YES | Write to broken pipe |
| SIGALRM | 14 | Terminate | YES | alarm() timer |
| SIGTERM | 15 | Terminate | YES | kill (default), graceful shutdown |
| SIGCHLD | 17 | Ignore | YES | Child terminated |
| SIGCONT | 18 | Continue | YES | Resume stopped process |
| SIGSTOP | **19** | Stop | **NO** | Ctrl+Z-like |
| SIGTSTP | 20 | Stop | YES | Ctrl+Z |

**KEY INTERVIEW POINT:** `SIGKILL (9)` and `SIGSTOP (19)` **CANNOT** be caught, blocked, or ignored. This is by design — they give the OS and root users ultimate control over any process.

---

# ═══════════════════════════════════════════════════════
# CHAPTER 3: THREADS — DEEP DIVE
# ═══════════════════════════════════════════════════════

---

## 3.1 What is a Thread?

A **thread** is the smallest unit of execution scheduled by the OS. A process can have **multiple threads** sharing the same address space.

### What Threads Share vs What is Private

```
PROCESS ADDRESS SPACE
┌──────────────────────────────────────────────────────┐
│                    SHARED by ALL threads              │
│  ┌─────────────────────────────────────────────────┐  │
│  │ Code Segment   (all threads run the same code)  │  │
│  │ Data Segment   (global variables — SHARED!)     │  │
│  │ Heap           (malloc — SHARED, must sync!)    │  │
│  │ File Descriptors (open files — SHARED!)         │  │
│  │ Signal handlers, cwd, uid/gid                   │  │
│  └─────────────────────────────────────────────────┘  │
│                                                        │
│     Thread 1          Thread 2          Thread 3      │
│  ┌────────────┐    ┌────────────┐    ┌────────────┐  │
│  │ PRIVATE:   │    │ PRIVATE:   │    │ PRIVATE:   │  │
│  │ Stack      │    │ Stack      │    │ Stack      │  │
│  │ Registers  │    │ Registers  │    │ Registers  │  │
│  │ PC (RIP)   │    │ PC (RIP)   │    │ PC (RIP)   │  │
│  │ Thread ID  │    │ Thread ID  │    │ Thread ID  │  │
│  │ errno      │    │ errno      │    │ errno      │  │
│  │ Signal mask│    │ Signal mask│    │ Signal mask│  │
│  │ TLS vars   │    │ TLS vars   │    │ TLS vars   │  │
│  └────────────┘    └────────────┘    └────────────┘  │
└──────────────────────────────────────────────────────┘
```

---

### Thread vs Process: The Full Comparison

| Feature | Process | Thread |
|---------|---------|--------|
| **Address space** | Own separate address space | Shared with other threads |
| **Creation cost** | Expensive (~1ms, needs COW setup) | Cheap (~10μs, just a new stack) |
| **Context switch cost** | Expensive (TLB flush, CR3 change) | Cheaper (no CR3 change if same process) |
| **Communication** | IPC (pipe, shared memory, sockets) | Direct shared memory (but needs sync!) |
| **Fault isolation** | Bug crashes only this process | Bug can corrupt all threads in process |
| **Security** | Separate address spaces → isolated | All threads can touch each other's memory |
| **Parallelism** | Yes (true parallelism on multi-core) | Yes (if kernel threads, one per core) |
| **Linux syscall** | `fork()` / `clone()` | `clone()` with CLONE_VM flag |

---

### Thread Models

#### Many-to-One Model (Pure User-Level Threads)

```
User Space:   Thread A  Thread B  Thread C  Thread D
              ────────────────────────────────────
              User-Level Thread Library (scheduler)
                              │
Kernel Space:         [ Kernel Thread ]  ← Only ONE!
```

**Problem:** If Thread B does a blocking system call, ALL threads block (the kernel thread blocks, and there's only one kernel thread).

**Example:** Early Java green threads, Python 2 threads (before adding kernel threads)

#### One-to-One Model (POSIX pthreads — Standard Today)

```
User Space:   Thread A  Thread B  Thread C  Thread D
                │           │          │         │
Kernel Space: [KT1]       [KT2]      [KT3]    [KT4]
              (Kernel Threads — scheduled by OS directly)
```

**Advantages:** True parallelism on multi-core. Blocking one thread doesn't block others.

**Linux implements this:** Each pthread = a Linux task (task_struct) created with `clone(CLONE_VM | CLONE_FS | CLONE_FILES | ...)`.

---

### POSIX Threads (pthreads) Complete Code Example

```c
/* Phase03/02_pthreads_basics.c
 * Compile: gcc -Wall -pthread -o pthreads_demo 02_pthreads_basics.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>   /* POSIX threads API */
#include <unistd.h>

/* Shared counter — accessed by multiple threads (potential race condition!) */
static long counter = 0;
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Thread function: signature must be void* f(void*) */
void* increment_worker(void *arg) {
    int id = *(int*)arg;       /* Cast arg back to int* and dereference */
    int iterations = 1000000;
    
    printf("Thread %d starting (tid=%lu)\n", id, pthread_self());
    
    for (int i = 0; i < iterations; i++) {
        /* CORRECT: protect shared variable with mutex */
        pthread_mutex_lock(&counter_mutex);
        counter++;
        pthread_mutex_unlock(&counter_mutex);
    }
    
    printf("Thread %d done\n", id);
    return NULL;  /* Thread return value (can be retrieved with pthread_join) */
}

int main(void) {
    const int NUM_THREADS = 4;
    pthread_t threads[NUM_THREADS];  /* Thread handles */
    int ids[NUM_THREADS];            /* Separate ID for each thread */
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        int ret = pthread_create(
            &threads[i],      /* Thread handle (output) */
            NULL,             /* Thread attributes (NULL = defaults) */
            increment_worker, /* Thread function */
            &ids[i]           /* Argument passed to thread function */
        );
        if (ret != 0) {
            fprintf(stderr, "pthread_create failed: %d\n", ret);
            exit(EXIT_FAILURE);
        }
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < NUM_THREADS; i++) {
        void *retval;
        pthread_join(threads[i], &retval);  /* Block until thread i finishes */
    }
    
    /* With mutex: counter should be exactly 4,000,000 */
    printf("Final counter: %ld (expected: %d)\n", counter, NUM_THREADS * 1000000);
    
    pthread_mutex_destroy(&counter_mutex);  /* Cleanup mutex resources */
    return 0;
}
/* Output (with mutex — correct):
 * Thread 0 starting (tid=140234567890)
 * Thread 1 starting (tid=140234556789)
 * Thread 2 starting (tid=140234545678)
 * Thread 3 starting (tid=140234534567)
 * Thread 2 done
 * Thread 0 done
 * Thread 3 done
 * Thread 1 done
 * Final counter: 4000000 (expected: 4000000)
 */
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 4: CPU SCHEDULING — COMPLETE WITH ALGORITHMS
# ═══════════════════════════════════════════════════════

---

## 4.1 Scheduling Concepts and Goals

The **CPU scheduler** decides which process runs next. This is one of the most critical OS components, directly determining system responsiveness, throughput, and fairness.

### Key Performance Metrics

| Metric | Definition | Formula | Goal |
|--------|-----------|---------|------|
| **CPU Utilization** | % of time CPU does useful work | useful_time / total_time × 100 | Maximize (90-99% ideal) |
| **Throughput** | Processes completed per second | processes_done / time | Maximize |
| **Turnaround Time** | Time from submission to completion | completion_time - arrival_time | Minimize |
| **Waiting Time** | Time spent in ready queue | turnaround_time - burst_time | Minimize |
| **Response Time** | Time from submission to FIRST response | first_response - arrival_time | Minimize (interactive) |

**The Impossibility:**
You CANNOT minimize all metrics simultaneously! There are inherent tradeoffs:
- Minimizing response time (for interactive apps) requires frequent context switches
- Frequent context switches reduce throughput
- Round-robin gives good response time but poor turnaround time for long jobs
- SJF minimizes average waiting time but starves long jobs

---

## 4.2 FCFS (First-Come First-Served)

### Algorithm
Queue processes by arrival time. Run until complete. No preemption.

### Worked Example 1 — Simple FCFS

```
Processes: P1 (burst=24), P2 (burst=3), P3 (burst=3). All arrive at t=0.
Order: P1 P2 P3 (arrived in this order)

Gantt Chart:
┌────────────────────────────┬───┬───┐
│            P1              │P2 │P3 │
│           (24)             │(3)│(3)│
└────────────────────────────┴───┴───┘
0                           24  27  30

Calculations:
  Completion times: P1=24, P2=27, P3=30
  Turnaround times: P1=24-0=24, P2=27-0=27, P3=30-0=30
  Waiting times:    P1=0,        P2=24,       P3=27

  Avg Turnaround = (24+27+30)/3 = 27
  Avg Waiting    = (0+24+27)/3  = 17
```

### The Convoy Effect
If P1 had been SHORT and P2 was LONG:
```
Order: P1(3), P2(24), P3(3)
Avg Waiting = (0 + 3 + 27) / 3 = 10  ← MUCH BETTER!
```
Lesson: **Long jobs at the front hurt everyone behind them** — this is the Convoy Effect.

---

## 4.3 SJF (Shortest Job First) — Non-Preemptive

### Algorithm
When CPU is free, pick the process with the **shortest next CPU burst**.

### Key Property: Optimal Average Waiting Time
SJF is provably optimal for minimizing average waiting time among all **non-preemptive** algorithms.

**Proof sketch:** If we have two adjacent jobs with burst times a and b where a > b, swapping them reduces average waiting time. Therefore the order that minimizes waiting has jobs in non-decreasing order of burst time = SJF.

### Worked Example — SJF with Different Arrival Times

```
Process  Arrival  Burst
  P1       0       7
  P2       2       4
  P3       4       1
  P4       5       4

At t=0: Only P1 available → run P1
At t=2: P2 arrives but P1 is running (non-preemptive)
At t=4: P3 arrives but P1 still running
At t=5: P4 arrives but P1 still running
At t=7: P1 completes. Ready queue: {P2(4), P3(1), P4(4)}
         SJF picks SHORTEST: P3(burst=1) → run P3
At t=8:  P3 done. Ready queue: {P2(4), P4(4)}
         Both have same burst → FCFS order → P2 runs
At t=12: P2 done. Run P4.
At t=16: P4 done.

Gantt Chart:
┌──────────┬─┬────┬────┐
│    P1    │P3│ P2 │ P4 │
│   (7)   │(1)│(4) │(4) │
└──────────┴─┴────┴────┘
0          7  8    12   16

Turnaround: P1=7-0=7, P2=12-2=10, P3=8-4=4, P4=16-5=11
Waiting:    P1=0,      P2=6,        P3=3,      P4=7
Avg TAT = (7+10+4+11)/4 = 8
Avg WT  = (0+6+3+7)/4   = 4
```

### The Problem: We Don't Know Future Burst Times!
**Solution — Exponential Averaging:**
Predict next burst using past bursts:
```
τ(n+1) = α × t(n) + (1-α) × τ(n)

Where:
  τ(n+1) = predicted next burst
  t(n)   = actual last burst
  τ(n)   = previous prediction
  α ∈ [0,1] = weight of recent history

Example with α=0.5:
  Actual bursts:     6,   4,   6,   4,   13,  13,   13
  Predictions τ(n+1):10, 8,   6,   6,   5,   9,   11,  12

If α=0: Only history matters (t(n) ignored completely)
If α=1: Only most recent burst matters (history ignored)
Typical: α=0.5
```

---

## 4.4 SRTF (Shortest Remaining Time First) — Preemptive SJF

### Algorithm
At every point in time, run the process with the **shortest REMAINING burst time**. If a new process arrives with shorter remaining time than current, preempt current.

### Worked Example — SRTF Step by Step

```
Process  Arrival  Burst
  P1       0       8
  P2       1       4
  P3       2       9
  P4       3       5

Timeline analysis (need to check at every arrival):

t=0: Only P1. Run P1. Remaining: P1=8
t=1: P2 arrives (burst=4). Current P1 remaining=7. 4<7 → PREEMPT P1, run P2
t=2: P3 arrives (burst=9). P2 remaining=3. 3<9 → continue P2
t=3: P4 arrives (burst=5). P2 remaining=2. 2<5 → continue P2
t=5: P2 completes. Queue: {P1(rem=7), P3(rem=9), P4(rem=5)}
     Shortest remaining = P4(5) → run P4
t=10: P4 completes. Queue: {P1(rem=7), P3(rem=9)}
      Shortest = P1(7) → run P1
t=17: P1 completes. Queue: {P3(rem=9)}
      Run P3
t=26: P3 completes.

Gantt Chart:
┌─┬────┬─────┬───────┬─────────┐
│P1│ P2 │ P4  │  P1   │   P3    │
│1│ 4  │ 5   │  7    │   9     │
└─┴────┴─────┴───────┴─────────┘
0  1    5    10       17       26

Turnaround: P1=17-0=17, P2=5-1=4, P3=26-2=24, P4=10-3=7
Waiting:    P1=17-8=9,  P2=5-1-4=0, P3=26-2-9=15, P4=10-3-5=2
Avg WT = (9+0+15+2)/4 = 6.5
```

---

## 4.5 Round Robin (RR) — The Foundation of Modern Scheduling

### Algorithm
Each process gets a fixed **time quantum** (q). After q time units, the process is preempted and moved to the back of the queue. This is what desktop/server OS actually use.

### Worked Example — RR with q=2

```
Processes: P1(burst=5), P2(burst=3), P3(burst=1), P4(burst=2)
All arrive at t=0.

Queue at each step:
t=0:  [P1,P2,P3,P4] → Run P1 for 2
t=2:  [P2,P3,P4,P1(rem=3)] → Run P2 for 2
t=4:  [P3,P4,P1(rem=3),P2(rem=1)] → Run P3 for 1 (completes!)
t=5:  [P4,P1(rem=3),P2(rem=1)] → Run P4 for 2 (completes!)
t=7:  [P1(rem=3),P2(rem=1)] → Run P1 for 2
t=9:  [P2(rem=1),P1(rem=1)] → Run P2 for 1 (completes!)
t=10: [P1(rem=1)] → Run P1 for 1 (completes!)
t=11: Done!

Gantt Chart:
┌──┬──┬─┬──┬──┬─┬─┐
│P1│P2│P3│P4│P1│P2│P1│
│ 2│ 2│1 │2 │2 │1 │1 │
└──┴──┴─┴──┴──┴─┴─┘
0   2  4  5  7  9  10 11

Completion: P1=11, P2=10, P3=5, P4=7
TAT: P1=11, P2=10, P3=5, P4=7
WT:  P1=6,  P2=7,  P3=4, P4=5
Avg WT = (6+7+4+5)/4 = 5.5
```

### Time Quantum Selection

```
If q is too SMALL (e.g., q=1ms):
  ✓ Great response time
  ✗ Too many context switches → high overhead
  ✗ If context switch costs 1ms and q=1ms → 50% overhead!

If q is too LARGE (e.g., q=100ms):
  ✓ Very low context switch overhead
  ✗ Degenerates to FCFS
  ✗ Poor response time for interactive processes

RULE OF THUMB: q should be larger than 80% of CPU bursts.
Typical values: Linux CFS uses ~4-8ms as target latency.
```

---

## 4.6 Priority Scheduling

### Algorithm
Each process has a priority number. Higher priority processes run first.

**Convention:** Lower number = higher priority (like Linux nice values) OR higher number = higher priority — depends on OS. **Always clarify in interviews.**

### Priority Inversion — The Mars Pathfinder Bug

This is one of the most famous OS bugs in history. In 1997, NASA's Mars Pathfinder rover kept resetting itself. Investigation revealed a **priority inversion**:

```
Process priorities: High=BUS (P_high)  Medium=Camera(P_med)  Low=Weather(P_low)

Timeline of the bug:
1. P_low acquires mutex for shared bus
2. P_med starts running (higher priority than P_low) → P_low never gets CPU
3. P_high needs the bus mutex → BLOCKED (P_low holds it, but P_low can't run!)
4. P_med runs forever
5. WATCHDOG TIMER FIRES: "System hung!" → RESET

This is priority INVERSION: P_med (medium priority) effectively blocks P_high!
```

**Solution — Priority Inheritance:**
When P_high blocks waiting for P_low's mutex, temporarily **boost P_low's priority to P_high's level** so P_low can run, release the mutex, and P_high can proceed.

NASA fixed Pathfinder by enabling the priority inheritance protocol in VxWorks RTOS (it was already implemented, just not enabled!).

---

## 4.7 Linux CFS — Completely Fair Scheduler

### Design Goal: Perfect Fairness

CFS (introduced in Linux 2.6.23, 2007) aims to give each process exactly 1/N of the CPU where N is the number of processes.

### Virtual Runtime (vruntime)

```
KEY CONCEPT: vruntime = virtual runtime = how much "virtual" time a process has used

Rule: Always run the process with MINIMUM vruntime

When a process runs for actual time t:
  vruntime += t × (1024 / process_weight)

  Where: weight = 1024 / (1.25 ^ nice_value)
  nice=-20 (high priority) → weight=88761 → vruntime increases SLOWLY
  nice=0   (default)       → weight=1024  → vruntime increases at 1:1 rate
  nice=19  (low priority)  → weight=15    → vruntime increases FAST

Result: High-priority processes' vruntime grows slowly → they get more CPU
        Low-priority processes' vruntime grows fast → they get less CPU
        But ALL processes eventually run (no starvation!)
```

### Red-Black Tree: CFS's Run Queue

CFS stores all runnable processes in a **red-black tree** ordered by vruntime:

```
      [vruntime=100, P3]
      /                 \
[vrt=80, P1]         [vrt=120, P5]
    /      \               \
[vrt=70,P4][vrt=90,P2]  [vrt=130,P6]

→ Leftmost node (minimum vruntime) = next to run = P4 (vrt=70)
→ O(log n) to find min, insert, delete
→ Cache the leftmost node for O(1) lookup of next task!
```

---

## 4.8 MLFQ — Multilevel Feedback Queue

MLFQ is the scheduler used (in spirit) by Windows and macOS. It:
- Has multiple queues with different priorities
- Moves processes between queues based on their behavior
- Rewards I/O-bound processes (interactive, responsive)
- Penalizes CPU-bound processes (lower priority over time)

### MLFQ Rules

```
Q0 (highest priority, shortest quantum q=2)
Q1 (medium priority, quantum q=4)  
Q2 (lowest priority, quantum q=8 or FCFS)

Rules:
1. New processes start at Q0 (highest priority)
2. If a process uses its ENTIRE quantum → demoted to lower queue
   (It's CPU-intensive, not interactive → lower priority)
3. If a process gives up CPU BEFORE quantum expires → stays in same queue
   (It's I/O-intensive → keep high priority for responsiveness)
4. Every S seconds, ALL processes boosted to Q0 (prevents starvation)

Example:
Process A (CPU-intensive): uses full 2ms at Q0 → moved to Q1
                           uses full 4ms at Q1 → moved to Q2
                           stays in Q2 running with low priority

Process B (interactive): uses 0.5ms then waits for keyboard → stays in Q0
                         always gets fast response (still in Q0!)
```

---

## 4.9 Scheduling Algorithms: Master Comparison Table

| Algorithm | Preemptive? | Starvation? | Overhead | Best For |
|-----------|-------------|-------------|----------|----------|
| FCFS | No | No | Very Low | Batch, simple jobs |
| SJF | No | Yes (long jobs) | Low | Batch with known bursts |
| SRTF | Yes | Yes (long jobs) | Medium | Optimal avg WT |
| Round Robin | Yes | No | Medium | Interactive, fair |
| Priority | Both | Yes | Medium | Real-time, tiered |
| MLFQ | Yes | No (w/ boost) | High | Desktop OS |
| CFS | Yes | No | Medium | Linux general purpose |
| EDF | Yes | No | High | Real-time systems |

---

## 4.10 Numerical Problems — Master Set

### Problem 1 (FCFS with Arrival Times)

```
Process  Arrival  Burst
P1         0        4
P2         1        3
P3         2        1
P4         3        2
P5         4        5

Solution (FCFS — first arrived, no preemption):

Gantt:
┌────┬───┬─┬──┬─────┐
│ P1 │P2 │P3│P4│ P5  │
└────┴───┴─┴──┴─────┘
0    4   7  8  10   15

Completion: P1=4, P2=7, P3=8, P4=10, P5=15
TAT = completion - arrival:
  P1=4-0=4, P2=7-1=6, P3=8-2=6, P4=10-3=7, P5=15-4=11
Waiting = TAT - burst:
  P1=0, P2=3, P3=5, P4=5, P5=6

Avg TAT = (4+6+6+7+11)/5 = 34/5 = 6.8
Avg WT  = (0+3+5+5+6)/5  = 19/5 = 3.8
```

### Problem 2 (Round Robin q=2)

```
Process  Arrival  Burst
P1         0        5
P2         0        3
P3         0        4

Gantt (q=2, order of arrival: P1,P2,P3):
t=0: Run P1 for 2. Queue: [P2,P3,P1(rem=3)]
t=2: Run P2 for 2. Queue: [P3,P1(rem=3),P2(rem=1)]
t=4: Run P3 for 2. Queue: [P1(rem=3),P2(rem=1),P3(rem=2)]
t=6: Run P1 for 2. Queue: [P2(rem=1),P3(rem=2),P1(rem=1)]
t=8: Run P2 for 1. DONE. Queue: [P3(rem=2),P1(rem=1)]
t=9: Run P3 for 2. DONE. Queue: [P1(rem=1)]
t=11: Run P1 for 1. DONE.

Gantt:
┌──┬──┬──┬──┬─┬──┬─┐
│P1│P2│P3│P1│P2│P3│P1│
│2 │2 │2 │2 │1│ 2│1 │
└──┴──┴──┴──┴─┴──┴─┘
0  2  4  6  8  9  11 12

Completion: P1=12, P2=9, P3=11
TAT: P1=12, P2=9, P3=11
WT:  P1=7, P2=6, P3=7
Avg WT = (7+6+7)/3 = 6.67
```

### Problem 3 (Priority Scheduling — Non-Preemptive)

```
Process  Arrival  Burst  Priority(lower=higher)
P1         0        10      3
P2         0         1      1
P3         0         2      4
P4         0         1      5
P5         0         5      2

Order: P2(pri=1), P5(pri=2), P1(pri=3), P3(pri=4), P4(pri=5)

Gantt:
┌─┬─────┬──────────┬──┬─┐
│P2│ P5  │    P1    │P3│P4│
│1│  5  │    10    │2 │1 │
└─┴─────┴──────────┴──┴─┘
0  1    6           16 18 19

WT: P2=0, P5=1, P1=6, P3=16, P4=18
Avg WT = (0+1+6+16+18)/5 = 41/5 = 8.2
STARVATION: P3 and P4 had to wait 16 and 18 time units! (Aging would fix this)
```

---

## MCQ MEGA-BANK — Scheduling

**Q1.** Which scheduling algorithm can cause a convoy effect? [MEDIUM]
- A) Round Robin
- B) FCFS ✓ (CORRECT)
- C) SJF
- D) SRTF

*Explanation:* Convoy effect = short processes stuck behind a long process. FCFS never preempts, so a long process holds the CPU while short processes wait. RR preempts after each quantum, avoiding convoy effect.

**Q2.** Round Robin scheduling with q=∞ is equivalent to: [MEDIUM]
- A) SJF
- B) SRTF
- C) FCFS ✓ (CORRECT)
- D) Priority scheduling

**Q3.** The Linux CFS scheduler uses which data structure for its run queue? [HARD]
- A) Binary heap
- B) Sorted array
- C) Red-Black tree ✓ (CORRECT)
- D) Hash table

*Explanation:* CFS needs O(log n) insert/delete and O(1) min-vruntime lookup. Red-Black tree (balanced BST) provides O(log n) operations. The leftmost node (min vruntime) is cached for O(1) lookup.

**Q4.** Which scheduling algorithm is provably optimal for minimizing average waiting time? [HARD]
- A) FCFS
- B) SJF ✓ (CORRECT — for non-preemptive)
- C) Round Robin
- D) Priority scheduling

**Q5.** Priority inversion occurs when: [HARD]
- A) A low-priority process runs before a high-priority process ✓ (CORRECT)
- B) A process's priority is inverted from positive to negative
- C) The scheduler runs backwards
- D) Two processes have the same priority

**Q6.** In MLFQ, a process that always uses its entire time quantum will: [HARD]
- A) Stay at the highest priority queue
- B) Be moved to a lower priority queue ✓ (CORRECT)
- C) Be killed
- D) Be moved to the highest priority queue

**Q7.** Which of the following is NOT a preemptive scheduling algorithm? [EASY]
- A) SRTF
- B) Round Robin
- C) SJF ✓ (CORRECT — non-preemptive version)
- D) FCFS ✓ (CORRECT — also correct)

*Note:* Both SJF and FCFS are non-preemptive. SRTF is the preemptive version of SJF.

**Q8.** The scheduling algorithm that gives each process a 1/N share of the CPU: [MEDIUM]
- A) FCFS
- B) Priority scheduling
- C) CFS / Round Robin ✓ (CORRECT)
- D) SJF

**Q9.** Starvation in priority scheduling can be prevented by: [MEDIUM]
- A) Using a smaller time quantum
- B) Aging (gradually increasing priority of waiting processes) ✓ (CORRECT)
- C) Using non-preemptive scheduling
- D) Reducing the number of processes

**Q10.** A process spends most of its time waiting for disk I/O. It is: [EASY]
- A) CPU-bound
- B) I/O-bound ✓ (CORRECT)
- C) Memory-bound
- D) Compute-bound

---

# ═══════════════════════════════════════════════════════
# CHAPTER 5: COMPLETE INTERVIEW QUESTION BANK
# ═══════════════════════════════════════════════════════

## Process & Thread Interview Questions

### Q1: "What is the difference between a process and a thread?"

**30-second answer:**
> "A process is an independent unit of execution with its own address space, file descriptors, and resources. A thread is a unit of execution within a process that shares the address space with other threads in the same process. Threads are cheaper to create but require synchronization to access shared data safely."

**Full 5-minute answer:**
> "A **process** is defined by its own virtual address space — all memory (code, stack, heap, data) is private to that process. When one process crashes, other processes are unaffected. Communication between processes requires IPC (pipes, sockets, shared memory), which has overhead.
>
> A **thread** lives inside a process and shares the code segment, data segment, heap, and file descriptors with all other threads in that process. Threads have their own stack and CPU registers (including program counter). Creating a thread is faster than creating a process because there's no need to copy the address space (just allocate a new stack).
>
> **When to use processes:** When you need fault isolation (web browser: each tab in its own process so one crash doesn't kill all tabs), or when security matters (separate processes can't read each other's memory).
>
> **When to use threads:** When tasks need to share data efficiently (database thread pool: all threads share the connection pool), or when you need many workers without the overhead of many processes (web server: thousands of concurrent connections).
>
> In Linux, both are implemented via the `clone()` system call — the difference is which resources are shared: `clone(CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND)` creates a thread; `fork()` creates a process."

---

### Q2: "What is a zombie process? Why does it exist?"

**Answer:**
> "A zombie process is a process that has **completed execution** (called exit()) but whose **PCB (task_struct) still exists** in the kernel's process table.
>
> It exists because when a process exits, the OS needs to preserve the exit status (exit code, resource usage statistics) until the **parent process calls wait()**. Only after the parent calls wait() does the kernel free the PCB.
>
> If the parent never calls wait(), zombies accumulate. They don't use CPU or memory (they're 'dead'), but they DO occupy a slot in the process table. If too many zombies accumulate, the process table fills up and new processes cannot be created.
>
> **Prevention:** Use `waitpid()` in the parent, or set `signal(SIGCHLD, SIG_IGN)` to tell the kernel to automatically reap children.
>
> **Difference from orphan:** An orphan is a process whose PARENT died first. The OS reparents orphans to PID 1 (systemd), which calls wait() for them. Orphans are NOT zombies — they're still running. Zombies are dead but unreaped."

---

### Q3: "What happens when you call fork()?"

**Answer:**
> "fork() is a system call that creates an exact copy (child) of the calling process (parent). Here's what happens internally:
>
> 1. **Allocate new PCB:** Kernel creates a new task_struct for the child with a new unique PID.
>
> 2. **Copy PCB fields:** Most fields are copied from parent to child (credentials, signal handlers, file descriptors — all inherited).
>
> 3. **Copy-on-Write (COW) for memory:** Instead of physically copying all memory pages (which could be GBs!), the child gets a new page table pointing to the SAME physical pages as the parent. Both page tables mark these pages as READ-ONLY. When either process writes to a page, a page fault occurs, the kernel creates a private copy for the writer, and marks it writable. This makes fork() nearly O(1) in time!
>
> 4. **Set return values:** Parent receives child's PID (so it can wait for it / send signals). Child receives 0 (indicating it's the child).
>
> 5. **Both resume execution:** Both parent and child continue from the instruction AFTER fork(). They follow different code paths based on the return value.
>
> Common pattern: `fork()` then immediately `exec()` in the child — this is how shells run programs."

---

### Q4: "Explain CFS scheduler."

**Answer:**
> "CFS (Completely Fair Scheduler) is Linux's main scheduler, introduced in kernel 2.6.23.
>
> **Core idea:** Give every process exactly 1/N of the CPU where N is the number of runnable processes. Achieve this by tracking 'virtual runtime' (vruntime) — how much virtual time each process has consumed.
>
> **vruntime accounting:** When a process runs for actual time t, its vruntime increases by `t × (1024 / weight)`. High-priority processes have high weight → vruntime grows SLOWLY → they get MORE CPU time. Low-priority processes have low weight → vruntime grows FAST → they get LESS CPU time.
>
> **Scheduling decision:** Always run the process with the MINIMUM vruntime. This is stored in a red-black tree ordered by vruntime, so the next process is always the leftmost node — O(1) lookup (kernel caches it).
>
> **Preemption:** A running process is preempted when its vruntime exceeds the next process's vruntime by more than a 'granularity' threshold (avoids too many context switches).
>
> **Result:** Over time, all processes accumulate approximately equal vruntime → perfectly fair CPU distribution."

---

### Q5: "What is a context switch and why is it expensive?"

**Answer:**
> "A context switch is the process of saving the complete CPU state of a running process and loading the saved state of another process so it can run.
>
> **What gets saved:** All CPU registers (16 general-purpose on x86-64), program counter (RIP), stack pointer (RSP), CPU flags (RFLAGS), floating-point/SIMD state (hundreds of bytes via XSAVE), and the page table base register (CR3).
>
> **Why expensive — 3 reasons:**
>
> 1. **Register save/restore overhead:** ~500 bytes of state saved/loaded per switch. On modern CPUs doing billions of ops/sec, this is relatively fast (~100ns), but at 1000 context switches/second, that's 100μs/second overhead.
>
> 2. **TLB flush:** Changing CR3 (page table base) invalidates all cached virtual→physical translations in the TLB. The new process must re-walk page tables for every memory access until the TLB warms up again. This can add microseconds per access for thousands of accesses.
>
> 3. **Cache pollution:** The new process's code and data evict the old process's data from L1/L2/L3 caches. The new process runs 'cache-cold' until its working set is cached again. For cache-heavy workloads, this can cause 10-100× slower memory accesses.
>
> **Mitigation:** ASIDs (Address Space Identifiers) in ARM and PCID (Process-Context IDentifiers) in x86 let multiple processes share TLB entries, avoiding full TLB flushes on context switch."

---

## Common Mistakes Students Make

### Mistake 1: Confusing Process State Transitions
❌ **WRONG:** "When I/O completes, the process goes from WAITING to RUNNING."
✅ **CORRECT:** "When I/O completes, the process goes from WAITING to **READY**. The scheduler may then pick it to run (READY → RUNNING), but only if no higher-priority process is waiting."

### Mistake 2: Thinking fork() copies all memory
❌ **WRONG:** "fork() copies all gigabytes of the parent's memory."
✅ **CORRECT:** "fork() uses Copy-on-Write. Physical pages are shared until one process writes. Only the page table is copied (O(1) practically), not the actual data."

### Mistake 3: SIGKILL can be caught
❌ **WRONG:** "I can write a signal handler to catch SIGKILL and prevent my process from being killed."
✅ **CORRECT:** "SIGKILL (signal 9) CANNOT be caught, blocked, or ignored — by design. The only way to 'resist' kill -9 is to be in uninterruptible sleep (D state) waiting for kernel I/O, but even then, kill -9 is pending and kills you when you wake."

### Mistake 4: Round Robin always gives better response time than FCFS
❌ **WRONG:** "Round Robin always has better average waiting time than FCFS."
✅ **CORRECT:** "Round Robin has better RESPONSE TIME (time to first response) for all processes. But for TURNAROUND TIME and AVERAGE WAITING TIME, it's often WORSE than SJF/FCFS for short jobs. A short job that runs immediately under SJF might wait several time quanta under RR."

### Mistake 5: Priority scheduling with lower number = lower priority
❌ **WRONG:** "Priority 1 is lower priority than priority 10."
✅ **CORRECT (Linux/Unix convention):** "Lower priority NUMBER = HIGHER priority. nice = -20 is highest priority, nice = +19 is lowest. Always clarify convention in interviews!"

---

## CODING EXERCISES

### Exercise 1: Implement FCFS Scheduler Simulation

```c
/* Complete FCFS Scheduler
 * Compile: gcc -Wall -o fcfs fcfs_scheduler.c
 */
#include <stdio.h>
#include <string.h>

typedef struct {
    int pid;          /* Process ID */
    int arrival;      /* Arrival time */
    int burst;        /* CPU burst time needed */
    int start;        /* When did it start running */
    int finish;       /* When did it finish */
    int waiting;      /* Time spent in ready queue */
    int turnaround;   /* Total time from arrival to finish */
} Process;

/* Compare by arrival time for sorting */
int cmp_arrival(const void *a, const void *b) {
    return ((Process*)a)->arrival - ((Process*)b)->arrival;
}

void fcfs(Process *procs, int n) {
    /* Sort by arrival time */
    /* (In this example, assume already sorted) */
    
    int current_time = 0;
    
    for (int i = 0; i < n; i++) {
        /* If CPU is idle, jump to next process's arrival */
        if (current_time < procs[i].arrival)
            current_time = procs[i].arrival;
        
        procs[i].start      = current_time;
        procs[i].finish     = current_time + procs[i].burst;
        procs[i].turnaround = procs[i].finish - procs[i].arrival;
        procs[i].waiting    = procs[i].turnaround - procs[i].burst;
        
        current_time = procs[i].finish;
    }
}

void print_results(Process *procs, int n) {
    printf("\n%-5s %-8s %-6s %-6s %-6s %-6s %-12s %-12s\n",
           "PID", "Arrival", "Burst", "Start", "End", "Wait", "TAT", "");
    printf("%-5s %-8s %-6s %-6s %-6s %-6s %-12s %-12s\n",
           "---", "-------", "-----", "-----", "---", "----", "-----------", "");
    
    double total_wait = 0, total_tat = 0;
    
    for (int i = 0; i < n; i++) {
        printf("%-5d %-8d %-6d %-6d %-6d %-6d %-12d\n",
               procs[i].pid, procs[i].arrival, procs[i].burst,
               procs[i].start, procs[i].finish,
               procs[i].waiting, procs[i].turnaround);
        total_wait += procs[i].waiting;
        total_tat  += procs[i].turnaround;
    }
    
    printf("\nAverage Waiting Time:    %.2f\n", total_wait / n);
    printf("Average Turnaround Time: %.2f\n", total_tat / n);
    
    /* Print Gantt chart */
    printf("\nGantt Chart:\n");
    printf("|");
    for (int i = 0; i < n; i++)
        printf(" P%-3d |", procs[i].pid);
    printf("\n");
    printf("0");
    for (int i = 0; i < n; i++)
        printf("      %d", procs[i].finish);
    printf("\n");
}

int main(void) {
    Process procs[] = {
        {1, 0, 7},   /* PID=1, arrives at t=0, needs 7ms */
        {2, 2, 4},   /* PID=2, arrives at t=2, needs 4ms */
        {3, 4, 1},   /* PID=3, arrives at t=4, needs 1ms */
        {4, 5, 4},   /* PID=4, arrives at t=5, needs 4ms */
    };
    int n = sizeof(procs) / sizeof(procs[0]);
    
    fcfs(procs, n);
    print_results(procs, n);
    
    return 0;
}
```

---

## Summary: Key Formulas

```
Turnaround Time (TAT)     = Completion Time - Arrival Time
Waiting Time (WT)         = TAT - Burst Time = TAT - CPU Time
Response Time             = First Run Time - Arrival Time
CPU Utilization           = (Total Busy Time / Total Time) × 100%
Throughput                = Number of Processes / Total Time

Exponential Average (SJF) = α × actual(n) + (1-α) × predicted(n)

EAT with TLB:
  EAT = α × (t_TLB + t_mem) + (1-α) × (t_TLB + 2×t_mem)
  where α = TLB hit ratio

CFS vruntime:
  vruntime += actual_time × (1024 / process_weight)
  weight(nice) = 1024 / (1.25 ^ nice)

RMS schedulability:
  CPU_util ≤ n × (2^(1/n) - 1) ≈ 0.693 as n→∞
```

---

*End of DOC_1 — Process, Thread & CPU Scheduling*
*Total coverage: OS Intro, Processes, Threads, CPU Scheduling algorithms (FCFS, SJF, SRTF, RR, Priority, MLFQ, CFS, EDF), Interview Q&A, MCQs, Numericals*

---
> **Next:** See DOC_2_Memory_Management_Virtual_Memory.md for paging, page replacement, virtual memory, TLB, and memory allocation algorithms.
