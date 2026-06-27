/* Lucas Boyle
 * CS 107e
 * 2/9/26
 * 
 * File: malloc.c
 * --------------
 * This file implements malloc.h, providing functionality for allocating and freeing
 * memory on the heap. 
 */


#include "malloc.h"
#include "backtrace.h"
#include "memmap.h"
#include "printf.h"
#include <stddef.h> // FOR NULL
#include "strings.h"


// DATA VARIABLES TO HELP WITH DEBUGGING
static int count_allocs, count_frees, total_bytes_requested;

/*
 * The segment of memory available for the heap runs from HEAP_START
 * to HEAP_MAX (markers placed in memmap.ld establish these boundaries,
 * constants declared in memmap.h)
 *
 * The pointer variable cur_heap_end is initialized to HEAP_START and
 * is adjusted upward as in-use portion of heap segment enlarges.
 * Because cur_head_end is qualified as static, this variable
 * is not stored in stack frame, instead variable is located in data segment.
 * The one variable is shared by all and retains its value between calls.
 */


 /*
  * This function extends the allocated heap by nbytes, moving the cur_heap_end
  * to accomodate for the requested bytes. If there is not enough room left in 
  * the heap, the fuction will return NULL. 
  */
void *sbrk(size_t nbytes) {
    
    // IMPORTANT: DECLARED static
    static void *cur_heap_end = HEAP_START;

    // EXTEND HEAP END
    void *new_heap_end = (char *)cur_heap_end + nbytes;
    
    // IF REQUEST WOULD EXTEND BEYOND HEAP MAX, RETURN NULL
    if (new_heap_end > HEAP_MAX) {
        return NULL;
    }

    // SET NEW HEAP END AND RETURN PREVIOUS
    void *prev_heap_end = cur_heap_end;
    cur_heap_end = new_heap_end;
    return prev_heap_end;
}


// MACRO TO ROUND UP x TO MULTIPLE OF n
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

// DEFINE THE HEADER SIZE AS 8 (TWO 4-BYTE INTS)
#define HEADER_SIZE sizeof(header)

// CREATE THE HEADER STRUCT
typedef struct {
    int payload_size;
    int requested_size;
    int status;         // 0 IF FREE, 1 IF IN USE
    frame_t back_frames[3];
} header;


/*
 * This function takes in a number of requested bytes and will return a pointer
 * to the next header that is open and has enough space to hold the data. 
 */
header *next_fitting_block(size_t nbytes) {
    
    // SET BOUNDARIES AT START AND CURRENT END OF THE HEAP
    void *curr = HEAP_START;
    void *cur_heap_end = sbrk(0);

    // WHILE THERE IS ENOUGH SPACE FOR ANOTHER HEADER AND DATA TO FIT IN THE HEAP
    while (((char *)curr + HEADER_SIZE + 8) <= ((char *)cur_heap_end)) {
        header *curr_header = (header *)curr;
        
        // DETERMINE IF THE DATA CAN FIT IN THE HEADER
        if ((curr_header->payload_size >= nbytes) && (curr_header->status == 0)) {
            return curr_header;
        }

        // IF IT DID NOT FIT, ADVANCE TO THE NEXT HEADER
        curr = (void *)((char *)curr + HEADER_SIZE + curr_header->payload_size);
    }

    return NULL;
}


/*
 * This function takes in a number of bytes and will allocate memory to hold them,
 * include a header at the beginning and spliting open blocks if necessary.
 */
void *malloc (size_t nbytes) {
 
    // IF 0 BYTES ARE REQUESTED, RETURN NULL
    if (nbytes == 0) {
        return NULL;
    }

    // INCREASE DATA VARIABLES
    count_allocs++;
    total_bytes_requested += nbytes;
    
    // ROUND THE NUMBER OF BYTES AND FIND THE NEXT OPEN BLOCK
    int requested = nbytes;
    nbytes = roundup(nbytes, 8);
    header* open_block = next_fitting_block(nbytes);

    if (open_block) {

        // IF A BLOCK WAS OPEN, STORE THE DATA THERE
        int og_size = open_block->payload_size;
        open_block->payload_size = nbytes;
        open_block->status = 1;
        open_block->requested_size = requested;

        frame_t frames[4];
        int num = backtrace_gather_frames(frames, 4);
        for (int i=0; i<3 && num-i>0; i++) {
            open_block->back_frames[i] = frames[i+1];
        }

        // IF THE SIZE OF THE BLOCK WAS PREVIOUS LARGE ENOUGH TO SPLIT, SPLIT IT
        if (og_size >= (nbytes + HEADER_SIZE + 8)) {
            header *header_ptr = (header *)((char *)open_block + nbytes + HEADER_SIZE);
            header_ptr->payload_size = og_size - nbytes - HEADER_SIZE;
            header_ptr->status = 0;
        } else {
            open_block->payload_size = og_size;
        }

        // RETURN THE ADDRESS TO THE START OF THE DATA
        return (void *)(open_block + 1);

    } else {
    
        // IF THERE WERE NO OPEN BLOCKS, ALLOCATE A NEW HEADER AT THE END OF THE HEAP
        header *header_ptr = (header *)(sbrk(nbytes + HEADER_SIZE));

        // IF THERE IS NOT ENOUGH MEMORY LEFT TO ALLOCATE, RETURN NULL
        if (!header_ptr) {      
            return NULL;
        }

        // SET THE HEADER TO APPROPRIATE VALUES
        header_ptr->payload_size = nbytes;
        header_ptr->status = 1;
        header_ptr->requested_size = requested;
        
        frame_t frames[4];
        int num = backtrace_gather_frames(frames, 4);
        for (int i=0; i<3 && num-i>0; i++) {
            header_ptr->back_frames[i] = frames[i+1];
        }

        // RETURN THE ADDRESS TO THE START OF THE DATA
        return (void *)(header_ptr + 1);
    }
}


