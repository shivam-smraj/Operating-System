/* Phase01/09_complete_mini_shell.c
 * TOPIC: Mini Shell — Combining all Phase01 concepts
 * Compile: gcc -Wall -o minishell 09_complete_mini_shell.c
 * Run: ./minishell
 *
 * This implements a minimal but functional Unix shell demonstrating:
 * 1. fork() — creating child processes
 * 2. exec() — executing programs
 * 3. wait() — waiting for child completion
 * 4. Pipes — connecting command input/output
 * 5. Signal handling — Ctrl+C behavior
 * 6. Built-in commands: cd, exit, help
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_INPUT  1024
#define MAX_ARGS   64
#define MAX_PIPES  8

/* Signal handler for SIGINT (Ctrl+C) */
void sigint_handler(int sig) {
    (void)sig;
    /* Shell itself ignores Ctrl+C; child process gets it */
    printf("\n[minishell] Ctrl+C: type 'exit' to quit\n");
    printf("$ ");
    fflush(stdout);
}

/* Parse input into array of argument strings */
int parse_args(char *input, char **args, int max_args) {
    int count = 0;
    char *token = strtok(input, " \t\n");
    while (token && count < max_args - 1) {
        args[count++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[count] = NULL;  /* execvp() needs NULL-terminated array */
    return count;
}

/* Built-in: cd */
int builtin_cd(char **args) {
    if (args[1] == NULL) {
        /* cd with no args → go to HOME */
        const char *home = getenv("HOME");
        if (home) chdir(home);
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
            return 1;
        }
    }
    return 0;
}

/* Execute a single command with optional I/O redirection */
void execute_command(char **args, int input_fd, int output_fd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        /* CHILD PROCESS */
        /* Set up I/O redirection */
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        /* Restore SIGINT handling for child */
        signal(SIGINT, SIG_DFL);
        /* Replace child process image with the requested program */
        execvp(args[0], args);
        /* If execvp returns: command not found */
        fprintf(stderr, "minishell: %s: command not found\n", args[0]);
        exit(127);  /* 127 = command not found (convention) */
    }
    /* PARENT: wait for child */
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        /* Print exit code for non-zero exits */
        printf("[exit code: %d]\n", WEXITSTATUS(status));
    }
}

/* Execute command pipeline: cmd1 | cmd2 | cmd3 */
void execute_pipeline(char *commands[], int num_cmds) {
    int pipes[MAX_PIPES][2];  /* File descriptors for pipes */
    int num_pipes = num_cmds - 1;

    /* Create all needed pipes */
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) { perror("pipe"); return; }
    }

    for (int i = 0; i < num_cmds; i++) {
        char *args[MAX_ARGS];
        char cmd_copy[MAX_INPUT];
        strncpy(cmd_copy, commands[i], MAX_INPUT-1);
        parse_args(cmd_copy, args, MAX_ARGS);

        /* Determine input and output file descriptors */
        int input_fd  = (i == 0)           ? STDIN_FILENO  : pipes[i-1][0];
        int output_fd = (i == num_cmds-1)  ? STDOUT_FILENO : pipes[i][1];

        pid_t pid = fork();
        if (pid == 0) {
            /* Set up pipe I/O */
            if (input_fd != STDIN_FILENO) {
                dup2(input_fd, STDIN_FILENO);
            }
            if (output_fd != STDOUT_FILENO) {
                dup2(output_fd, STDOUT_FILENO);
            }
            /* Close ALL pipe fds in child (only use the dup2'd ones) */
            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            signal(SIGINT, SIG_DFL);
            execvp(args[0], args);
            fprintf(stderr, "%s: command not found\n", args[0]);
            exit(127);
        }
    }

    /* Parent: close all pipe fds and wait for all children */
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    for (int i = 0; i < num_cmds; i++)
        wait(NULL);
}

int main(void) {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    char hostname[256] = "host";

    /* Get hostname for prompt */
    gethostname(hostname, sizeof(hostname));

    /* Install signal handler — shell ignores Ctrl+C */
    signal(SIGINT, sigint_handler);

    printf("=== MiniShell v1.0 ===\n");
    printf("Supports: commands, pipes (|), built-ins (cd, exit, help)\n");
    printf("Type 'help' for help, 'exit' to quit.\n\n");

    while (1) {
        /* Display prompt: user@host:cwd$ */
        char cwd[256];
        getcwd(cwd, sizeof(cwd));
        printf("[minishell] %s $ ", cwd);
        fflush(stdout);

        /* Read input */
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;  /* EOF (Ctrl+D) */
        }

        /* Remove trailing newline */
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;  /* Empty line */

        /* ── Built-in commands ── */
        char input_copy[MAX_INPUT];
        strncpy(input_copy, input, MAX_INPUT-1);
        int argc = parse_args(input_copy, args, MAX_ARGS);
        if (argc == 0) continue;

        if (strcmp(args[0], "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }
        if (strcmp(args[0], "cd") == 0) {
            builtin_cd(args);
            continue;
        }
        if (strcmp(args[0], "help") == 0) {
            printf("MiniShell built-ins:\n");
            printf("  cd [dir]  — change directory\n");
            printf("  exit      — exit the shell\n");
            printf("  help      — this help\n");
            printf("Any other command: fork+exec\n");
            printf("Pipe: cmd1 | cmd2 | cmd3\n");
            continue;
        }

        /* ── Check for pipes ── */
        if (strchr(input, '|') != NULL) {
            char *commands[MAX_PIPES];
            char pipe_copy[MAX_INPUT];
            strncpy(pipe_copy, input, MAX_INPUT-1);
            int num_cmds = 0;
            char *cmd = strtok(pipe_copy, "|");
            while (cmd && num_cmds < MAX_PIPES) {
                commands[num_cmds++] = cmd;
                cmd = strtok(NULL, "|");
            }
            execute_pipeline(commands, num_cmds);
        } else {
            /* ── Single command ── */
            strncpy(input_copy, input, MAX_INPUT-1);
            argc = parse_args(input_copy, args, MAX_ARGS);
            execute_command(args, STDIN_FILENO, STDOUT_FILENO);
        }
    }
    return 0;
}
