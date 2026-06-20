/* Phase10/04_disk_scheduling.c
 * FCFS, SSTF, SCAN, LOOK, C-LOOK disk scheduling algorithms with comparison
 * Compile: gcc -Wall -o disk_sched 04_disk_scheduling.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100

int req[MAX], n_req, head;

int abs_val(int x){ return x<0?-x:x; }
int cmp(const void *a,const void *b){ return *(int*)a-*(int*)b; }

int find_nearest(int *served, int cur) {
    int best=-1, bst=999999;
    for(int i=0;i<n_req;i++) {
        if(served[i]) continue;
        int d=abs_val(req[i]-cur);
        if(d<bst){bst=d;best=i;}
    }
    return best;
}

int fcfs(void) {
    int total=0,cur=head; printf("FCFS: %d",head);
    for(int i=0;i<n_req;i++){total+=abs_val(req[i]-cur);cur=req[i];printf("→%d",cur);}
    printf(" (total=%d)\n",total); return total;
}

int sstf(void) {
    int served[MAX]={0},total=0,cur=head; printf("SSTF: %d",head);
    for(int k=0;k<n_req;k++){
        int i=find_nearest(served,cur);
        total+=abs_val(req[i]-cur);cur=req[i];served[i]=1;printf("→%d",cur);
    }
    printf(" (total=%d)\n",total); return total;
}

int look(void) {
    int s[MAX]; memcpy(s,req,n_req*sizeof(int)); qsort(s,n_req,sizeof(int),cmp);
    int total=0,cur=head,split=0; printf("LOOK: %d",head);
    while(split<n_req && s[split]<head) split++;
    for(int i=split;i<n_req;i++){total+=abs_val(s[i]-cur);cur=s[i];printf("→%d",cur);}
    for(int i=split-1;i>=0;i--){total+=abs_val(s[i]-cur);cur=s[i];printf("→%d",cur);}
    printf(" (total=%d)\n",total); return total;
}

int clook(void) {
    int s[MAX]; memcpy(s,req,n_req*sizeof(int)); qsort(s,n_req,sizeof(int),cmp);
    int total=0,cur=head,split=0; printf("C-LOOK: %d",head);
    while(split<n_req && s[split]<head) split++;
    for(int i=split;i<n_req;i++){total+=abs_val(s[i]-cur);cur=s[i];printf("→%d",cur);}
    if(split>0){
        total+=abs_val(cur-s[0]);cur=s[0];printf("→[jump %d]",cur);
        for(int i=1;i<split;i++){total+=abs_val(s[i]-cur);cur=s[i];printf("→%d",cur);}
    }
    printf(" (total=%d)\n",total); return total;
}

int main(void) {
    printf("=== Disk Scheduling Algorithms ===\n\n");
    int test[]={98,183,37,122,14,124,65,67};
    n_req=sizeof(test)/sizeof(test[0]); head=53;
    memcpy(req,test,n_req*sizeof(int));

    printf("Head: %d  Queue: ",head);
    for(int i=0;i<n_req;i++) printf("%d ",req[i]); printf("\n\n");

    int f=fcfs(), s=sstf(), l=look(), c=clook();
    printf("\n=== Summary ===\n");
    printf("FCFS  : %d  (simple, poor)\n",f);
    printf("SSTF  : %d  (good avg, starvation risk)\n",s);
    printf("LOOK  : %d  (good balance)\n",l);
    printf("C-LOOK: %d  (uniform wait time)\n",c);
    return 0;
}
