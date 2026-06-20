/* Phase07/10_malloc_implementation.c
 * Simple malloc/free using first-fit free list with coalescing
 * Compile: gcc -Wall -o mymalloc 10_malloc_implementation.c
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct Block {
    size_t       size;
    int          free;
    struct Block *next;
} Block;

#define HDR  sizeof(Block)
#define ALIGN8(s) (((s)+7)&~7)
#define MIN_SPLIT 32

static Block *head = NULL;

static Block *extend(size_t sz) {
    Block *b = sbrk(0);
    if(sbrk(HDR+sz) == (void*)-1) return NULL;
    b->size=sz; b->free=0; b->next=NULL;
    if(!head) head=b;
    else { Block *c=head; while(c->next) c=c->next; c->next=b; }
    return b;
}

static Block *find_free(size_t sz) {
    Block *b=head;
    while(b){ if(b->free && b->size>=sz) return b; b=b->next; }
    return NULL;
}

static void split(Block *b, size_t sz) {
    if(b->size >= sz+HDR+MIN_SPLIT) {
        Block *nb=(Block*)((char*)b+HDR+sz);
        nb->size=b->size-sz-HDR; nb->free=1; nb->next=b->next;
        b->size=sz; b->next=nb;
    }
}

void *my_malloc(size_t sz) {
    if(!sz) return NULL;
    sz=ALIGN8(sz);
    Block *b=find_free(sz);
    if(b) { split(b,sz); b->free=0; }
    else  { b=extend(sz); if(!b) return NULL; }
    return (void*)(b+1);
}

static void coalesce(void) {
    Block *c=head;
    while(c && c->next) {
        if(c->free && c->next->free){
            c->size+=HDR+c->next->size; c->next=c->next->next;
        } else c=c->next;
    }
}

void my_free(void *p) {
    if(!p) return;
    Block *b=(Block*)p-1;
    b->free=1; coalesce();
}

void dump_heap(const char *label) {
    printf("Heap [%s]:\n", label);
    Block *b=head; int i=0;
    while(b){ printf("  [%d] size=%zu %s\n",i++,b->size,b->free?"FREE":"USED"); b=b->next; }
}

int main(void) {
    printf("=== Custom malloc/free Demo ===\n\n");

    int   *arr = my_malloc(sizeof(int)*10);
    char  *str = my_malloc(50);
    double*dbl = my_malloc(sizeof(double)*5);

    for(int i=0;i<10;i++) arr[i]=i*i;
    strcpy(str,"Hello from my_malloc!");
    for(int i=0;i<5;i++) dbl[i]=i*3.14;

    printf("arr[5]=%d  str='%s'  dbl[2]=%.2f\n", arr[5], str, dbl[2]);
    dump_heap("after 3 allocs");

    my_free(arr); my_free(str);
    dump_heap("after freeing arr+str");

    int *r = my_malloc(sizeof(int)*3);
    printf("Re-alloc ptr: %p (should reuse freed memory)\n",(void*)r);
    my_free(r); my_free(dbl);

    printf("\n[Key concepts]\n");
    printf("1. Header before each payload stores size+free flag\n");
    printf("2. First-fit scan: find first free block >= requested size\n");
    printf("3. Split: if block is much larger, split into 2 blocks\n");
    printf("4. Coalesce: merge adjacent free blocks on free()\n");
    return 0;
}
