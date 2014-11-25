/*
 * Implicit list
 * First fit. 
 * Each block has footer and header for 4 bytes.
 * Use padding to eliminate bond conditions of coalscing. 
 *
 * Structure of block:
 * 
 *  --------------------------
 * |  HEADER  | DATA|  FOOTER|
 *  --------------------------
 *
 *
 * Structure of header and footer(alignment: 8) :
 *
 *   31                     3  2  1  0
 *  ------------------------------------
 * | s  s  s  s  ... s  s  s  0  0  a/f|
 *  ------------------------------------
 *
 *
 * Structure of heap:
 *
 *  -----------------------------------------------------------------
 * |  prelogue  | block1| block2| block3 | block4 |......| epilogue |
 *  -----------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "fl",
    /* First member's full name */
    "Juan Li",
    /* First member's email address */
    "juanli2014@u.northwestern.edu",
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

#define WSIZE       4           /* word size (bytes) */
#define DSIZE       8
#define PROLOGUE_SIZE 8         /* size of header and footer*/
#define MIN_BLOCKSIZE 16        /* min block size (bytes) */
#define MIN_HEAPSIZE (1<<8)    /* initial heap size */

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HEADER(bp)      ((char *)(bp) - WSIZE)
#define FOOTER(bp)      ((char *)(bp) + GET_SIZE(HEADER(bp)) - DSIZE)

/* Given block ptr bp, compute address of its next block's header and previous's block footer */
#define NEXT_HEADER(bp) (FOOTER(bp) + WSIZE)
#define PREV_FOOTER(bp) (HEADER(bp) - WSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* max of x and y*/
#define MAX(x, y) ((x) > (y)? (x) : (y))

/*a pointer to the first block */
static char *mm_heap;

/*functions, internal helper*/
static void *find_firstfit(size_t size);      /* Find first fit block */
static void addBlock(void *bp, size_t size);   /* Add block, if necessary, split block */
static void *coalesce(void *bp);               /*coalesce free blocks*/
static void *extendHeap(size_t size);          /* extend heap*/

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //printf("Initialize....");
    
    mm_heap = mem_sbrk(WSIZE << 2);
    
    /* check if mem_sbrk succeed */
    if((int)mm_heap == -1) return -1;
    
    /* empty heap init */
    PUT(mm_heap, 0);
    PUT(mm_heap+WSIZE, PACK(PROLOGUE_SIZE, 1));          /*header */
    PUT(mm_heap+PROLOGUE_SIZE, PACK(PROLOGUE_SIZE, 1));  /*footer*/
    PUT(mm_heap+WSIZE+PROLOGUE_SIZE, PACK(0, 1));        /*epilogue*/
    mm_heap += PROLOGUE_SIZE;
    
    /*extend heap with min heap size */
    if (extendHeap(MIN_HEAPSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    //printf("Malloc...");
    
    char *bp;
    size_t extend;
    
    /* If size <= 0, ignore */
    if(size <= 0) return NULL;
    
    /* Add header and footer*/
    size_t newsize = ALIGN(size + SIZE_T_SIZE);
    // size_t newsize;
    // if (size <= DSIZE)
    //     newsize = DSIZE + PROLOGUE_SIZE;
    // else
    //     newsize = DSIZE * ((size + (PROLOGUE_SIZE) + (DSIZE-1)) / DSIZE);
    
    /*first fit search*/
    bp = find_firstfit(newsize);
    if(bp != NULL){
        addBlock(bp, newsize);
        return bp;
    }
    
    /* If no fit, extend heap to get more memory*/
    extend = MAX(newsize, MIN_HEAPSIZE);
    bp = extendHeap(extend/WSIZE);

    /* check if extend successfully*/
    //printf("bp after extends, before addblock:%X\n",bp);
    if(bp == NULL) return NULL;
    
    /* add block */
    addBlock(bp, newsize);
    
    return bp;
   }

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

    /* get block size*/
    size_t size = GET_SIZE(HEADER(ptr));
    
    /* change to unallocated*/
    PUT(HEADER(ptr), PACK(size, 0));
    PUT(FOOTER(ptr), PACK(size, 0));

    //printf("free.........\n          HEADER(ptr)%X \n        SIze:%d\n",HEADER(ptr),size );
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    //printf("reallocate..............");
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    /* if size = 0, free ptr */
    if(size == 0){
        mm_free(ptr);
        return NULL;
    }

    /* if ptr doesn't exist, malloc */
    if(ptr == NULL) return mm_malloc(size);

    /* if ptr exist */
    newptr = mm_malloc(size);

    /* if fail to reallocate, return null */
    if (newptr == NULL)
      return NULL;

   // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    copySize = GET_SIZE(HEADER(ptr));
    
    /* compare size */
    if (size < copySize) copySize = size;
    
    /* copy */
    memcpy(newptr, oldptr, copySize);

    /* free old block */
    mm_free(oldptr);

    return newptr;
}

