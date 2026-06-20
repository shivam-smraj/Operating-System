# OS Course — Master Makefile
# Usage:
#   make all        → Build every program in all phases
#   make phase01    → Build only Phase01
#   make phase02    → Build only Phase02
#   ...
#   make clean      → Remove all compiled binaries
#
# Requirements:
#   Linux/WSL:   sudo apt-get install gcc build-essential
#   macOS:       xcode-select --install
#   Windows:     Use WSL (Windows Subsystem for Linux)
#
# Note: -lrt is needed for POSIX shared memory and message queues (Linux)
#       On macOS: remove -lrt (not needed)

CC       = gcc
CFLAGS   = -Wall -Wextra -g -std=c11
PTHREAD  = -pthread
LDFLAGS  = -lrt

# ─── Phase01: Intro and Architecture ────────────────────────────────────────
P01 = Phase01_Intro_And_Architecture
phase01: \
	$(P01)/01_what_is_os \
	$(P01)/02_kernel_vs_userspace \
	$(P01)/03_system_call_demo \
	$(P01)/04_os_boot_sequence_simulation \
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

# ─── Phase02: Process Management ────────────────────────────────────────────
P02 = Phase02_Process_Management
phase02: \
	$(P02)/01_process_concept \
	$(P02)/02_fork_exec_wait \
	$(P02)/03_process_states_simulation \
	$(P02)/04_pcb_and_context_switch \
	$(P02)/06_zombie_orphan_process

$(P02)/01_process_concept: $(P02)/01_process_concept.c
	$(CC) $(CFLAGS) -o $@ $<
$(P02)/02_fork_exec_wait: $(P02)/02_fork_exec_wait.c
	$(CC) $(CFLAGS) -o $@ $<
$(P02)/03_process_states_simulation: $(P02)/03_process_states_simulation.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<
$(P02)/04_pcb_and_context_switch: $(P02)/04_pcb_and_context_switch.c
	$(CC) $(CFLAGS) -o $@ $<
$(P02)/06_zombie_orphan_process: $(P02)/06_zombie_orphan_process.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase03: Threads and Concurrency ───────────────────────────────────────
P03 = Phase03_Threads_And_Concurrency
phase03: \
	$(P03)/01_pthread_basics \
	$(P03)/03_mutex_and_condition_variable \
	$(P03)/05_thread_pool_implementation \
	$(P03)/06_race_condition_demo

$(P03)/01_pthread_basics: $(P03)/01_pthread_basics.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<
$(P03)/03_mutex_and_condition_variable: $(P03)/03_mutex_and_condition_variable.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<
$(P03)/05_thread_pool_implementation: $(P03)/05_thread_pool_implementation.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<
$(P03)/06_race_condition_demo: $(P03)/06_race_condition_demo.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<

# ─── Phase04: CPU Scheduling ─────────────────────────────────────────────────
P04 = Phase04_CPU_Scheduling
phase04: \
	$(P04)/02_fcfs_scheduler \
	$(P04)/05_all_schedulers_comparison

$(P04)/02_fcfs_scheduler: $(P04)/02_fcfs_scheduler.c
	$(CC) $(CFLAGS) -o $@ $<
$(P04)/05_all_schedulers_comparison: $(P04)/05_all_schedulers_comparison.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase05: Synchronization ─────────────────────────────────────────────────
P05 = Phase05_Synchronization
phase05: \
	$(P05)/01_mutex_deep_dive \
	$(P05)/03_semaphore_implementation \
	$(P05)/07_dining_philosophers

$(P05)/01_mutex_deep_dive: $(P05)/01_mutex_deep_dive.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<
$(P05)/03_semaphore_implementation: $(P05)/03_semaphore_implementation.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<
$(P05)/07_dining_philosophers: $(P05)/07_dining_philosophers.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<

# ─── Phase06: Deadlocks ───────────────────────────────────────────────────────
P06 = Phase06_Deadlocks
phase06: \
	$(P06)/01_deadlock_simulation \
	$(P06)/03_bankers_algorithm

$(P06)/01_deadlock_simulation: $(P06)/01_deadlock_simulation.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<
$(P06)/03_bankers_algorithm: $(P06)/03_bankers_algorithm.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase07: Memory Management ──────────────────────────────────────────────
P07 = Phase07_Memory_Management
phase07: \
	$(P07)/03_paging_simulation \
	$(P07)/10_malloc_implementation

$(P07)/03_paging_simulation: $(P07)/03_paging_simulation.c
	$(CC) $(CFLAGS) -o $@ $<
$(P07)/10_malloc_implementation: $(P07)/10_malloc_implementation.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase08: Virtual Memory ──────────────────────────────────────────────────
P08 = Phase08_Virtual_Memory
phase08: $(P08)/05_page_replacement_algorithms

$(P08)/05_page_replacement_algorithms: $(P08)/05_page_replacement_algorithms.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase09: File Systems ────────────────────────────────────────────────────
P09 = Phase09_File_Systems
phase09: $(P09)/05_file_system_simulation

$(P09)/05_file_system_simulation: $(P09)/05_file_system_simulation.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase10: I/O and Storage ─────────────────────────────────────────────────
P10 = Phase10_IO_And_Storage
phase10: $(P10)/04_disk_scheduling

