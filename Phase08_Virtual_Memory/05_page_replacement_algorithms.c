/* Phase08/05_page_replacement_algorithms.c
 * FIFO, LRU, Optimal (OPT) page replacement with comparison
 * Compile: gcc -Wall -o page_replace 05_page_replacement_algorithms.c
 */
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define MAX_FRAMES 10
#define MAX_REFS   50

int frames[MAX_FRAMES], n_frames;

int find_page(int page) {
    for(int i=0;i<n_frames;i++) if(frames[i]==page) return i;
    return -1;
}
int empty_frame(void) {
    for(int i=0;i<n_frames;i++) if(frames[i]==-1) return i;
    return -1;
}

/* FIFO: evict oldest (circular queue) */
int fifo(int *refs, int n) {
    memset(frames,-1,sizeof(frames)); int faults=0,ptr=0;
    for(int t=0;t<n;t++) {
        if(find_page(refs[t])==-1) {
            faults++;
            int fi=empty_frame();
            if(fi==-1) fi=ptr;
            frames[fi]=refs[t]; ptr=(ptr+1)%n_frames;
        }
    }
    return faults;
}

/* LRU: evict least recently used */
int lru(int *refs, int n) {
    memset(frames,-1,sizeof(frames));
    int last[MAX_FRAMES]; memset(last,-1,sizeof(last));
    int faults=0;
    for(int t=0;t<n;t++) {
        int fi=find_page(refs[t]);
        if(fi==-1) {
            faults++;
            fi=empty_frame();
            if(fi==-1) {
                int min=last[0],idx=0;
                for(int i=1;i<n_frames;i++) if(last[i]<min){min=last[i];idx=i;}
                fi=idx;
            }
            frames[fi]=refs[t];
        }
        last[fi]=t;
    }
    return faults;
}

/* OPT: evict page used furthest in future */
int opt(int *refs, int n, int t_start) {
    memset(frames,-1,sizeof(frames)); int faults=0;
    for(int t=t_start;t<n;t++) {
        int fi=find_page(refs[t]);
        if(fi==-1) {
            faults++;
            fi=empty_frame();
            if(fi==-1) {
                /* Find page whose next use is furthest away */
                int far=0,far_idx=0;
                for(int i=0;i<n_frames;i++) {
                    int next=n; /* Assume never used again */
                    for(int k=t+1;k<n;k++) if(refs[k]==frames[i]){next=k;break;}
                    if(next>far){far=next;far_idx=i;}
                }
                fi=far_idx;
            }
            frames[fi]=refs[t];
        }
    }
    return faults;
}

int main(void) {
    printf("=== Page Replacement Algorithms ===\n");
    int refs[] = {7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1};
    int n = sizeof(refs)/sizeof(refs[0]);

    printf("Reference string: ");
    for(int i=0;i<n;i++) printf("%d ",refs[i]); printf("\n\n");

    printf("%-8s | FIFO | LRU  | OPT\n","Frames");
    printf("--------+------+------+-----\n");
    for(int f=1;f<=5;f++) {
        n_frames=f;
        int ff=fifo(refs,n);
        int lf=lru(refs,n);
        int of=opt(refs,n,0);
        printf("   %d    |  %2d  |  %2d  |  %2d\n",f,ff,lf,of);
    }

    printf("\n[Key Observations]\n");
    printf("OPT always has fewest faults — theoretical minimum (not implementable)\n");
    printf("LRU approximates OPT well in practice\n");
    printf("FIFO suffers Belady's Anomaly (more frames can mean MORE faults!)\n");
    printf("LRU is a STACK algorithm — no Belady's Anomaly\n");
    return 0;
}