/*
 * This function frees the memory associated with the data passed in by the ptr. 
 * If the freed block is adjacent to another open block, they will merge to form
 * a larger open block. 
 */
void free (void *ptr) {

    // IF THE POINTER IS INVALID, RETURN
    if (!ptr) {
        return;
    }

    // STORE THE CURRENT END OF THE HEAP
    void *cur_heap_end = sbrk(0);
    count_frees++;

    // SET THE HEADER FOR THE DATA TO BE 0
    header *header_ptr = (header *)((char *)ptr - HEADER_SIZE);
    header_ptr->status = 0;

    // CHECK IF THE NEXT BLOCK IS EMPTY AND MERGE THEM IF NEEDED
    header *adj_header = (header *)(((char *)header_ptr) + HEADER_SIZE + header_ptr->payload_size);
    while (((void *)adj_header < cur_heap_end) && (adj_header->status == 0)) {
        header_ptr->payload_size += adj_header->payload_size + HEADER_SIZE;

        adj_header = (header *)(((char *)header_ptr) + HEADER_SIZE + header_ptr->payload_size);
    }
}


/*
 * This function will print out each header's information currently in the heap
 * for debugging purposes. 
 */
void heap_dump (const char *label) {
    void *cur_heap_end = sbrk(0);
    printf("\n---------- HEAP DUMP (%s) ----------\n", label);
    printf("Heap segment at %p - %p\n", HEAP_START, cur_heap_end);

    void *curr_header = HEAP_START;

    // WHILE THERE CAN STILL BE A HEADER IN THE HEAP, PRINT OUT THE SIZE AND STATUS
    while ((char *)curr_header < ((char *)cur_heap_end - HEADER_SIZE)) {
        int size = ((header *)curr_header)->payload_size;
        int status = ((header *)curr_header)->status;
        printf("Payload address: %p, Payload Size: %d, Status: %d\n", (void *)curr_header, size, status);

        curr_header = (void *)(((char *)curr_header) + HEADER_SIZE + size);
    }

    printf("----------  END DUMP (%s) ----------\n", label);
    printf("Stats: %d in-use (%d allocs, %d frees), %d total payload bytes requested\n\n",
        count_allocs - count_frees, count_allocs, count_frees, total_bytes_requested);
}


/*
 * This function print out a malloc report, displaying how many allocs, frees, and 
 * payload bytes were requested by the user, along with the number of leaked 
 * bytes. If memory is leaked, a backtrace will also be printed for the user to 
 * review. 
 */
void malloc_report (void) {
    printf("\n=============================================\n");
    printf(  "         Mini-Valgrind Malloc Report         \n");
    printf(  "=============================================\n");

    printf("final stats: %d allocs, %d frees, %d total payload bytes requested\n\n", count_allocs, count_frees, total_bytes_requested);
    
    void *cur_heap_end = sbrk(0);
    void *curr_header = HEAP_START;

    // WHILE THERE CAN STILL BE A HEADER IN THE HEAP
    while ((char *)curr_header < ((char *)cur_heap_end - HEADER_SIZE)) {
        
        int num = ((header *)curr_header)->requested_size;

        // IF THE MEMORY WAS NOT FREED, DISPLAY APPROPRIATE INFORMATION
        if (((header *)curr_header)->status == 1) {
            printf("%d bytes are lost, allocated by\n", num);
            backtrace_print_frames(((header *)curr_header)->back_frames, 3);
            printf("\n");
        }
        curr_header = (void *)(((char *)curr_header) + HEADER_SIZE + ((header *)curr_header)->payload_size);
    }

}


/*
 * This function will...
 */
void report_damaged_redzone (void *ptr) {
    printf("\n=============================================\n");
    printf(  " **********  Mini-Valgrind Alert  ********** \n");
    printf(  "=============================================\n");
    printf("Attempt to free address %p that has damaged red zone(s):", ptr);
    /***** TODO EXTENSION: Your code goes here if implementing extension *****/
}
