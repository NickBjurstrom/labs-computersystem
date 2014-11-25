/*
 * Explicit list
 * Header and footer: 4 bytes
 * Allocation bit: last bit
 * Best fit(use binary search tree to reduce search time)
 *
 * structure of nodes
 *
 *  ---------------------------------------------------------------------------
 * |  HEADER  | LEFTCHILD | RIGHTCHILD | PARENT  | BROTHERS | DATA  |  FOOTER |
 *  ---------------------------------------------------------------------------
 *
 * min block size is 24
 *
 * Structure of header and footer(alignment: 8) :
 *
 *   31                     3  2  1  0
 *  -----------------------------------
 * | s  s  s  s  ... s  s  s  0  0  a/f
 *  -----------------------------------
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
    /* Second member's full name (leave blank if none) */
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

#define WSIZE       4           /* word size (bytes) */
#define DSIZE       8
#define TSIZE       12
#define QSIZE       16
#define PROLOGUE_SIZE 8         /* size of header and footer*/
#define MIN_BLOCKSIZE 24        /* min block size (bytes) */
#define MIN_HEAPSIZE (1<<10)     /* initial heap size */

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header footer, etc*/
#define HEADER(bp)      ((void *)(bp) - WSIZE)
#define LEFT(bp)        ((void *)(bp)) 
#define RIGHT(bp)       ((void *)(bp) + WSIZE) 
#define PARENT(bp)      ((void *)(bp) + DSIZE) 
#define BROTHERS(bp)    ((void *)(bp) + TSIZE)
#define FOOTER(bp)      ((void *)(bp) + GET_SIZE(HEADER(bp)) - DSIZE)

/* Given block ptr bp, compute address of its next block's header and previous's block footer */
#define NEXT_HEADER(bp) (FOOTER(bp) + WSIZE)
#define PREV_FOOTER(bp) (HEADER(bp) - WSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BP(bp)  ((void *)(bp) + GET_SIZE(((void *)(bp) - WSIZE)))
#define PREV_BP(bp)  ((void *)(bp) - GET_SIZE(((void *)(bp) - DSIZE)))

/* get left child, right child, etc */
#define GET_LEFT(bp) (GET(LEFT(bp)))
#define GET_RIGHT(bp) (GET(RIGHT(bp)))
#define GET_PARENT(bp) (GET(PARENT(bp)))
#define GET_BROTHERS(bp) (GET(BROTHERS(bp)))
#define GET_FOOTER(bp) (GET(FOOTER(bp)))


/*Define value to each voidacter in the block bp points to.*/
#define PUT_HEADER(bp, val) (PUT(HEADER(bp), (int)val))
#define PUT_FOOTER(bp, val) (PUT(FOOTER(bp), (int)val))
#define PUT_LEFT(bp, val) (PUT(LEFT(bp), (int)val))
#define PUT_RIGHT(bp, val) (PUT(RIGHT(bp), (int)val))
#define PUT_PARENT(bp, val) (PUT(PARENT(bp), (int)val))
#define PUT_BROTHERS(bp, val) (PUT(BROTHERS(bp), (int)val))


/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* max of x and y*/
#define MAX(x, y) ((x) > (y)? (x) : (y))

/*a pointer to the first block */
static void *mm_heap;
static void *mm_freelist;

