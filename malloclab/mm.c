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

#define SET_PTR(p,ptr) (*(unsigned int*)(p) = (unsigned int)(ptr))

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
#define LIST_SIZE 16

void* list[LIST_SIZE];

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *place(void *bp,size_t asize);
static void insert(void *bp,size_t size);
static void delete(void *bp);



//used whenever initialize a new heap or 
//when mm_malloc can not find a fit block,so as to
//keep alignment extend_heap will request for a number multiple of
//8 bytes, then ask for extra heap space 
static void *extend_heap(size_t size){

    char *bp;
    size=  ALIGN(size);

    if((long)(bp=mem_sbrk(size))==-1)
        return NULL;

    //free block header
    PUT(HDRP(bp),PACK(size,0));
    //free block footer
    PUT(FTRP(bp),PACK(size,0));
    //new epilogue header
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));

    insert(bp,size);

    return coalesce(bp);
}


static void insert(void *ptr,size_t size){

    int index = 0;
    void *search_ptr = NULL;
    void *insert_ptr = NULL;

    while(index<(LIST_SIZE-1)&&(size>1)){
        size>>=1;
        index++;
    }

    search_ptr = list[index];

    while((search_ptr!=NULL)&&(size>GET_SIZE(HDRP(search_ptr)))){
        insert_ptr = search_ptr;
        search_ptr = PRED(search_ptr);
    }

    if(search_ptr!=NULL){
        //1.xx->insert->xx
        //left SUCC  right PRED
        //insert_ptr left  search_ptr right
        if(insert_ptr!=NULL){
            
            SET_PTR(PRED_PTR(ptr),search_ptr);
            SET_PTR(SUCC_PTR(search_ptr),ptr);
            SET_PTR(SUCC_PTR(ptr),insert_ptr);
            SET_PTR(PRED_PTR(insert_ptr),ptr);
        }
        //list[idx]->insert->xx
        else{
            SET_PTR(PRED_PTR(ptr),search_ptr);
            SET_PTR(SUCC_PTR(search_ptr),ptr);
            SET_PTR(SUCC_PTR(ptr),NULL);
            list[index] = ptr;
        }
    }
    else{
        //->xxxxx->insert
        if(insert_ptr!=NULL){

            SET_PTR(PRED_PTR(ptr),NULL);
            SET_PTR(SUCC_PTR(ptr),insert_ptr);
            SET_PTR(PRED_PTR(insert_ptr),ptr);
        }
        //list[idx]->insert
        //first time insert;
        else{
            SET_PTR(PRED_PTR(ptr),NULL);
            SET_PTR(SUCC_PTR(ptr),NULL);
            list[index] = ptr;
        }
    }
}

static void delete(void *ptr){

    int index = 0;
    size_t size = GET_SIZE(HDRP(ptr));

    while((index<LIST_SIZE-1)&&(size>1)){
        size>>=1;
        index++;
    }

  
    //left SUCC  right PRED
    if(PRED(ptr)!=NULL){
          //1.xx->ptr->xx
        if(SUCC(ptr)!=NULL){
            SET_PTR(SUCC_PTR(PRED(ptr)),SUCC(ptr));
            SET_PTR(PRED_PTR(SUCC(ptr)),PRED(ptr));
        }
        //first one [index]->ptr->xxx
        else{
            //set  xxx's succ pointer to NULL;
            SET_PTR(SUCC_PTR(PRED(ptr)),NULL);
            list[index] = PRED(ptr);

        }

    }
    else{
        // xx->ptr
        if(SUCC(ptr)!=NULL){
            SET_PTR(PRED_PTR(SUCC(ptr)),NULL);
        }
        // list[index]->ptr;
        else{
            list[index] = NULL;
        }
    }
}


