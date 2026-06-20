# OS Course — Master Makefile
# Usage: make all        (build everything)
#        make phase01    (build only Phase01)
#        make clean      (remove all binaries)
#
# Requirements: gcc, POSIX system (Linux/macOS), pthread library
#
# Windows users: Use WSL (Windows Subsystem for Linux)
# Install: sudo apt-get install gcc build-essential

CC      = gcc
CFLAGS  = -Wall -Wextra -g
PTHREAD = -pthread
LDFLAGS = -lrt

# ─── Phase01: Intro and Architecture ───────────────────────────
P01 = Phase01_Intro_And_Architecture
phase01: $(P01)/01_what_is_os $(P01)/02_kernel_vs_userspace \
         $(P01)/03_system_call_demo $(P01)/04_os_boot_sequence_simulation \
         $(P01)/09_complete_mini_shell

$(P01)/01_what_is_os: $(P01)/01_what_is_os.c
	$(CC) $(CFLAGS) -o $@ $<

$(P01)/02_kernel_vs_userspace: $(P01)/02_kernel_vs_userspace.c
	$(CC) $(CFLAGS) -o $@ $<

$(P01)/03_system_call_demo: $(P01)/03_system_call_demo.c
	$(CC) $(CFLAGS) -o $@ $<

$(P01)/04_os_boot_sequence_simulation: $(P01)/04_os_boot_sequence_simulation.c
	$(CC) $(CFLAGS) -o $@ $<

$(P01)/09_complete_mini_shell: $(P01)/09_complete_mini_shell.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase02: Process Management ──────────────────────────────
P02 = Phase02_Process_Management
phase02: $(P02)/01_process_concept $(P02)/03_process_states_simulation \
         $(P02)/06_zombie_orphan_process $(P02)/09_real_world_process_manager

$(P02)/01_process_concept: $(P02)/01_process_concept.c
	$(CC) $(CFLAGS) -o $@ $<

$(P02)/03_process_states_simulation: $(P02)/03_process_states_simulation.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<

$(P02)/06_zombie_orphan_process: $(P02)/06_zombie_orphan_process.c
	$(CC) $(CFLAGS) -o $@ $<

$(P02)/09_real_world_process_manager: $(P02)/09_real_world_process_manager.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase03: Threads and Concurrency ────────────────────────
P03 = Phase03_Threads_And_Concurrency
phase03: $(P03)/05_thread_pool_implementation $(P03)/06_race_condition_demo

$(P03)/05_thread_pool_implementation: $(P03)/05_thread_pool_implementation.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<

$(P03)/06_race_condition_demo: $(P03)/06_race_condition_demo.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<

# ─── Phase04: CPU Scheduling ──────────────────────────────────
P04 = Phase04_CPU_Scheduling
phase04: $(P04)/02_fcfs_scheduler

$(P04)/02_fcfs_scheduler: $(P04)/02_fcfs_scheduler.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase05: Synchronization ─────────────────────────────────
P05 = Phase05_Synchronization
phase05: $(P05)/07_dining_philosophers

$(P05)/07_dining_philosophers: $(P05)/07_dining_philosophers.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<

# ─── Phase06: Deadlocks ───────────────────────────────────────
P06 = Phase06_Deadlocks
phase06: $(P06)/03_bankers_algorithm

$(P06)/03_bankers_algorithm: $(P06)/03_bankers_algorithm.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase07: Memory Management ───────────────────────────────
P07 = Phase07_Memory_Management
phase07: $(P07)/10_malloc_implementation

$(P07)/10_malloc_implementation: $(P07)/10_malloc_implementation.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase08: Virtual Memory ──────────────────────────────────
P08 = Phase08_Virtual_Memory
phase08: $(P08)/05_page_replacement_algorithms

$(P08)/05_page_replacement_algorithms: $(P08)/05_page_replacement_algorithms.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase09: File Systems ────────────────────────────────────
P09 = Phase09_File_Systems
phase09: $(P09)/07_inode_structure

$(P09)/07_inode_structure: $(P09)/07_inode_structure.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase10: I/O and Storage ─────────────────────────────────
P10 = Phase10_IO_And_Storage
phase10: $(P10)/04_disk_scheduling

$(P10)/04_disk_scheduling: $(P10)/04_disk_scheduling.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase11: IPC ─────────────────────────────────────────────
P11 = Phase11_InterProcess_Communication
phase11: $(P11)/03_pipe_communication

$(P11)/03_pipe_communication: $(P11)/03_pipe_communication.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase12: Security ────────────────────────────────────────
P12 = Phase12_OS_Security_And_Protection
phase12: $(P12)/04_access_control_demo

$(P12)/04_access_control_demo: $(P12)/04_access_control_demo.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase13: Interview Questions ─────────────────────────────
P13 = Phase13_Interview_Questions
phase13: $(P13)/04_scheduling_predict_output

$(P13)/04_scheduling_predict_output: $(P13)/04_scheduling_predict_output.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase14: Capstone OS Design ──────────────────────────────
P14 = Phase14_OS_Design_Problems
phase14: $(P14)/10_complete_os_simulation

$(P14)/10_complete_os_simulation: $(P14)/10_complete_os_simulation.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<

# ─── All phases ───────────────────────────────────────────────
all: phase01 phase02 phase03 phase04 phase05 phase06 phase07 \
     phase08 phase09 phase10 phase11 phase12 phase13 phase14
	@echo ""
	@echo "╔══════════════════════════════════════╗"
	@echo "║   ALL OS COURSE PROGRAMS COMPILED!    ║"
	@echo "╚══════════════════════════════════════╝"

# ─── Clean ────────────────────────────────────────────────────
clean:
	find . -maxdepth 2 -type f ! -name "*.c" ! -name "*.md" ! -name "Makefile" \
	       -not -name ".*" -delete
	@echo "Cleaned all compiled binaries."

.PHONY: all clean phase01 phase02 phase03 phase04 phase05 phase06 \
        phase07 phase08 phase09 phase10 phase11 phase12 phase13 phase14