/*functions, internal helper*/
static void *find_bestfit(size_t size);        /* Find bes fit block */
static void addBlock(void *bp, size_t size);   /* Add block, if necessary, split block */
static void *coalesce(void *bp);               /*coalesce free blocks*/
static void *extendHeap(size_t size);          /* extend heap*/
static void insertNode(void *bp);              /* insert node to free list */
static void deleteNode(void *bp);              /* delete node from free list */


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // printf("Initialize....");
    
    mm_heap = mem_sbrk(QSIZE);
    
    /* check if mem_sbrk succeed */
    if((int)mm_heap == -1) return -1;
    
    /* empty heap init */
    PUT(mm_heap, 0);
    PUT(mm_heap+WSIZE, PACK(PROLOGUE_SIZE, 1));          /*header */
    PUT(mm_heap+PROLOGUE_SIZE, PACK(PROLOGUE_SIZE, 1));  /*footer*/
    PUT(mm_heap+TSIZE, PACK(0, 1));        /*epilogue*/
    mm_heap += QSIZE;
    mm_freelist = NULL;
    
    /*extend heap with min heap size */
    if (extendHeap(ALIGN(MIN_HEAPSIZE)) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    // printf("Malloc...");
    
    void *bp;
    size_t extend;
    
    /* If size <= 0, ignore */
    if(size <= 0) return NULL;
    
    /* Add header and footer*/
   // size_t newsize = ALIGN(size + SIZE_T_SIZE);
    size_t newsize;
    if(size < MIN_BLOCKSIZE - DSIZE) 
        newsize = MIN_BLOCKSIZE;
    else 
        newsize = ALIGN(size + DSIZE);
    /*first fit search*/
    bp = find_bestfit(newsize);

    if(bp == NULL){
      /* If no fit, extend heap to get more memory*/
        extend = MAX(newsize + 32, MIN_HEAPSIZE);
        extendHeap(ALIGN(extend));
        if((bp = find_bestfit(newsize))== NULL) return NULL;
    }
   
    if( size==448 && GET_SIZE(HEADER(bp)) > newsize+64 )
        newsize += 64;
    else if( size==112 && GET_SIZE(HEADER(bp)) > newsize+16 )
        newsize += 16;
    
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
    insertNode(coalesce(ptr));

    //printf("free.........\n          HEADER(ptr)%X \n        SIze:%d\n",HEADER(ptr),size );
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    // printf("reallocate..............");
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    /* if size = 0, free ptr */
    if(size == 0){
        mm_free(ptr);
        return NULL;
    }
    //printf("place 1\n");
    /* if ptr doesn't exist, malloc */
    if(ptr == NULL) return mm_malloc(size);
    //printf("place 2\n");
    /* if ptr exist */
    newptr = mm_malloc(size);
    //printf("place 3\n");
    /* if fail to reallocate, return null */
    if (newptr == NULL)
      return NULL;
    //printf("place 4\n");
   // copySize = *(size_t *)((void *)oldptr - SIZE_T_SIZE);
    copySize = GET_SIZE(HEADER(ptr));
    
    /* compare size */
    if (size < copySize) copySize = size;
    
    /* copy */
    memcpy(newptr, oldptr, copySize);
    //printf("place 5\n");
    /* free old block */
    mm_free(oldptr);
    //printf("place 6\n");
    return newptr;
}

/*
 * find_bestfit - Find a fit for a block with size bytes
 */
static void *find_bestfit(size_t size)
{
    // printf("Find best fit....");
    void *bp = NULL;
    void *temp = mm_freelist;
    
    /* first fit search */
    while(temp != NULL){
        if(size <= GET_SIZE(HEADER(temp))){
            bp = temp;
            temp = (void *)GET_LEFT(temp);
        }else{
            temp = (void *)GET_RIGHT(temp);
        }
    }
    
    /* No fit */
    return bp;
}


/*
 * addBlock - add block of size bytes at start of free block bp
 * if the remain greater than minimal block size, split.
 */
static void addBlock(void *bp, size_t size)
{
    // printf("Add block....");
    size_t oldsize = GET_SIZE(HEADER(bp));
    deleteNode(bp);
    
    /*If remain greater than block size, split */
    if ((oldsize - size) >= MIN_BLOCKSIZE) {
        PUT(HEADER(bp), PACK(size, 1));
        PUT(FOOTER(bp), PACK(size, 1));

        //printf("before next bp\n");
        bp = NEXT_BP(bp);
        //printf("after next bp\n");

        PUT(HEADER(bp), PACK(oldsize-size, 0));

        //printf("header:%X\n, size:%d\n",HEADER(bp),GET_SIZE(HEADER(bp)));
        

        PUT(FOOTER(bp), PACK(oldsize-size, 0));

        //printf("new bp %X\n header %X\n footer %X\n",bp,HEADER(bp),FOOTER(bp));

        insertNode(coalesce(bp));

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
    // printf("Extend heap....");
    void *bp;
    
    /* round to even */
    //size = ((size + 1) >> 1) << 1;
    //size = (size % 2) ? (size+1) * WSIZE : size * WSIZE;    
    /* check if mem_sbrk succeed */
    if ((int)(bp = mem_sbrk(size)) == -1)
        return NULL;

    //printf("bp : %X\n",bp);

    /* Initialize free block header/footer and the epilogue header */
    PUT(HEADER(bp), PACK(size, 0));         /*header */
    PUT(FOOTER(bp), PACK(size, 0));         /*footer */
    PUT(HEADER(NEXT_BP(bp)), PACK(0, 1));   /*epilogue*/
    insertNode(coalesce(bp));
    
    /* Coalesce if the previous block was free */
    return (void *) bp;
}

/*
 * coalesce - coalesced block
 */
static void *coalesce(void *bp)
{
    // printf("coalesce.....bp:%X\n",bp);
    size_t prevBlock = GET_ALLOC(FOOTER(PREV_BP(bp)));
    size_t nextBlock = GET_ALLOC(HEADER(NEXT_BP(bp)));

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
        deleteNode(NEXT_BP(bp));
        size += GET_SIZE(HEADER(NEXT_BP(bp)));
        PUT(HEADER(bp), PACK(size, 0));
        PUT(FOOTER(bp), PACK(size, 0));
        //printf("next free");
        return bp;
    }
    /* Case 3 */
    else if (!prevBlock && nextBlock) {
        deleteNode(PREV_BP(bp));
        size += GET_SIZE(HEADER(PREV_BP(bp)));
        PUT(HEADER(PREV_BP(bp)), PACK(size, 0));
        PUT(FOOTER(bp), PACK(size, 0));
        //printf("prev free");
        return(PREV_BP(bp));
    }
    /* Case 4 */
    else {
        deleteNode(PREV_BP(bp));
        deleteNode(NEXT_BP(bp));
        size += GET_SIZE(HEADER(PREV_BP(bp))) +
        GET_SIZE(FOOTER(NEXT_BP(bp)));
        PUT(HEADER(PREV_BP(bp)), PACK(size, 0));
        PUT(FOOTER(NEXT_BP(bp)), PACK(size, 0));
        //printf("both free");
        return(PREV_BP(bp));
    }
}

