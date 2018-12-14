/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
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
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define WSIZE 4 //word and header/footer size
#define DSIZE 8 //double word size (bytes)

#define INITCHUNKSIZE (1<<6) //the size of extended heap 
#define CHUNKSIZE (1<<12) //extend heap 

#define MAX(x,y) ((x)>(y))?(x):(y)
#define MIN(x,y) ((x)<(y))?(x):(y)

#define PACK(size,alloc) ((size)|(alloc))

//read and write a word at addr p
#define GET(p) (*(unsigned int*)(p))
#define PUT(p,val) (*(unsigned int*)(p) = (val))

#define GET_SIZE(p)  (GET(p)&~0x7)
#define GET_ALLOC(p)  (GET(p)&0x1)

//given block ptr bp,compute address of its header and footer
#define HDRP(bp)  ((char*)(bp)-WSIZE)
// (char*bp)+size -wsize= next's block's bp 's position,-DSIZE,that means 
//  previous block's foot address
#define FTRP(bp)  ((char*)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp)-WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp)-DSIZE))

#define PRED_PTR(ptr) ((char*)(ptr))
#define SUCC_PTR(ptr) ((char*)(ptr)+WSIZE)

#define PRED(ptr) (*(char **)(ptr))
#define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))


static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t size);
static void place(void *bp,size_t asize);
//points tp the prolouge block
static char* heap_listp = 0;



static void* find_fit(size_t asize){

    void *bp;

    for(bp=heap_listp;GET_SIZE(HDRP(bp))>0;bp=NEXT_BLKP(bp)){
        if(!GET_ALLOC(HDRP(bp))&&(asize<=GET_SIZE(HDRP(bp)))){
            return bp;
        }
    }
    return NULL;
}


static void place(void *bp,size_t asize){

    size_t csize = GET_SIZE(HDRP(bp));
    //when the splitted block >= the minimum block
    //then we split the block
    //before moving to next block,we need to place a new allocated block
    if((csize-asize)>=(2*DSIZE)){

        PUT(HDRP(bp),PACK(asize,1));
        PUT(FTRP(bp),PACK(asize,1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp),PACK(csize-asize,0));
        PUT(FTRP(bp),PACK(csize-asize,0));
    }else{
        PUT(HDRP(bp),PACK(csize,1));
        PUT(FTRP(bp),PACK(csize,1));
    }
}

//used whenever initialize a new heap or 
//when mm_malloc can not find a fit block,so as to
//keep alignment extend_heap will request for a number multiple of
//8 bytes, then ask for extra heap space 
static void *extend_heap(size_t words){

    char *bp;
    size_t size;

    size = (words%2)?((words+1)*WSIZE):words*WSIZE;

    if((long)(bp=mem_sbrk(size))==-1)
        return NULL;

    //free block header
    PUT(HDRP(bp),PACK(size,0));
    //free block footer
    PUT(FTRP(bp),PACK(size,0));
    //new epilogue header
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));

    return coalesce(bp);
}


static void *coalesce(void *bp){

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc&&next_alloc)
        return bp;
    else if(!prev_alloc && next_alloc){
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    else if(prev_alloc&&!next_alloc){
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else{
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    return bp;

}


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((heap_listp=mem_sbrk(4*WSIZE))==(void*)-1)
        return -1;
    //alignment padding
    PUT(heap_listp,0);
    PUT(heap_listp+(1*WSIZE),PACK(DSIZE,1));  //prologue header
    PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1)); //prologue footer
    PUT(heap_listp+(3*WSIZE),PACK(0,1)); //Epilogue header

    //always points to prologue block
    heap_listp = heap_listp+(2*WSIZE);

    if(extend_heap(CHUNKSIZE/WSIZE)==NULL)
        return -1;
    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

void *mm_malloc(size_t size)
{

    size_t asize;  //adjust block size
    size_t extendsize; //amount to extend heap if no fit
    char *bp;

    //ignore spurious requeses
    if(size==0)
        return NULL;
    
    if(size<=DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE*((size+(DSIZE)+(DSIZE-1))/DSIZE);

    //search the free list for a fit
    if((bp=find_fit(asize))!=NULL){
        place(bp,asize);
        return bp;
    }

    extendsize = MAX(asize,CHUNKSIZE);
    if((bp=extend_heap(extendsize/WSIZE))==NULL)
        return NULL;
    place(bp,asize);
    return bp;
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    if(bp==0)
        return;

    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    if(size==0)
    {
        mm_free(ptr);
        return 0;
    }

    if(ptr==NULL)
        return mm_malloc(size);


    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    //copy old block
    copySize = GET_SIZE(HDRP(ptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
