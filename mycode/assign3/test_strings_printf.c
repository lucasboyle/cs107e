/* Lucas Boyle
 * 2/3/26
 * CS 107e
 * 
 * File: test_strings_printf.c
 * ---------------------------
 * This file provides a thorough testing framework for the printf.c and strings.c functions.
 */

#include "assert.h"
#include "printf.h"
#include <stddef.h>
#include "strings.h"
#include "uart.h"

// PROTOTYPES FROM printf.c TO ALLOW FOR EASIER TESTING
void num_to_string(unsigned long num, int base, char *outstr);
const char *hex_string(unsigned long val);
const char *decimal_string(long val);


/*
 * This function provides tests for the memset function in strings.c.
 */
static void test_memset(void) {
    
    // TEST SETTING MEMORY AND CHECKING CHANGE
    char buf[25];
    size_t bufsize = sizeof(buf);
    memset(buf, 0x7e, bufsize);
    for (int i = 0; i < bufsize; i++) {
        assert(buf[i] == 0x7e);
    }

    // CHECK THAT MEMORY BEFORE AND AFTER BUFFER HAS NOT BEEN CHANGED
    assert(*(buf - 1) != 0x7e);
    assert(buf[bufsize+1] != 0x7e);

    // TEST VALUES LARGER THAN AN UNSIGNED CHAR, SHOULD USE LEAST SIGNIFICANT BYTE
    char big[25];
    size_t bigsize = sizeof(big);
    memset(big, 0x107, bigsize);
    for (int i = 0; i < bigsize; i++) {
        assert(big[i] == 0x07);
    }

    // ONLY SPECIFIED NUMBER OF VALUES SHOULD BE SET
    memset(big, 0x99, bigsize-5);
    for (int i = 0; i < bigsize-5; i++) {
        assert(big[i] == 0x99);
    }
    for (int i = bigsize-5; i < bigsize-5; i++) {
        assert(big[i] == 0x07);
    }
}


/*
 * This function provides tests for the strcmp function in strings.c.
 */
static void test_strcmp(void) {
    assert(strcmp("me", "me") == 0);
    assert(strcmp("A", "A") == 0);
    assert(strcmp("a", "a") == 0);
    assert(strcmp("me", "you") != 0);
    assert(strcmp("Me", "me") != 0);
    assert(strcmp("apple", "orange") < 0);
    assert(strcmp("orange", "apple") > 0);
    assert(strcmp("Apple", "apple") < 0);
    assert(strcmp("apple", "apples") < 0);
    assert(strcmp("apPle", "aPple") > 0);
}


/*
 * This function provides tests for the strlcat function in strings.c.
 */
static void test_strlcat(void) {
    char buf[20];
    size_t bufsize = sizeof(buf);

    // INITIALIZE CONTENTS FOR EASIER DEBUGGING. 
    memset(buf, 0x7e, bufsize); 

    // START WITH AN EMPTY STRING
    buf[0] = '\0';

    // APPEND DIFFERNET INPUTS, TEST LENGTH
    assert(strcmp(buf, "") == 0);
    strlcat(buf, "CS", bufsize);
    assert(strcmp(buf, "CS") == 0);
    strlcat(buf, "107e", bufsize);
    assert(strcmp(buf, "CS107e") == 0);
    assert(strlcat(buf, " is great!", bufsize) == 16);

    // TEST CASE WHERE THE NULL OPERATOR IS NOT FOUND IN THE STRING
    char no_null[15];
    size_t no_null_size = sizeof(no_null);
    memset(no_null, 0x7e, no_null_size);
    assert(strlcat(no_null, "CS107e", no_null_size) == 21); 

    // TEST CASE WHERE dstsize IS SMALLER THAN WHERE THE NULL OPERATOR IS FOUND
    char small[30];
    size_t small_size = sizeof(small);
    memset(small, 0x7e, small_size);
    small[29] = '\0';
    assert(strlcat(small, "CS107e", small_size-10) == small_size-10 + 6);

    // TEST CASE WHERE dstsize IS SMALLER THAN SRC + DST
    char smaller[10];
    memset(smaller, 0x7e, 10);
    smaller[0] = '\0';
    assert(strlcat(smaller, "This is not going to be able to fit in the single string", 10) 
                   == strlen("This is not going to be able to fit in the single string"));
}


/*
 * This function provides tests for the strtonum function in strings.c.
 */