static void *coalesce(void *bp){

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc&&next_alloc)
        return bp;
    else if(!prev_alloc && next_alloc){
        delete(bp);
        delete(PREV_BLKP(bp));
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    else if(prev_alloc&&!next_alloc){
        delete(bp);
        delete(NEXT_BLKP(bp));
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else{
        delete(bp);
        delete(PREV_BLKP(bp));
        delete(NEXT_BLKP(bp));
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    insert(bp,size);
    return bp;

}


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    void *heap;
    if((heap=mem_sbrk(4*WSIZE))==(void*)-1)
        return -1;
    
    int idx;

    for(idx=0;idx<LIST_SIZE;idx++){
        list[idx] = NULL;
    }
    //alignment padding
    PUT(heap,0);
    PUT(heap+(1*WSIZE),PACK(DSIZE,1));  //prologue header
    PUT(heap+(2*WSIZE),PACK(DSIZE,1)); //prologue footer
    PUT(heap+(3*WSIZE),PACK(0,1)); //Epilogue header

    if(extend_heap(CHUNKSIZE/WSIZE)==NULL)
        return -1;
    return 0;
}



static void*place(void *ptr,size_t size){

    size_t ptr_size = GET_SIZE(HDRP(ptr));
    //remain size after allocating size
    size_t remainSize = ptr_size-size;

    delete(ptr);

    if(remainSize<DSIZE*2){

        PUT(HDRP(ptr),PACK(ptr_size,1));
        PUT(FTRP(ptr),PACK(ptr_size,1));
    }
    else{
        PUT(HDRP(ptr),PACK(size,1));
        PUT(FTRP(ptr),PACK(size,1));
        PUT(HDRP(NEXT_BLKP(ptr)),PACK(remainSize,0));
        PUT(FTRP(NEXT_BLKP(ptr)),PACK(remainSize,0));
        insert(NEXT_BLKP(ptr),remainSize);
    }
    return ptr;
}




/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

void *mm_malloc(size_t size)
{
    //ignore spurious requeses
    if(size==0)
        return NULL;
    
    if(size<=DSIZE)
        size = 2*DSIZE;
    else
        size = ALIGN(size+DSIZE);

    int idx = 0;
    size_t ssize = size;
    void *ptr = NULL;

    while(idx<LIST_SIZE){

        if((ssize<=1)&&(list[idx]!=NULL)){
            ptr = list[idx];
            while(ptr&&((size>GET_SIZE(HDRP(ptr))))){
                ptr = PRED(ptr);
            }
            if(ptr!=NULL)
                break;
        }

        ssize>>=1;
        idx++;
    }

    //if not fit block then extend the heap
    if(ptr==NULL){
        int extendsize = MAX(size,CHUNKSIZE);
        if((ptr=extend_heap(extendsize))==NULL)
            return NULL;
    }

    //alocate blocks that are size's number in free blocks
    ptr = place(ptr,size);

    return ptr;
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    if(bp==0)
        return;

    size_t size = GET_SIZE(HDRP(bp));
    insert(bp,size);
    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{

   void *new_block = ptr;
   int remainSize;

    if(size==0)
    {
        mm_free(ptr);
        return 0;
    }
    if(ptr==NULL)
        return mm_malloc(size);

    if(size<=DSIZE){
        size = 2*DSIZE;
    }
    else{
        size = ALIGN(size+DSIZE);
    }

    if((remainSize=GET_SIZE(HDRP(ptr))-size)>=0)
        return ptr;
    else if(!GET_ALLOC(HDRP(NEXT_BLKP(ptr)))||!GET_SIZE(HDRP(NEXT_BLKP(ptr))))
    {
        if((remainSize = GET_SIZE(HDRP(ptr))+GET_SIZE(HDRP(NEXT_BLKP(ptr)))-size)<0){
            if(extend_heap(MAX(remainSize,CHUNKSIZE))==NULL)
                return NULL;
            remainSize +=MAX(remainSize,CHUNKSIZE);
        }

        delete(NEXT_BLKP(ptr));
        PUT(HDRP(ptr),PACK(size+remainSize,1));
        PUT(FTRP(ptr),PACK(size+remainSize,1));
    }

    else{

        new_block = mm_malloc(size);
        memcpy(new_block,ptr,GET_SIZE(HDRP(ptr)));
        mm_free(ptr);

    }
    return new_block;
}