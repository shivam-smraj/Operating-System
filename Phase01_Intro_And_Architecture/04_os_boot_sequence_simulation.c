/* Phase01/04_os_boot_sequence_simulation.c
 * TOPIC: OS Boot Sequence — From Power-On to User Space
 * Compile: gcc -Wall -o boot_sim 04_os_boot_sequence_simulation.c
 *
 * This is a SIMULATION of the boot process.
 * It models each phase with print statements and artificial delays.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void delay_ms(int ms) {
    struct timespec ts = { .tv_sec=0, .tv_nsec = ms * 1000000L };
    nanosleep(&ts, NULL);
}

void print_step(int step, const char *phase, const char *detail) {
    printf("[Step %2d] [%-20s] %s\n", step, phase, detail);
    delay_ms(50);  /* Small delay for visual effect */
}

void phase_bios_uefi(void) {
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║    PHASE 1: BIOS/UEFI (Firmware in ROM)       ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    print_step(1, "POWER ON", "CPU starts executing from fixed address 0xFFFFFFF0 (BIOS entry)");
    print_step(2, "POST", "Testing CPU registers: PASS");
    print_step(3, "POST", "Testing RAM: 16GB detected, PASS");
    print_step(4, "POST", "Testing keyboard controller: PASS");
    print_step(5, "POST", "Detecting storage: NVMe SSD at PCIe slot 0: FOUND");
    print_step(6, "POST", "Detecting GPU: NVIDIA GeForce: FOUND");
    print_step(7, "UEFI", "Reading UEFI boot entries from NVRAM");
    print_step(8, "UEFI", "Boot entry 1: ubuntu /EFI/ubuntu/grubx64.efi");
    print_step(9, "UEFI", "Loading GRUB bootloader from EFI partition...");
}

void phase_bootloader(void) {
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║    PHASE 2: BOOTLOADER (GRUB2)                ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    print_step(10, "GRUB2", "GRUB Stage 1 loaded (446 bytes from MBR or UEFI)");
    print_step(11, "GRUB2", "Loading /boot/grub/grub.cfg...");
    print_step(12, "GRUB2", "Displaying boot menu (5 second timeout)...");
    print_step(13, "GRUB2", "User selected: Ubuntu 22.04 LTS (kernel 5.15.0-76)");
    print_step(14, "GRUB2", "Loading kernel: /boot/vmlinuz-5.15.0-76-generic");
    print_step(15, "GRUB2", "Kernel size: 12.4 MB. Decompressing...");
    print_step(16, "GRUB2", "Loading initramfs: /boot/initrd.img-5.15.0-76-generic");
    print_step(17, "GRUB2", "initramfs size: 75 MB. Loaded into RAM.");
    print_step(18, "GRUB2", "Passing kernel parameters: root=/dev/nvme0n1p2 quiet splash");
}

void phase_kernel_init(void) {
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║    PHASE 3: KERNEL INITIALIZATION             ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    print_step(19, "head.S", "First kernel assembly: setting up GDT (Global Descriptor Table)");
    print_step(20, "head.S", "Setting up IDT (Interrupt Descriptor Table)");
    print_step(21, "head.S", "Enabling protected mode / long mode (64-bit)");
    print_step(22, "head.S", "Setting up initial page tables (identity mapping)");
    print_step(23, "start_kernel()", "main.c: start_kernel() called — entering C code!");
    print_step(24, "cpu_init()", "Per-CPU data structures initialized for CPU 0");
    print_step(25, "mm_init()", "Memory zones: DMA=16MB, Normal=4GB, HighMem=rest");
    print_step(26, "mm_init()", "Buddy allocator initialized. Free pages: 3,921,804");
    print_step(27, "sched_init()", "Scheduler: CFS initialized. Per-CPU run queues created.");
    print_step(28, "vfs_caches_init()", "VFS: dentry cache and inode cache initialized");
    print_step(29, "signals_init()", "Signal tables initialized");
    print_step(30, "drivers", "Loading NVMe driver... OK");
    print_step(31, "drivers", "Loading ext4 filesystem module... OK");
    print_step(32, "initramfs", "Mounting initramfs as temporary root /");
    print_step(33, "initramfs", "Running /init from initramfs...");
    print_step(34, "real_root", "Mounting real root filesystem: /dev/nvme0n1p2 (ext4)");
    print_step(35, "pivot_root", "Switching root from initramfs to real filesystem");
}

void phase_userspace(void) {
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║    PHASE 4: USER SPACE (PID 1 and Services)  ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    print_step(36, "systemd(PID=1)", "/sbin/init (systemd): PID 1 started!");
    print_step(37, "systemd", "Reading unit files from /etc/systemd/system/");
    print_step(38, "systemd", "Mounting: /proc (procfs), /sys (sysfs), /dev (devtmpfs)");
    print_step(39, "systemd", "Starting: systemd-udevd (device events)... OK");
    print_step(40, "systemd", "Starting: networking.service... OK");
    print_step(41, "systemd", "Starting: sshd.service... OK");
    print_step(42, "systemd", "Starting: cron.service... OK");
    print_step(43, "systemd", "Starting: getty@tty1.service (login prompt)... OK");
    print_step(44, "getty", "Displaying login: prompt on tty1");
    print_step(45, "login", "User types: nilesh");
    print_step(46, "PAM", "Pluggable Authentication Modules checking password...");
    print_step(47, "PAM", "Authentication: SUCCESSFUL");
    print_step(48, "bash", "Starting /bin/bash for user nilesh");
    print_step(49, "bash", "Loading ~/.bashrc, ~/.profile...");
    print_step(50, "READY!", "$ ← Shell prompt ready! Boot complete.");
}

int main(void) {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   OS BOOT SEQUENCE SIMULATION                 ║\n");
    printf("║   Power-On to Login Screen                    ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("(This simulates what happens in ~10 seconds on real hardware)\n");

    phase_bios_uefi();
    phase_bootloader();
    phase_kernel_init();
    phase_userspace();

    printf("\n[BOOT COMPLETE]\n");
    printf("Total simulated steps: 50\n");
    printf("Real boot takes 5-30 seconds depending on hardware.\n");

    return 0;
}