static void test_strtonum(void) {
    
    // TEST SIMPLE CONVERSION
    assert(strtonum("67", NULL) == 67);

    // TEST CONVERSION WITH TEXT AFTER NUMBERS, CHECK NEW POINTER
    const char *input = "107rocks";
    const char *rest = NULL;
    long val = strtonum(input, &rest);
    assert(val == 107);
    assert(rest == &input[3]); 

    // TEST HEX INPUTS (UPPER CASE AND LOWER CASE)
    assert(strtonum("0x7e", NULL) == 16*7 + 0xe);
    assert(strtonum("0x7erocks", NULL) == 16*7 + 0xe);
    assert(strtonum("0x7E", NULL) == 16*7 + 0xe);
    assert(strtonum("0x7Erocks", NULL) == 16*7 + 0xe);
    
    // TEST ENDPTR ON HEX INPUT
    input = "0x67rocks";
    val = strtonum(input, &rest);
    assert(val == 16*6 + 7);
    assert(rest == &input[4]);

    // TEST DEFAULT COVERSION TO 0
    assert(strtonum("\0", NULL) == 0);
    assert(strtonum("0\0", NULL) == 0);
    assert(strtonum("0x\0", NULL) == 0);
}


/*
 * This function provides tests for the num_to_string, decimal_string, and hex_string functions
 * from printf.c.
 */
static void test_helpers(void) {
    
    // INITIALIZE CONTENTS OF ARRAY FOR DEBUGGING
    char buf[32];
    size_t bufsize = sizeof(buf);
    memset(buf, 0x7e, bufsize);

    // TEST RANGE OF NUMBERS
    num_to_string(45, 10, buf);
    assert(strcmp(buf, "45") == 0);
    num_to_string(45, 16, buf);
    assert(strcmp(buf, "2d") == 0);
    num_to_string(002, 10, buf);
    assert(strcmp(buf, "2") == 0);
    num_to_string(0, 10, buf);
    assert(strcmp(buf, "0") == 0);
    num_to_string(0, 16, buf);
    assert(strcmp(buf, "0") == 0);

    // TEST HELPER FUNCTIONS, WHICH ALLOWS A NEGATIVE FOR DECIMALS
    assert(strcmp(decimal_string(-88), "-88") == 0);
    assert(strcmp(decimal_string(-0), "0") == 0);
    assert(strcmp(hex_string(0x107e), "107e") == 0);
}


/*
 * This function provides tests for the snprintf function in printf.c.
 */
