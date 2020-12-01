#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};

//struct _block *tracker = NULL;
struct _block *heapList = NULL; /* Free list to track the _blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size)
{
   struct _block *curr = heapList;
   //keeps track of the block for next fit
   static struct _block * tracker = NULL;
   if( tracker == NULL ) tracker = heapList;


#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size))
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
size_t h=0;
size_t y=0;
struct _block *temp = NULL;
//run this loop when curr is not null
while(curr)
{
   *last = curr;
   //if current block is free and current free block is greater than the allocated size
   //then execute this condition
   if(curr->free && curr->size >= size)
   {
      //hold the size to compare
       h=curr->size;
       //first value gets stored in y
      if(y==0)
      {
        y = curr->size;
        //store current block address on temp
        temp = curr;
     }
     //if y is not 0
    else
    {
       //compare recent and stored and store the smallest
      if(h < y)
      {
        y =curr->size;
        //save the address of the smallest block
        temp = curr;
      }

    }
 }
 //increment pointer to next block
   curr = curr->next;
}
//return the address of block which leaves the least leftover
curr = temp;
#endif

#if defined WORST && WORST == 0
size_t hold =0;
struct _block *final = NULL;
while(curr)
{
   *last = curr;
   //if current block is free and size is grater than the requested size, then store it
   //hold is also zero in the beginnig, so it will store the first free block
   if(hold == 0 && curr->free && curr->size >= size)
   {
      //store current size and hold current address to return
      hold = curr->size;
      final = curr;
   }
   //hold is not zero and current block is free
   else
   {
      //store the free block that gives the most leftover
       if(curr->free && curr->size >= size && hold < curr->size)
      {
         hold = curr->size;
         final = curr;
      }
   }
   //increment pointer to next block
   curr = curr->next;
}
//store the final address to be returned
curr = final;


#endif

#if defined NEXT && NEXT == 0
//while tracker is not null and its free size is greater than the allocated size
while(tracker && !(tracker->free && tracker->size >=size ))
{
   *last = tracker;
   tracker = tracker->next;
}
curr = tracker;

#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size)
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1)
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL)
   {
      heapList = curr;
   }

   /* Attach new _block to prev _block */
   if (last)
   {
      last->next = curr;
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process
 * or NULL if failed
 */
void *malloc(size_t size)
{
   //store the requested size, num of malloc calls
   num_requested = size;
   num_mallocs++;
   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0)
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: Split free _block if possible */

   size_t block_size = sizeof(struct _block);
   struct _block * c = next;
  if(c!=NULL)
  {
      if((c->size - size) > block_size)
      {
         //increase blocks and number of splits
         num_splits++;
         num_blocks++;
         //hold current size
         size_t old_size = c->size;
         //hold the next address
         struct _block *hold1= c->next;
         //add header size, current malloced size on the starting address to get to the free address
         //inside the same block
         uint8_t *ptr =(uint8_t*)c + size + block_size;
         c->next=(struct _block*) ptr;
         c->size = size;
         //substract malloced and header size to get the new free address
         c->next->size = old_size - size - block_size;
         //point to the next address
         c->next->next = hold1;
         //declare the remaining block as free
         c->next->free = true;

      }
   }

   /* Could not find free _block, so grow heap */
   if (next == NULL)
   {
      next = growHeap(last, size);
      //add the size to find the max heap
      max_heap = max_heap + next->size;
      //increase heap grow and number of blocks
      num_grows++;
      num_blocks++;

   }
   else
   {
      //increase reuses and number of blocks
      num_reuses++;
      num_blocks++;
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL)
   {
      return NULL;
   }

   /* Mark _block as in use */
   next->free = false;


   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}

void *calloc(size_t nmemb, size_t size)
{
   //malloc size * total number
   void *ptr = malloc(nmemb * size);
   //initialize with zeros
   memset(ptr,0,nmemb * size);
   return ptr;
}

void *realloc(void *ptr, size_t size)
{
   //malloc size
   void *ptr1 = malloc(size);
   //copy from old to new
   memcpy(ptr1, ptr, size);
   return ptr1;
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr)
{
   if (ptr == NULL)
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   //count total frees
   num_frees++;

   /* TODO: Coalesce free _blocks if needed */
   //struct _block *hold=NULL;
   curr = heapList;
   while(curr)
   {
      //if there is next block free block and curent block is free
      if(curr->next && curr->free && curr->next->free)
      {
         //increase the value
         num_coalesces++;
         struct _block * old_next;
         //hold the address pointing by the next block
         old_next = curr->next->next;
         //add the size to combine
         curr->size = curr->size + curr->next->size + sizeof(struct _block);
         //now point to next block after combining the free block next to current block
         curr->next = old_next;
#if 0
         hold = curr->next;
         if(hold->next)
         {
            curr = hold->next;
            continue;
         }
         else
         {
            break;
         }
#endif
      }
      //increment block if not free
      curr = curr->next;
   }

}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
