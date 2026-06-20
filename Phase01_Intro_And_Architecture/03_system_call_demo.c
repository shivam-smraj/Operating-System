/* Phase01/03_system_call_demo.c
 * TOPIC: System Calls — The Gate Between User Space and Kernel
 * Compile: gcc -Wall -o syscall_demo 03_system_call_demo.c
 * Study: Use "strace ./syscall_demo" to see ALL system calls made!
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>

void demo_file_syscalls(void) {
    printf("\n--- File System Calls ---\n");
    /* open(): syscall number 2 */
    int fd = open("/tmp/syscall_test.txt",
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0644);   /* permissions: rw-r--r-- */
    if (fd < 0) { perror("open"); return; }
    printf("open() returned fd=%d\n", fd);

    /* write(): syscall number 1 */
    const char *data = "Hello, OS World!\n";
    ssize_t written = write(fd, data, strlen(data));
    printf("write() wrote %zd bytes\n", written);

    /* fsync(): flush to disk */
    fsync(fd);
    printf("fsync() flushed to disk\n");

    /* close(): syscall number 3 */
    close(fd);
    printf("close() closed fd\n");

    /* open for reading */
    fd = open("/tmp/syscall_test.txt", O_RDONLY);
    char buf[100] = {0};
    /* read(): syscall number 0 */
    ssize_t n = read(fd, buf, sizeof(buf)-1);
    printf("read() got: '%s'\n", buf);
    close(fd);
    unlink("/tmp/syscall_test.txt");  /* Delete file */
}

void demo_process_syscalls(void) {
    printf("\n--- Process System Calls ---\n");
    printf("getpid()  = %d\n", getpid());
    printf("getppid() = %d\n", getppid());
    printf("getuid()  = %d\n", getuid());
    printf("getgid()  = %d\n", getgid());
    /* geteuid(): effective user ID (might differ if setuid) */
    printf("geteuid() = %d (effective UID)\n", geteuid());
}

void demo_memory_syscalls(void) {
    printf("\n--- Memory System Calls ---\n");
    /* brk()/sbrk(): old heap management (used by old malloc) */
    void *old_brk = sbrk(0);  /* Get current break address */
    printf("Current program break: %p\n", old_brk);

    /* mmap(): map anonymous memory (what modern malloc uses) */
    #include <sys/mman.h>
    void *mem = mmap(NULL,        /* Let OS choose address */
                     4096,        /* 4KB */
                     PROT_READ | PROT_WRITE,  /* Permissions */
                     MAP_PRIVATE | MAP_ANONYMOUS, /* Not backed by file */
                     -1,          /* No file (anonymous) */
                     0);          /* No offset */
    if (mem == MAP_FAILED) { perror("mmap"); return; }
    printf("mmap() allocated 4KB at %p\n", mem);

    /* Use the memory */
    *(int*)mem = 42;
    printf("Wrote 42 to mmap'd memory, read back: %d\n", *(int*)mem);

    /* munmap(): return memory to OS */
    munmap(mem, 4096);
    printf("munmap() returned memory to OS\n");
}

int main(void) {
    printf("=== System Call Demo ===\n");
    printf("Run 'strace ./syscall_demo' to see all system calls!\n");

    demo_file_syscalls();
    demo_process_syscalls();
    demo_memory_syscalls();

    printf("\n[KEY SYSTEM CALL NUMBERS on x86-64 Linux]\n");
    printf("  read   = 0,  write  = 1,  open   = 2,  close = 3\n");
    printf("  stat   = 4,  mmap   = 9,  munmap = 11, brk   = 12\n");
    printf("  fork   = 57, execve = 59, exit   = 60, wait4 = 61\n");
    printf("  getpid = 39, getuid = 102, socket = 41\n");

    return 0;
}