static void test_snprintf(void) {
    char buf[100];
    size_t bufsize = sizeof(buf);

    // NO FORMATTING CODES
    int count = snprintf(buf, bufsize, "Hello, world!");
    assert(strcmp(buf, "Hello, world!") == 0);
    assert(count == 13);

    // PERCENT FORMATTING CODE
    count = snprintf(buf, bufsize, "50%%");
    assert(strcmp(buf, "50%") == 0);
    assert(count == 3);

    // ONE STRING FORMATTING CODE
    count = snprintf(buf, bufsize, "%s", "binky");
    assert(strcmp(buf, "binky") == 0);
    assert(count == 5);

    // ONE CHAR FORMATTING CODE
    count = snprintf(buf, bufsize, "%c", 'e');
    assert(strcmp(buf, "e") == 0);
    assert(count == 1);

    // ONE DIGIT FORMATTING CODE
    count = snprintf(buf, bufsize, "%d", 107);
    assert(strcmp(buf, "107") == 0);
    assert(count == 3);

    // ONE HEX FORMATTING CODE
    count = snprintf(buf, bufsize, "%x", 0x107e);
    assert(strcmp(buf, "107e") == 0);
    assert(count == 4);

    // ONE HEX FORMATTING CODE
    count = snprintf(buf, bufsize, "%x", 18);
    assert(strcmp(buf, "12") == 0);
    assert(count == 2);

    // ONE LONG DIGIT FORMATTING CODE
    count = snprintf(buf, bufsize, "%ld", 0xfffffffff);
    assert(strcmp(buf, "68719476735") == 0);
    assert(count == 11);

    // ONE LONG HEX FORMATTING CODE
    count = snprintf(buf, bufsize, "%lx", 0xfffffffff);
    assert(strcmp(buf, "fffffffff") == 0);
    assert(count == 9);

    // ONE LONG HEX FORMATTING CODE
    count = snprintf(buf, 20, "%lx", ~0L);
    assert(strcmp(buf, "ffffffffffffffff") == 0);
    assert(count == 16);

    // ONE POINTER FORMATTING CODE, VERY LONG
    count = snprintf(buf, 20, "%p", (void *)0x100200300400L);
    assert(strcmp(buf, "0x100200300400") == 0);
    assert(count == 14);

    // ONE STRING FORMATTING CODE WITH BEGINNING TEXT
    count = snprintf(buf, bufsize, "dog %s", "bone");
    assert(strcmp(buf, "dog bone") == 0);
    assert(count == strlen("dog bone"));

    // ONE STRING FORMATTING CODE WITH END TEXT
    count = snprintf(buf, bufsize, "%s bottle", "binky");
    assert(strcmp(buf, "binky bottle") == 0);
    assert(count == strlen("binky bottle"));

    // ONE STRING FORMATTING CODE WITH BEGINNING AND END TEXT
    count = snprintf(buf, bufsize, "the %s bottle", "binky");
    assert(strcmp(buf, "the binky bottle") == 0);
    assert(count == strlen("the binky bottle"));

    // TWO STRING FORMATTING CODES
    count = snprintf(buf, bufsize, "%s%s", "binky", "bottle");
    assert(strcmp(buf, "binkybottle") == 0);
    assert(count == strlen("binkybottle"));

    // TWO STRING FORMATTING CODES WITH INNER TEXT
    count = snprintf(buf, bufsize, "%s drinks from the %s", "baby", "bottle");
    assert(strcmp(buf, "baby drinks from the bottle") == 0);
    assert(count == strlen("baby drinks from the bottle"));

    // TWO STRING FORMATTING CODES WITH INNER AND OUTER TEXT
    count = snprintf(buf, bufsize, "the %s drinks from the %s for dinner", "baby", "bottle");
    assert(strcmp(buf, "the baby drinks from the bottle for dinner") == 0);
    assert(count == strlen("the baby drinks from the bottle for dinner"));

    // THREE STRING FORMATTING CODES
    count = snprintf(buf, bufsize, "%s%s%s", "CS", "107e", "!!");
    assert(strcmp(buf, "CS107e!!") == 0);
    assert(count == strlen("CS107e!!"));

    // MIXED FORMATTING CODES
    count = snprintf(buf, bufsize, "50%% is %c cool %s", 'a', "number");
    assert(strcmp(buf, "50% is a cool number") == 0);
    assert(count == strlen("50% is a cool number"));

    // GOING OVER bufsize, NO ARGUMENTS
    count = snprintf(buf, 5, "107e > 107");
    assert(strcmp(buf, "107e") == 0);
    assert(count == strlen("107e > 107"));

    // GOING OVER bufsize, ARGUMENTS AFTER
    count = snprintf(buf, 5, "107e > %s", "107");
    assert(strcmp(buf, "107e") == 0);
    assert(count == strlen("107e > 107"));

    // GOING OVER bufsize, ARGUMENTS AND TEXT AFTER
    count = snprintf(buf, 5, "107e > %s!!", "107");
    assert(strcmp(buf, "107e") == 0);
    assert(count == strlen("107e > 107!!"));

    // GOING OVER bufsize, MIX OF ARGUMENTS AND TEXT AFTER
    count = snprintf(buf, 6, "The %s, the %s, an%c the %s", "lion", "witch", 'd', "wardrobe");
    assert(strcmp(buf, "The l") == 0);
    assert(count == strlen("The lion, the witch, and the wardrobe"));

    // LONG HEX FORMATTING CODE WITH OTHER ARGUMENTS
    count = snprintf(buf, bufsize, "Displaying %s of the letter %c: %lx. Cool!", "lots", 'f', 0xfffffffff);
    assert(strcmp(buf, "Displaying lots of the letter f: fffffffff. Cool!") == 0);
    assert(count == strlen("Displaying lots of the letter f: fffffffff. Cool!"));

    // GOING OVER bufsize, LONG HEX FORMATTING CODE WITH OTHER ARGS
    count = snprintf(buf, 3, "Displaying %s of the letter %c: %lx. Cool!", "lots", 'f', 0xfffffffff);
    assert(strcmp(buf, "Di") == 0);
    assert(count == strlen("Displaying lots of the letter f: fffffffff. Cool!"));

    // GOING OVER bufsize, LONG HEX FORMATTING CODE WITH OTHER ARGS
    count = snprintf(buf, 3, "%lx is a cool number!", 0xfffffffff);
    assert(strcmp(buf, "ff") == 0);
    assert(count == strlen("fffffffff is a cool number!"));

    // PADDING WITH CHAR
    count = snprintf(buf, bufsize, "%10c", 'e');
    assert(strcmp(buf, "         e") == 0);
    assert(count == 10);

    // PADDING WITH CHAR OVER bufsize
    count = snprintf(buf, 5, "%10c", 'e');
    assert(strcmp(buf, "    ") == 0);
    assert(count == 10);

    // PADDING WITH STRING
    count = snprintf(buf, bufsize, "%10s", "CS107e");
    assert(strcmp(buf, "    CS107e") == 0);
    assert(count == 10);

    // PADDING WITH DECIMAL
    count = snprintf(buf, bufsize, "%10d <<<", 107);
    assert(strcmp(buf, "       107 <<<") == 0);
    assert(count == 14);

    // PADDING WITH HEX
    count = snprintf(buf, bufsize, "%10x <<<", 263);
    assert(strcmp(buf, "0000000107 <<<") == 0);
    assert(count == 14);

    // PADDING WITH HEX
    count = snprintf(buf, bufsize, "%2x <<<", 263);
    assert(strcmp(buf, "107 <<<") == 0);
    assert(count == 7);

    // PADDING WITH LONG DECIMAL
    count = snprintf(buf, bufsize, "%15ld", 0xfffffffff);
    assert(strcmp(buf, "    68719476735") == 0);
    assert(count == 15);

    // PADDING WITH LONG HEX
    count = snprintf(buf, bufsize, "%15lx", 0xffffffffff);
    assert(strcmp(buf, "00000ffffffffff") == 0);
    assert(count == 15);

    // PADDING WITH POINTER
    count = snprintf(buf, bufsize, "%p", (void *)0x107);
    assert(strcmp(buf, "0x00000107") == 0);
    assert(count == 10);

    // PADDING WITH POINTER
    count = snprintf(buf, bufsize, "%4p", (void *)0x107);
    assert(strcmp(buf, "0x0107") == 0);
    assert(count == 6);
}


