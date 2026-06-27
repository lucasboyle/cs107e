/* Lucas Boyle 
 * CS 107e
 * 2/8/26
 *
 * File: test_backtrace_malloc.c
 * -----------------------------
 * This file provides a testing framework for backtrace.c and malloc.c. 
 */


#include "assert.h"
#include "backtrace.h"
#include "malloc.h"
#include "printf.h"
#include <stdint.h>
#include "strings.h"
#include "timer.h"
#include "uart.h"


// AVAILABLE IN malloc.c BUT NOT PUBLIC INTERFACE
void heap_dump(const char *label);


// SAMPLE FUNCTION TO HELP TRACING FOR BACKTRACE
static int recursion(int n) {
    printf("\nEnter call recursion(%d):\n", n);
    backtrace_print();
    if (n == 0) {
        return 0;
    } else if (n % 2 == 0) {  // EVEN
        return 2 * recursion(n-1);
    } else {                   // ODD
        return 1 + recursion(n-1);
    }
}


// SAMPLE FUNCTION TO HELP TRACING FOR BACKTRACE
static void show_frames(int nframes) {
    frame_t f[nframes];
    printf("\nEnter call show_frames(%d):\n", nframes);
    int frames_filled = backtrace_gather_frames(f, nframes);

    assert(frames_filled <= nframes);
    printf("Gathered backtrace contains %d frames:\n", frames_filled);
    backtrace_print_frames(f, frames_filled);
    printf("\n");
}


// SAMPLE FUNCTION TO HELP TRACING FOR BACKTRACE
static void silly_goose(int nframes) {
    show_frames(nframes);
}


// SAMPLE FUNCTION TO HELP TRACING FOR BACKTRACE
static void happy_day(void) {
    silly_goose(2);
    silly_goose(6);
    show_frames(3);
}


/*
 * This function helps to test backtrace.c, calling on functions that will print
 * the backtrace for comparison. 
 */
static void test_backtrace(void) {
    happy_day();
    recursion(4);
}


// SAMPLE FUNCTION TO HELP TRACING FOR STACK PROTECTOR
static void mars(void) {
    
    // THIS SHOULD NEVER EXECUTE
    uart_putstring("saying hello from mars.  How did execution get here?\n");
}


// SAMPLE FUNCTION TO HELP TRACING FOR STACK PROTECTOR
static void good_egg(void *val) {
    
    // STACK CANARY SHOULD BE INTACT AT THE END
    void *array[3];

    for (int i = 0; i < 3; i++) {
        array[i] = val;
    }
    printf("good_egg correctly used stack array at addr %p\n", array);
}


// SAMPLE FUNCTION TO HELP TRACING FOR STACK PROTECTOR
static void bad_guy(int num_beyond, void *val) {
    
    // STACK SMASH SHOULD BE DETECTED AT THE END
    int size = 3;
    void *array[size];

    for (int i = 0; i < size + num_beyond; i++) {
        array[i] = val;
    }
    printf("bad_guy wrote %d value(s) past end of stack array at addr %p\n", num_beyond, array);
}


/*
 * This function provides a test for the stack protector. the call to bad_guy() 
 * will write past the end of the stack buffer, which should trigger StackGaurd.
 */
void test_stack_protector(void) {

    printf("\nTesting stack protector, be sure that stack-protector is enabled in Makefile.\n");
    void *val = 0x0;
    // val = (void *)mars;
    // val = (void *)0x10000;

    good_egg(val);
    for (int count = 1; count < 5; count++) {
        printf("\ncall bad_guy(%d) ...\n", count);
        bad_guy(count, val);
        printf("... survived bad_guy(%d).\n", count);
    }
}


/*
 * This function tests the heap dump, allocating and deallocating bits of memory. 
 */
static void test_heap_dump(void) {
    heap_dump("Empty heap");

    int *p = malloc(sizeof(int));
    *p = 0;
    heap_dump("After p = malloc(4)");

    char *q = malloc(16);
    memcpy(q, "aaaaaaaaaaaaaaa", 16);
    heap_dump("After q = malloc(16)");

    free(p);
    heap_dump("After free(p)");

    free(q);
    heap_dump("After free(q)");
}


/*
 * This function tests recycling memory. As each piece of memory is allocated and 
 * freed, it should repeatedly use the same memory.
 */
static void test_recycle(void) {
    for (int i=0; i<107; i++) {
        int *p = malloc(16);
        int *q = malloc(16);
        int *r = malloc(16);
        int *s = malloc(16);
        free(p);
        int *t = malloc(16);
        free(t);
        free(q);
        free(r);
        free(s);
    }
    heap_dump("Should be four 16 byte blocks empty");
}


