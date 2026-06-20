/* Phase11/06_message_queue_ipc.c
 * TOPIC: POSIX Message Queue IPC
 * Compile: gcc -Wall -lrt -o mq_demo 06_message_queue_ipc.c
 * Run: ./mq_demo sender  (in one terminal)
 *      ./mq_demo receiver (in another)
 * Or:  ./mq_demo         (self-contained fork demo)
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MQ_NAME    "/os_course_mq"
#define MAX_MSGS   10
#define MAX_MSG_SZ 256

/* Message structure with priority */
typedef struct {
    int  priority;
    int  seq;
    char text[200];
} Message;

void do_sender(void) {
    printf("[Sender] Opening message queue '%s'...\n", MQ_NAME);

    struct mq_attr attr = {
        .mq_flags   = 0,
        .mq_maxmsg  = MAX_MSGS,
        .mq_msgsize = sizeof(Message),
        .mq_curmsgs = 0,
    };

    mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq == (mqd_t)-1) { perror("mq_open (sender)"); exit(1); }

    /* Send messages with different priorities */
    Message msgs[] = {
        {1, 1, "Low priority: routine log message"},
        {5, 2, "Medium priority: status update"},
        {9, 3, "HIGH PRIORITY: critical alert!"},
        {3, 4, "Low-medium: background task result"},
        {7, 5, "High: user request response"},
    };

    for (int i = 0; i < 5; i++) {
        printf("[Sender] Sending (prio=%d): '%s'\n",
               msgs[i].priority, msgs[i].text);
        if (mq_send(mq, (char*)&msgs[i], sizeof(Message), msgs[i].priority) < 0)
            perror("mq_send");
        usleep(100000);
    }

    mq_close(mq);
    printf("[Sender] Done sending 5 messages.\n");
}

void do_receiver(void) {
    usleep(200000);  /* Wait for sender to create queue */
    printf("[Receiver] Opening message queue '%s'...\n", MQ_NAME);

    mqd_t mq = mq_open(MQ_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) { perror("mq_open (receiver)"); exit(1); }

    struct mq_attr attr;
    mq_getattr(mq, &attr);
    printf("[Receiver] Queue info: maxmsg=%ld msgsize=%ld\n",
           attr.mq_maxmsg, attr.mq_msgsize);
    printf("[Receiver] Receiving messages (NOTE: highest priority first!)\n\n");

    Message msg;
    unsigned int prio;
    for (int i = 0; i < 5; i++) {
        ssize_t n = mq_receive(mq, (char*)&msg, sizeof(Message), &prio);
        if (n < 0) { perror("mq_receive"); break; }
        printf("[Receiver] Got (prio=%u, seq=%d): '%s'\n",
               prio, msg.seq, msg.text);
    }

    mq_close(mq);
    mq_unlink(MQ_NAME);  /* Remove the queue */
    printf("\n[Receiver] Done. Queue removed.\n");
    printf("NOTE: Messages arrived in PRIORITY ORDER (highest first),\n");
    printf("      NOT in the order they were sent!\n");
}

int main(int argc, char *argv[]) {
    printf("=== POSIX Message Queue IPC Demo ===\n");
    printf("Queue: %s  (max %d messages, %d bytes each)\n\n",
           MQ_NAME, MAX_MSGS, MAX_MSG_SZ);

    if (argc > 1 && strcmp(argv[1], "sender") == 0) {
        do_sender();
    } else if (argc > 1 && strcmp(argv[1], "receiver") == 0) {
        do_receiver();
    } else {
        /* Self-contained demo: fork sender and receiver */
        printf("Running self-contained demo (fork)...\n\n");

        /* First clean up any leftover queue */
        mq_unlink(MQ_NAME);

        pid_t pid = fork();
        if (pid == 0) {
            do_receiver();
            exit(0);
        }
        do_sender();
        wait(NULL);
    }

    printf("\n[Message Queue vs Pipe vs Shared Memory]\n");
    printf("Message Queue:\n");
    printf("  + Message boundaries preserved (unlike pipes)\n");
    printf("  + Priority ordering\n");
    printf("  + Persistent until unlink (survives if process crashes)\n");
    printf("  + Any process can open by name\n");
    printf("  - Slower than shared memory\n");
    printf("  - Size limits\n\n");
    printf("Use case: job queues, event notification, request-response IPC\n");
    return 0;
}