/*
 * This function provides a test to ensure that the printf function in printf.c returns
 * the correct length.  
 */
static void test_printf(void) {
    int count = printf("Test: Displaying %s of the letter %c: %lx. Cool!\n\n", "lots", 'f', 0xfffffffff);
    assert(count == strlen("Test: Displaying lots of the letter f: fffffffff. Cool!\n\n"));
}


// CODE TO PRACTICE DISASSEMBLER ON
int sum(int n) {
    int result = 6;
    for (int i = 0; i < n; i++) {
        result += i * 3;
    }
    return result + 729;
}

/* 
 * This function provides tests for the disassembler created in the extension. 
 */
void test_disassemble(void) {
    const unsigned int add =  0x00f706b3;
    const unsigned int xori = 0x0015c593;
    const unsigned int bne =  0xfe061ce3;
    const unsigned int sd =   0x02113423;
    const unsigned int j =    0x0000006f;

    // formatting code %pI accesses the disassemble extension.
    // If extension not implemented, regular version of printf
    // will simply output pointer address followed by I
    // e.g.  "... disassembles to 0x07ffffd4I"
    printf("Encoded instruction %08x disassembles to %pI\n", add, &add);
    printf("Encoded instruction %08x disassembles to %pI\n", xori, &xori);
    printf("Encoded instruction %08x disassembles to %pI\n", bne, &bne);
    printf("Encoded instruction %08x disassembles to %pI\n", sd, &sd);
    printf("Encoded instruction %08x disassembles to %pI\n", j, &j);

    unsigned int *fn = (unsigned int *)sum; // disassemble instructions from sum function
    for (int i = 0; i < 10; i++) {
        printf("%p:  %08x  %pI\n", &fn[i], fn[i], &fn[i]);
    }
}


/*
 * Main function; runs all tests. 
 */
void main(void) {
    uart_init();
    uart_putstring("Start execute main() in test_strings_printf.c\n");

    test_memset();
    test_strcmp();
    test_strlcat();
    test_strtonum();
    test_helpers();
    test_snprintf();
    test_printf();

    // test_disassemble();

    uart_putstring("Successfully finished executing main() in test_strings_printf.c\n");
}