$(P10)/04_disk_scheduling: $(P10)/04_disk_scheduling.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase11: IPC ─────────────────────────────────────────────────────────────
P11 = Phase11_InterProcess_Communication
phase11: \
	$(P11)/03_pipe_communication \
	$(P11)/06_message_queue_ipc

$(P11)/03_pipe_communication: $(P11)/03_pipe_communication.c
	$(CC) $(CFLAGS) -o $@ $<
$(P11)/06_message_queue_ipc: $(P11)/06_message_queue_ipc.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

# ─── Phase12: Security ────────────────────────────────────────────────────────
P12 = Phase12_OS_Security_And_Protection
phase12: $(P12)/04_access_control_demo

$(P12)/04_access_control_demo: $(P12)/04_access_control_demo.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase13: Interview Questions ─────────────────────────────────────────────
P13 = Phase13_Interview_Questions
phase13: \
	$(P13)/04_scheduling_predict_output \
	$(P13)/07_memory_interview_problems

$(P13)/04_scheduling_predict_output: $(P13)/04_scheduling_predict_output.c
	$(CC) $(CFLAGS) -o $@ $<
$(P13)/07_memory_interview_problems: $(P13)/07_memory_interview_problems.c
	$(CC) $(CFLAGS) -o $@ $<

# ─── Phase14: OS Design Problems ──────────────────────────────────────────────
P14 = Phase14_OS_Design_Problems
phase14: \
	$(P14)/05_linux_kernel_concepts \
	$(P14)/10_complete_os_simulation

$(P14)/05_linux_kernel_concepts: $(P14)/05_linux_kernel_concepts.c
	$(CC) $(CFLAGS) -o $@ $<
$(P14)/10_complete_os_simulation: $(P14)/10_complete_os_simulation.c
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $<

# ─── Build everything ─────────────────────────────────────────────────────────
all: phase01 phase02 phase03 phase04 phase05 phase06 phase07 \
     phase08 phase09 phase10 phase11 phase12 phase13 phase14
	@echo ""
	@echo "╔════════════════════════════════════════════════╗"
	@echo "║    ALL 40 OS COURSE PROGRAMS COMPILED!          ║"
	@echo "║                                                  ║"
	@echo "║    Start learning: ./Phase01_.../01_what_is_os   ║"
	@echo "║    Full guide:    cat README.md                  ║"
	@echo "╚════════════════════════════════════════════════╝"

# ─── Quick demo targets ────────────────────────────────────────────────────────
demo-scheduling: phase04
	@echo "=== Running all schedulers comparison ==="
	./$(P04)/05_all_schedulers_comparison

demo-memory: phase07 phase08
	@echo "=== Running page replacement ==="
	./$(P08)/05_page_replacement_algorithms

demo-deadlock: phase06
	@echo "=== Running Banker's Algorithm ==="
	./$(P06)/03_bankers_algorithm

demo-capstone: phase14
	@echo "=== Running Mini OS Simulation ==="
	./$(P14)/10_complete_os_simulation

# ─── Clean ────────────────────────────────────────────────────────────────────
clean:
	@echo "Cleaning compiled binaries..."
	@find . -maxdepth 2 \( \
		-name "01_what_is_os" \
		-o -name "02_kernel_vs_userspace" \
		-o -name "03_system_call_demo" \
		-o -name "04_os_boot_sequence_simulation" \
		-o -name "09_complete_mini_shell" \
		-o -name "01_process_concept" \
		-o -name "02_fork_exec_wait" \
		-o -name "03_process_states_simulation" \
		-o -name "04_pcb_and_context_switch" \
		-o -name "06_zombie_orphan_process" \
		-o -name "01_pthread_basics" \
		-o -name "03_mutex_and_condition_variable" \
		-o -name "05_thread_pool_implementation" \
		-o -name "06_race_condition_demo" \
		-o -name "02_fcfs_scheduler" \
		-o -name "05_all_schedulers_comparison" \
		-o -name "01_mutex_deep_dive" \
		-o -name "03_semaphore_implementation" \
		-o -name "07_dining_philosophers" \
		-o -name "01_deadlock_simulation" \
		-o -name "03_bankers_algorithm" \
		-o -name "03_paging_simulation" \
		-o -name "10_malloc_implementation" \
		-o -name "05_page_replacement_algorithms" \
		-o -name "05_file_system_simulation" \
		-o -name "04_disk_scheduling" \
		-o -name "03_pipe_communication" \
		-o -name "06_message_queue_ipc" \
		-o -name "04_access_control_demo" \
		-o -name "04_scheduling_predict_output" \
		-o -name "07_memory_interview_problems" \
		-o -name "05_linux_kernel_concepts" \
		-o -name "10_complete_os_simulation" \
	\) -type f -delete
	@echo "Done."

.PHONY: all clean \
	phase01 phase02 phase03 phase04 phase05 phase06 phase07 \
	phase08 phase09 phase10 phase11 phase12 phase13 phase14 \
	demo-scheduling demo-memory demo-deadlock demo-capstone
