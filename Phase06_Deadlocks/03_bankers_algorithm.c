/* Phase06/03_bankers_algorithm.c
 * Complete Banker's Algorithm — Deadlock Avoidance
 * Compile: gcc -Wall -o banker 03_bankers_algorithm.c
 */
#include <stdio.h>
#include <string.h>

#define MAXP 10
#define MAXR 10

int N=5, M=3;
int Available[MAXR], Max[MAXP][MAXR], Alloc[MAXP][MAXR], Need[MAXP][MAXR];
int Work[MAXR], Finish[MAXP], SafeSeq[MAXP];

void compute_need(void) {
    for (int i=0;i<N;i++)
        for (int j=0;j<M;j++)
            Need[i][j] = Max[i][j] - Alloc[i][j];
}

int is_safe(void) {
    memcpy(Work, Available, M*sizeof(int));
    memset(Finish, 0, N*sizeof(int));
    int idx=0, progress=1;
    while(progress) {
        progress=0;
        for(int i=0;i<N;i++) {
            if(Finish[i]) continue;
            int ok=1;
            for(int j=0;j<M;j++) if(Need[i][j]>Work[j]){ok=0;break;}
            if(ok) {
                for(int j=0;j<M;j++) Work[j]+=Alloc[i][j];
                Finish[i]=1; SafeSeq[idx++]=i; progress=1;
            }
        }
    }
    for(int i=0;i<N;i++) if(!Finish[i]) return 0;
    return 1;
}

int request(int proc, int req[]) {
    for(int j=0;j<M;j++) if(req[j]>Need[proc][j]) return -1; /* exceeds max */
    for(int j=0;j<M;j++) if(req[j]>Available[j]) return 0;   /* must wait */
    /* Pretend allocate */
    for(int j=0;j<M;j++){Available[j]-=req[j];Alloc[proc][j]+=req[j];Need[proc][j]-=req[j];}
    if(is_safe()) return 1;
    /* Rollback */
    for(int j=0;j<M;j++){Available[j]+=req[j];Alloc[proc][j]-=req[j];Need[proc][j]+=req[j];}
    return 2; /* unsafe */
}

void print_state(void) {
    printf("\n%-4s | Alloc       | Max         | Need\n","Proc");
    printf("------+-------------+-------------+-------------\n");
    for(int i=0;i<N;i++) {
        printf("P%-3d | ",i);
        for(int j=0;j<M;j++) printf("%d ",Alloc[i][j]);
        printf("| ");
        for(int j=0;j<M;j++) printf("%d ",Max[i][j]);
        printf("| ");
        for(int j=0;j<M;j++) printf("%d ",Need[i][j]);
        printf("\n");
    }
    printf("Available: ");
    for(int j=0;j<M;j++) printf("%d ",Available[j]);
    printf("\n");
}

int main(void) {
    printf("=== Banker's Algorithm (Deadlock Avoidance) ===\n");

    /* Silberschatz textbook example */
    int av[]={{3,3,2}};  memcpy(Available,av,M*sizeof(int));
    int al[][3]={{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
    int mx[][3]={{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
    for(int i=0;i<N;i++){
        memcpy(Alloc[i],al[i],M*sizeof(int));
        memcpy(Max[i],mx[i],M*sizeof(int));
    }
    compute_need();
    print_state();

    printf("\n--- Safety Check ---\n");
    if(is_safe()) {
        printf("SAFE STATE. Safe sequence: ");
        for(int i=0;i<N;i++) printf("P%d ",SafeSeq[i]);
        printf("\n");
    } else printf("UNSAFE STATE\n");

    printf("\n--- P1 requests [1,0,2] ---\n");
    int req[]={1,0,2};
    int r = request(1,req);
    if(r==1) {
        printf("GRANTED. New safe sequence: ");
        for(int i=0;i<N;i++) printf("P%d ",SafeSeq[i]);
        printf("\n");
    } else if(r==0) printf("P1 must WAIT (resources unavailable)\n");
    else if(r==2)   printf("DENIED (would cause unsafe state)\n");
    else             printf("ERROR: request exceeds Max\n");

    return 0;
}
