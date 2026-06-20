/* Phase12/04_access_control_demo.c
 * Demonstrates Unix access control: permissions, setuid, capabilities
 * Compile: gcc -Wall -o acl_demo 04_access_control_demo.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void check_access(const char *path, const char *label) {
    printf("\n--- %s: %s ---\n", label, path);

    /* Test R/W/X access */
    printf("access(R): %s\n", access(path, R_OK)==0 ? "ALLOWED" : strerror(errno));
    printf("access(W): %s\n", access(path, W_OK)==0 ? "ALLOWED" : strerror(errno));
    printf("access(X): %s\n", access(path, X_OK)==0 ? "ALLOWED" : strerror(errno));

    struct stat st;
    if(stat(path, &st) == 0) {
        char perm[11]="----------";
        if(S_ISDIR(st.st_mode))  perm[0]='d';
        if(S_ISLNK(st.st_mode))  perm[0]='l';
        if(st.st_mode&S_IRUSR)   perm[1]='r';
        if(st.st_mode&S_IWUSR)   perm[2]='w';
        if(st.st_mode&S_IXUSR)   perm[3]='x';
        if(st.st_mode&S_IRGRP)   perm[4]='r';
        if(st.st_mode&S_IWGRP)   perm[5]='w';
        if(st.st_mode&S_IXGRP)   perm[6]='x';
        if(st.st_mode&S_IROTH)   perm[7]='r';
        if(st.st_mode&S_IWOTH)   perm[8]='w';
        if(st.st_mode&S_IXOTH)   perm[9]='x';
        if(st.st_mode&S_ISUID)   perm[3]='s';
        if(st.st_mode&S_ISGID)   perm[6]='s';
        if(st.st_mode&S_ISVTX)   perm[9]='t';
        printf("Permissions: %s (%04o)  UID=%d GID=%d\n",
               perm, st.st_mode&07777, st.st_uid, st.st_gid);
    }
}

void demonstrate_file_creation(void) {
    printf("\n=== File Creation and Permissions ===\n");
    const char *f1 = "/tmp/test_mode644";
    const char *f2 = "/tmp/test_mode700";
    const char *f3 = "/tmp/test_sticky_dir";

    /* Create files with specific permissions */
    int fd;
    fd = open(f1, O_CREAT|O_WRONLY|O_TRUNC, 0644);
    if(fd>=0){ write(fd,"test",4); close(fd); }
    fd = open(f2, O_CREAT|O_WRONLY|O_TRUNC, 0700);
    if(fd>=0){ write(fd,"secret",6); close(fd); }

    mkdir(f3, 01777);  /* Sticky bit = 1777 */

    check_access(f1, "File 0644 (rw-r--r--)");
    check_access(f2, "File 0700 (rwx------)");
    check_access(f3, "Dir  1777 (rwxrwxrwt)");

    /* Cleanup */
    unlink(f1); unlink(f2); rmdir(f3);
}

void explain_setuid(void) {
    printf("\n=== SetUID Bit Explanation ===\n");
    printf("SetUID on executable: runs with FILE OWNER's UID\n\n");

    /* Check /usr/bin/passwd setuid bit */
    struct stat st;
    if(stat("/usr/bin/passwd", &st) == 0) {
        printf("/usr/bin/passwd:\n");
        printf("  Owner: root (UID 0)\n");
        printf("  SetUID bit: %s\n",
               (st.st_mode & S_ISUID) ? "SET (s)" : "not set");
        printf("  Permissions: %04o\n", st.st_mode & 07777);
        printf("  Effect: Any user who runs passwd gets TEMPORARY root access\n");
        printf("          to modify /etc/shadow (which is only writable by root)\n");
    }

    printf("\nSecurity implication:\n");
    printf("  If passwd had a buffer overflow bug, attacker gets ROOT!\n");
    printf("  Modern systems use: capabilities, namespaces, seccomp\n");
    printf("  to reduce the need for setuid binaries.\n");
}

int main(void) {
    printf("╔═══════════════════════════════════════╗\n");
    printf("║     OS SECURITY: ACCESS CONTROL        ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    printf("Running as UID=%d (root=%s)\n\n", getuid(), getuid()==0?"YES":"no");

    demonstrate_file_creation();
    explain_setuid();

    printf("\n[Permission Concepts]\n");
    printf("Unix DAC: Discretionary Access Control\n");
    printf("  Each file has owner, group, world permissions (rwx each)\n");
    printf("  Owner DECIDES who can access (gives or restricts)\n\n");
    printf("Linux capabilities: fine-grained root powers\n");
    printf("  CAP_NET_BIND_SERVICE: bind port < 1024\n");
    printf("  CAP_KILL: send signals to any process\n");
    printf("  CAP_SYS_ADMIN: lots of admin ops\n");
    printf("  Better than full setuid-root!\n");
    return 0;
}