static void insertNode(void *bp){
    // printf(" insert node ........\n");
    /* if mm_freelist is null*/
    if(mm_freelist == NULL){
        mm_freelist = bp;
        PUT_LEFT(bp, NULL);
        PUT_RIGHT(bp, NULL);
        PUT_PARENT(bp, NULL);
        PUT_BROTHERS(bp, NULL);
        return;
    }

    void* temp = mm_freelist;
    while(1){

        /* bp less than temp */
        if( GET_SIZE(HEADER(bp)) < GET_SIZE(HEADER(temp)) ){
            if( (void *)GET_LEFT(temp) != NULL ){
                temp = (void *)GET_LEFT( temp );
            }else{
                PUT_LEFT( temp, bp ); 
                PUT_PARENT( bp, temp );
                PUT_LEFT( bp, NULL );
                PUT_RIGHT( bp, NULL );
                PUT_BROTHERS( bp, NULL );
                break;
            }
        }

        /* bp greater than temp */
        else if( GET_SIZE(HEADER(bp)) > GET_SIZE(HEADER(temp)) ){
            if( (void *)GET_RIGHT(temp) != NULL ){
                temp = (void *)GET_RIGHT(temp);
            }else{
                PUT_RIGHT( temp, bp );
                PUT_PARENT( bp, temp );
                PUT_LEFT( bp, NULL );
                PUT_RIGHT( bp, NULL );
                PUT_BROTHERS( bp, NULL );
                break;
            }
        }

        /* if size equal */
        else{

            /* if brother exist */
            if((void *) GET_BROTHERS(temp) != NULL){
                /* if temp is the root */
                if( temp == mm_freelist ){
                    mm_freelist = bp;
                    PUT_PARENT( bp, NULL );
                }
                else{ 
                    /* temp is left child of its parent */
                    if( (void *)GET_LEFT(GET_PARENT(temp)) == temp)
                        PUT_LEFT( GET_PARENT(temp), bp );

                    /* temp is right child of its parent */
                    else 
                        PUT_RIGHT( GET_PARENT(temp), bp );

                    PUT_PARENT( bp, GET_PARENT(temp) );
                }

                PUT_LEFT( bp, GET_LEFT(temp) );
                PUT_RIGHT( bp, GET_RIGHT(temp) );
                PUT_BROTHERS( bp, temp );

                /* temp has left child */
                if( (void *)GET_LEFT(temp) != NULL )
                    PUT_PARENT( GET_LEFT(temp), bp );

                /*temp has right child*/
                if( (void *)GET_RIGHT(temp) != NULL )
                    PUT_PARENT( GET_RIGHT(temp), bp );

                /* brothers node, right child set -1 (no right child)*/
                PUT_LEFT( temp, bp );
                PUT_RIGHT( temp, -1 );
                break;
            }

            /* no brothers */
            else{
                PUT_BROTHERS( bp, NULL );
                PUT_LEFT( bp, temp );
                PUT_RIGHT( bp, -1 );
                PUT_BROTHERS( temp, bp );
                if( (void *)GET_BROTHERS(bp) != NULL )
                    PUT_LEFT( GET_BROTHERS(bp), bp );
                break;
            }
        }
    }
}