/*
 * This function tests splitting empty blocks when malloc() is called and an empty 
 * block is available. A large block is allocated and freed, meaning that smaller 
 * blocks should be able to use the freed space. 
 */
static void test_split(void) {
    heap_dump("Pre-splitting");
    int *p = malloc(48);
    heap_dump("Large malloc");
    free(p);
    heap_dump("Freed large");
    int *q = malloc(16);
    heap_dump("malloc(16)");
    int *r = malloc(8);
    heap_dump("malloc(8)");
    free(q);
    free(r);
}


/*
 * This function tests coalescing adjacent blocks when memory is freed. Four small
 * pieces of memory are allocated and then freed in reverse order, which should
 * combine into one larger empty block. 
 */
static void test_coalesce(void) {
    int *p = malloc(8);
    heap_dump("malloc(8)");
    int *q = malloc(8);
    heap_dump("malloc(8)");
    int *r = malloc(8);
    heap_dump("malloc(8)");
    int *s = malloc(8);
    heap_dump("malloc(8)");

    free(s);
    free(r);
    free(q);
    free(p);
    heap_dump("Freed in reverse order");

    malloc(56);
    heap_dump("malloc(56)");
}


/*
 * This function tests simple allocations and freeing of the heap. 
 */
static void test_heap_simple(void) {
    const char *alphabet = "abcdefghijklmnopqrstuvwxyz";
    int len = strlen(alphabet);

    char *str = malloc(len + 1);
    memcpy(str, alphabet, len + 1);

    int n = 10;
    int *arr = malloc(n*sizeof(int));
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }

    assert(strcmp(str, alphabet) == 0);
    free(str);
    assert(arr[0] == 0 && arr[n-1] == n-1);
    free(arr);
}


/*
 * This function tests the edgecases that exist for malloc and free.
 */
static void test_heap_oddballs(void) {
    
    char *ptr;

    ptr = malloc(900000000); // TOO LARGE TO FIT
    assert(ptr == NULL); // SHOULD RETURN NULL
    heap_dump("After reject too-large request");

    ptr = malloc(0);
    heap_dump("After malloc(0)");
    free(ptr);

    free(NULL); // SHOULD DO NOTHING
    heap_dump("After free(NULL)");
}


/*
 * This function tests allocated space for strings of different lengths, 
 * ensuring that the contents stay intact. 
 */
static void test_heap_multiple(void) {
    
    // STRING FILLED WITH REPEATED CHARS, E.G. "A" , "BB" , "CCC"
    int n = 8;
    char *arr[n];

    for (int i = 0; i < n; i++) {
        int num_repeats = i + 1;
        char *ptr = malloc(num_repeats + 1);
        memset(ptr, 'A' - 1 + num_repeats, num_repeats);
        ptr[num_repeats] = '\0';
        arr[i] = ptr;
    }
    heap_dump("After all allocations");
    for (int i = n-1; i >= 0; i--) {
        int len = strlen(arr[i]);
        char first = arr[i][0], last = arr[i][len -1];
        assert(first == 'A' - 1 + len);  // VERIFY CONTENTS
        assert(first == last);
        free(arr[i]);
    }
    heap_dump("After all frees");
}


/*
 * This function tests the detection of memory leaks. Blocks are allocated and 
  * never freed, which should be reported by Valgrind. 
 */
static void test_heap_leaks(void) {
    char *ptr;

    ptr = malloc(9); // LEAKED
    ptr = malloc(5);
    free(ptr);
    ptr = malloc(107); // LEAKED
    malloc_report();
}


/*
 * This function tests the redzone protection for the heap. 
 */
void test_heap_redzones(void) {
    char *ptr;

    ptr = malloc(9);
    memset(ptr, 'a', 9); // WRITE INTO PAYLOAD
    free(ptr); // PTR IS OK

    ptr = malloc(5);
    ptr[-1] = 0x45; // WRITE BEFORE PAYLOAD
    free(ptr);      // PTR IS NOT OKAY

    ptr = malloc(12);
    ptr[13] = 0x45; // WRITE AFTER PAYLOAD
    free(ptr);      // PTR IS NOT OKAY
}


/*
 * The main function holds calls to each of the tests detailed above. Selectively
 * uncomment tests when ready to test different features. 
 */
void main(void) {
    uart_init();
    uart_putstring("Start execute main() in test_backtrace_malloc.c\n");

    // test_backtrace();
    // test_stack_protector();

    // test_heap_dump();
    // test_recycle();
    // test_split();
    // test_coalesce();
    // test_heap_simple();
    // test_heap_oddballs();
    // test_heap_multiple();
    test_heap_leaks();

    //test_heap_redzones(); // DO NOT USE unless you have implemented red zone protection!
    uart_putstring("\nSuccessfully finished executing main() in test_backtrace_malloc.c\n");
}
