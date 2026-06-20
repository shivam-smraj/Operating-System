/* Phase11/03_pipe_communication.c
 * Demonstrates anonymous pipes and named pipes (FIFOs) for IPC
 * Compile: gcc -Wall -o pipes 03_pipe_communication.c
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

void demo_anonymous_pipe(void) {
    printf("=== Anonymous Pipe (parent ↔ child) ===\n");
    int pipefd[2];  /* [0]=read, [1]=write */
    pipe(pipefd);

    pid_t pid = fork();
    if(pid == 0) {
        /* Child: read from pipe */
        close(pipefd[1]);  /* Close unused write end */
        char buf[128] = {0};
        ssize_t n = read(pipefd[0], buf, sizeof(buf)-1);
        printf("Child received (%zd bytes): '%s'\n", n, buf);
        close(pipefd[0]);
        return;
    }
    /* Parent: write to pipe */
    close(pipefd[0]);  /* Close unused read end */
    const char *msg = "Hello from parent via pipe!";
    write(pipefd[1], msg, strlen(msg));
    close(pipefd[1]);  /* IMPORTANT: close so child gets EOF */
    wait(NULL);
}

void demo_pipe_chain(void) {
    printf("\n=== Pipe Chain: ls | wc -l ===\n");
    int pipefd[2]; pipe(pipefd);

    pid_t pid = fork();
    if(pid == 0) {
        /* Child runs 'wc -l', reading from pipe */
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);  /* Stdin = pipe read end */
        close(pipefd[0]);
        execlp("wc", "wc", "-l", NULL);
        return;
    }

    pid_t pid2 = fork();
    if(pid2 == 0) {
        /* Grandchild runs 'ls', writing to pipe */
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);  /* Stdout = pipe write end */
        close(pipefd[1]);
        execlp("ls", "ls", "/etc", NULL);
        return;
    }

    close(pipefd[0]); close(pipefd[1]);
    wait(NULL); wait(NULL);
}

int main(void) {
    printf("╔═══════════════════════════════╗\n");
    printf("║   PIPE IPC DEMONSTRATION       ║\n");
    printf("╚═══════════════════════════════╝\n\n");

    demo_anonymous_pipe();
    demo_pipe_chain();

    printf("\n[Pipe Properties]\n");
    printf("- Unidirectional: one end reads, other writes\n");
    printf("- Byte stream: no message boundaries\n");
    printf("- Blocking: write blocks if full, read blocks if empty\n");
    printf("- Anonymous pipe: only between related processes (parent-child)\n");
    printf("- Named pipe (FIFO): any two processes by filename\n");
    return 0;
}