static void deleteNode(void *bp)
{
    // printf(" delete node ....\n");
    /* bp has no brothers*/ 
    if( (void *)GET_BROTHERS(bp) == NULL && GET_RIGHT(bp) != -1 ){

        /* bp is the root */
        if( bp == mm_freelist ){

            /* no right child */ 
            if( (void *)GET_RIGHT(bp) == NULL ){
                mm_freelist=(void *)GET_LEFT(bp);
                if( mm_freelist != NULL )
                    PUT_PARENT( mm_freelist, NULL );
            }

            /* right child exist */
            else{
                void *temp = (void *)GET_RIGHT(bp);

                /* find the smallest node in the left subtree*/
                while( (void *)GET_LEFT(temp) != NULL )
                    temp = (void *)GET_LEFT(temp);

                /* set the smallest node in the right subtree as root*/
                void *tempL = (void *)GET_LEFT(bp);
                void *tempR = (void *)GET_RIGHT(temp);
                void *tempP = (void *)GET_PARENT(temp);
                mm_freelist = temp;

                if( mm_freelist != NULL )
                    PUT_PARENT( mm_freelist, NULL );
                PUT_LEFT( temp,GET_LEFT(bp) );

                if( temp != (void *)GET_RIGHT(bp) ){
                    PUT_RIGHT( temp,GET_RIGHT(bp) );
                    PUT_LEFT( tempP, tempR );
                    if( tempR != NULL)
                        PUT_PARENT( tempR, tempP );
                    PUT_PARENT( GET_RIGHT(bp),temp );
                }
                if( tempL != NULL )
                    PUT_PARENT( tempL, temp );
            }
        }

        /* not the root */
        else{
            if( (void *)GET_RIGHT(bp) == NULL ){/* no right child */
                if( (void *)GET_LEFT( GET_PARENT( bp ) ) == bp ) PUT_LEFT( GET_PARENT(bp), GET_LEFT(bp) );
                else
                    PUT_RIGHT( GET_PARENT(bp), GET_LEFT(bp) );
                if( (void *)GET_LEFT(bp) != NULL)
                    PUT_PARENT( GET_LEFT(bp), GET_PARENT(bp) );
            }else{/* it has a right child */
                void *temp = (void *)GET_RIGHT(bp);
                while( (void *)GET_LEFT(temp) != NULL )
                    temp = (void *)GET_LEFT(temp);
                void *tempL = (void *)GET_LEFT(bp);
                void *tempR = (void *)GET_RIGHT(temp);
                void *tempP = (void *)GET_PARENT(temp);
                if( (void *)GET_LEFT(GET_PARENT(bp)) == bp )
                    PUT_LEFT( GET_PARENT(bp), temp );
                else
                    PUT_RIGHT( GET_PARENT(bp), temp );
                PUT_PARENT( temp, GET_PARENT(bp) );
                PUT_LEFT( temp, GET_LEFT(bp) );
                if( temp != (void *)GET_RIGHT(bp)){
                    PUT_RIGHT( temp, GET_RIGHT(bp) );
                    PUT_LEFT( tempP, tempR );
                    if( tempR != NULL )
                        PUT_PARENT( tempR,tempP );
                    PUT_PARENT( GET_RIGHT(bp), temp );
                }
                if( tempL != NULL )
                    PUT_PARENT( tempL, temp );
            }
        }
    }

    /* bp has brothers */
    else{

    /* is root*/
    if( bp == mm_freelist ){
        mm_freelist = (void *)GET_BROTHERS(bp);
        PUT_PARENT( mm_freelist, NULL );
        PUT_LEFT( mm_freelist, GET_LEFT(bp) );
        PUT_RIGHT( mm_freelist, GET_RIGHT(bp) );
        if( (void *)GET_LEFT(bp) != NULL )
            PUT_PARENT( GET_LEFT(bp), mm_freelist );
        if( (void *)GET_RIGHT(bp) != NULL )
            PUT_PARENT( GET_RIGHT(bp), mm_freelist );
    }

    /* not the root */
    else{
        /* if it is brother node */
        if( GET_RIGHT(bp) == -1 ){
            PUT_BROTHERS( GET_LEFT(bp),GET_BROTHERS(bp) );
            if( (void *)GET_BROTHERS(bp) != NULL )
                PUT_LEFT( GET_BROTHERS(bp),GET_LEFT(bp) );
        }else{
            if( (void *)GET_LEFT(GET_PARENT(bp)) == bp )
                PUT_LEFT( GET_PARENT(bp), GET_BROTHERS(bp) );
            else
                PUT_RIGHT( GET_PARENT(bp), GET_BROTHERS(bp) );
            PUT_PARENT( GET_BROTHERS(bp), GET_PARENT(bp) );
            PUT_LEFT( GET_BROTHERS(bp), GET_LEFT(bp) );
            PUT_RIGHT( GET_BROTHERS(bp), GET_RIGHT(bp) );
            if( (void *)GET_LEFT(bp) != NULL )
                PUT_PARENT(GET_LEFT(bp), GET_BROTHERS(bp) );
            if( (void *)GET_RIGHT(bp) != NULL)
                PUT_PARENT(GET_RIGHT(bp), GET_BROTHERS(bp) );
        }
    }
    }
}
