/*
 * find_firstfit - Find a fit for a block with size bytes
 */
static void *find_firstfit(size_t size)
{
    //printf("Find first fit....");
    void *bp;
    
    /* first fit search */
    //for (bp = mm_heap; GET_SIZE(HEADER(bp)) > 0; bp = NEXT_BP(bp)){

    for (bp = mm_heap; GET_SIZE(HEADER(bp)) > 0; bp = NEXT_BP(bp)){



        if (!GET_ALLOC(HEADER(bp)) && (size <= GET_SIZE(HEADER(bp)))) {
            //printf("fit block:%X",bp);
            return bp;
        }
    }
    
    /* No fit */
    return NULL;
}


/*
 * addBlock - add block of size bytes at start of free block bp
 * if the remain greater than minimal block size, split.
 */
static void addBlock(void *bp, size_t size)
{
    //printf("Add block....");
    size_t oldsize = GET_SIZE(HEADER(bp));
    
    if ((oldsize - size) >= MIN_BLOCKSIZE) {
        PUT(HEADER(bp), PACK(size, 1));
        PUT(FOOTER(bp), PACK(size, 1));
        //printf("bp %X\n header %X\n footer %X\n",bp,HEADER(bp),FOOTER(bp));

        //printf("before next bp\n");
        bp = NEXT_BP(bp);
        //printf("after next bp\n");

        PUT(HEADER(bp), PACK(oldsize-size, 0));

        //printf("header:%X\n, size:%d\n",HEADER(bp),GET_SIZE(HEADER(bp)));
        

        PUT(FOOTER(bp), PACK(oldsize-size, 0));

        //printf("new bp %X\n header %X\n footer %X\n",bp,HEADER(bp),FOOTER(bp));

    }
    else {
        PUT(HEADER(bp), PACK(oldsize, 1));
        PUT(FOOTER(bp), PACK(oldsize, 1));
        //printf("bp %X\n header %X\n footer %X\n     if is free%d\n",bp,HEADER(bp),FOOTER(bp),GET_ALLOC(HEADER(bp)));
        //printf("      pack(size,1): %X\n       get(p):%X\n",PACK(size,1),GET(HEADER(bp)));

    }
}

/*
 * extendHeap - if heap is full, extend heap with free block and return its block pointer
 */
static void *extendHeap(size_t size)
{
    //printf("Extend heap....");
    char *bp;
    
    /* round to even */
    //size = ((size + 1) >> 1) << 1;
    size = (size % 2) ? (size+1) * WSIZE : size * WSIZE;    
    /* check if mem_sbrk succeed */
    if ((int)(bp = mem_sbrk(size)) == -1)
        return NULL;

    //printf("bp : %X\n",bp);

    /* Initialize free block header/footer and the epilogue header */
    PUT(HEADER(bp), PACK(size, 0));         /*header */
    PUT(FOOTER(bp), PACK(size, 0));         /*footer */
    PUT(HEADER(NEXT_BP(bp)), PACK(0, 1));   /*epilogue*/
    
    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * coalesce - coalesced block
 */
static void *coalesce(void *bp)
{
    //printf("coalesce.....bp:%X\n",bp);
    size_t prevBlock = GET_ALLOC(FOOTER(PREV_BP(bp)));
    size_t nextBlock = GET_ALLOC(HEADER(NEXT_BP(bp)));

    // size_t prevBlock = GET_ALLOC(HEADER(PREV_BP(bp))) || (PREV_BP(bp) > mm_heap);
    //size_t nextBlock = GET_ALLOC(HEADER(NEXT_BP(bp))) || (NEXT_BP(bp) <= end);


    size_t size = GET_SIZE(HEADER(bp));
    //printf("prev block:%X",prevBlock);
    //printf("next block:%X",nextBlock);
    
    /* Case 1 */
    if (prevBlock && nextBlock) {
        //printf("all used");
        return bp;
    }
    /* Case 2 */
    else if (prevBlock && !nextBlock) {
        size += GET_SIZE(HEADER(NEXT_BP(bp)));
        PUT(HEADER(bp), PACK(size, 0));
        PUT(FOOTER(bp), PACK(size, 0));
        //printf("next free");
        return(bp);
    }
    /* Case 3 */
    else if (!prevBlock && nextBlock) {
        size += GET_SIZE(HEADER(PREV_BP(bp)));
        PUT(HEADER(PREV_BP(bp)), PACK(size, 0));
        PUT(FOOTER(bp), PACK(size, 0));
        //printf("prev free");
        return(PREV_BP(bp));
    }
    /* Case 4 */
    else {
        size += GET_SIZE(HEADER(PREV_BP(bp))) +
        GET_SIZE(FOOTER(NEXT_BP(bp)));
        PUT(HEADER(PREV_BP(bp)), PACK(size, 0));
        PUT(FOOTER(NEXT_BP(bp)), PACK(size, 0));
        //printf("both free");
        return(PREV_BP(bp));
    }
}














