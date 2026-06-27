/* Lucas Boyle
 * 2/3/26
 * CS 107e
 * 
 * File: printf.c
 * --------------
 * This file implements the printf.h header, incorporating functionality for printing strings
 * with various passed arguments. This is split into three functions that allow for a varied
 * number of arguments to be passed and functions to convert from numbers to strings. 
 */

#include "printf.h"
#include <stdarg.h>
#include <stdint.h>
#include "strings.h"
#include "uart.h"

/* 
 * Prototypes for internal helpers. In order to call them from the test program, 
 * must declare externally. 
 */
void num_to_string(unsigned long num, int base, char *outstr);
const char *hex_string(unsigned long val);
const char *decimal_string(long val);

// DEFINE THE MAX NUMBER OF DIGITS FOR A LONG AND SPACE FOR NEGATIVE AND NULL TERMINATOR
#define MAX_DIGITS 25


 /*
  * This function converts an unsigned long into a hex string, calling upon the 
  * num_to_string() function. The static qualifier is added to the buffer to ensure
  * the buffer memory is accessible after the function returns.  
  */
const char *hex_string(unsigned long val) {
    static char buf[MAX_DIGITS];
    num_to_string(val, 16, buf); 
    return buf;
}


/*
  * This function converts a long into a decimal string, calling upon the 
  * num_to_string() function. The static qualifier is added to the buffer to ensure
  * the buffer memory is accessible after the function returns.  
  */
const char *decimal_string(long val) {
    static char buf[MAX_DIGITS];
    if (val < 0) {
        buf[0] = '-'; 
        num_to_string(-val, 10, buf + 1);
    } else {
        num_to_string(val, 10, buf);
    }
    return buf;
}


/*
 * This function gets the char equivalent for an inputted number using the ASCII table. 
 */
static unsigned char get_char(unsigned char num) {
    if (num <= 9) {
        return num + '0';
    } else {
        return num + 'a' - 10;
    }
}


/*
 * This function converts an unsigned long to a string. It will also set the outstr pointer
 * to this new string so it can be acessed outide of the function. 
 */
void num_to_string(unsigned long num, int base, char *outstr) {
    
    // CREATE TEMP ARRAY WITH MAX POSSIBLE DIGITS
    char temp[MAX_DIGITS];

    // INITIALIZE COUNTER TO 0
    size_t n = 0;

    // ACCOUNT FOR EDGE CASE: NUM = 0
    temp[0] = '0';
    if (num == 0) {
        n++;
    }
    
    // CONVERT EACH DIGIT AND STORE IN TEMP ARRAY IN REVERSE
    while (num != 0) {
        unsigned char current_digit = num % base;
        temp[n] = get_char(current_digit);
        num /= base;
        n++;
    }

    // CREATE OUTPUT ARRAY; ONE SIZE LARGER THAN TEMP TO ACCOUNT FOR NULL
    char output[n+1];

    // COPY ELEMENTS OVER IN REVERSE
    for (size_t i = 0; i < n; i++) {
        output[i] = temp[n-i-1];
    }

    // ADD NULL TO THE END
    output[n] = '\0';

    // COPY OUTPUT TO THE outstr POINTER
    memcpy(outstr, output, n+1);
}


/*
 * This function applies an inputted amount of padding to string. The character used to pad
 * can be specified. The function will return the amount of padding that would be added, assuming
 * that enough space is available in the buffer. 
 */
static size_t pad(char *buf, size_t bufsize, size_t pad_size, size_t str_size, int character) {

    if (str_size >= pad_size) {
        return 0;
    }
    
    // DETERMINE THE LENGTH OF THE PADDING
    size_t padded_len = pad_size - str_size;
    
    // CREATE AND APPLY THE PADDING
    char padding[padded_len + 1];
    memset(padding, character, padded_len);
    padding[padded_len] = '\0';
    strlcat(buf, padding, bufsize);

    return padded_len;
}


/*
 * This function applies the printf formatting, writing the formatted string to the 
 * inputted buffer. The list of arguments can vary in size, but there must be one 
 * argument for each % identified in the format. The string will not exceed bufsize 
 * in length. If the formatted string fits within the size, the length of this string
 * will be returned. If not, the length of the string assuming that there would be space
 * is returned. 
 */
int vsnprintf(char *buf, size_t bufsize, const char *format, va_list args) {

    // MAKE THE BUFFER AN EMPTY STRING IF ALLOWED
    if (bufsize > 0) {
        buf[0] = '\0';
    }
    
    // CREATE A POINTER TO TRAVERSE THE FORMAT STRING
    const char *curr = format;

    size_t length = 0;
    
    // CONTINUE TO LOOP WHILE THE NULL TERMINATOR IS NOT REACHED
    while (*curr != '\0') {

        // CREATE A POINTER TO POINT TO THE STRING THAT WILL BE APPENDED
        const char *str_arg = NULL;
        char temp_buf[MAX_DIGITS];

        // HOLD THE CURRENT LENGTH OF THE BUFFER
        size_t buf_len = strlen(buf);
        char buf_char = ' ';

        // IF THERE IS A ARGUMENT, READ IT PROPERLY AND ADD IT TO THE BUFFER
        if (*curr == '%') {
            curr++;
            unsigned long pad_size = strtonum(curr, &curr);

            if (*curr == '%') {     // % ARGUMENT
                length += strlcat(buf, "%", bufsize) - buf_len;
                curr++;
                continue;

            } else if (*curr == 'c') {      // CHARACTER ARGUMENT
                temp_buf[0] = (char)va_arg(args, int);
                temp_buf[1] = '\0';
                str_arg = temp_buf;

            } else if (*curr == 's') {      // STRING ARGUMENT
                str_arg = va_arg(args, char*);

            } else if (*curr == 'd') {      // DECIMAL ARGUMENT
                const char *number_string = decimal_string(va_arg(args, int));
                memcpy(temp_buf, number_string, strlen(number_string) + 1);
                str_arg = temp_buf;

            } else if (*curr == 'x') {      // HEX ARGUMENT
                const char *number_string = hex_string(va_arg(args, unsigned int));
                memcpy(temp_buf, number_string, strlen(number_string) + 1);
                str_arg = temp_buf;
                buf_char = '0';
                
            } else if (*curr == 'l' && *(curr+1) == 'd') {      // LONG DECIMAL ARGUMENT
                const char *number_string = decimal_string(va_arg(args, long));
                memcpy(temp_buf, number_string, strlen(number_string) + 1);
                str_arg = temp_buf;
                curr++;

            } else if (*curr == 'l' && *(curr+1) == 'x') {      // LONG HEX ARGUMENT
                const char *number_string = hex_string(va_arg(args, unsigned long));
                memcpy(temp_buf, number_string, strlen(number_string) + 1);
                str_arg = temp_buf;
                buf_char = '0';
                curr++;

            } else if (*curr == 'p') {      // POINTER ARGUMENT
                const char *number_string = hex_string(va_arg(args, unsigned long));
                memcpy(temp_buf, number_string, strlen(number_string) + 1);
                str_arg = temp_buf;
                buf_char = '0';
                length += strlcat(buf, "0x", bufsize) - buf_len;

                if (pad_size == 0) {
                    pad_size = 8;
                } 

            }

            // APPEND THE PADDING AND THE NEW STRING
            length += pad(buf, bufsize, pad_size, strlen(str_arg), buf_char);
            buf_len = strlen(buf);
            length += strlcat(buf, str_arg, bufsize) - buf_len;

            // MOVE THE CURR POINTER FORWARD
            curr++;

        // IF THERE WAS NO ARGUMENT, ADD THE NEXT CHARACTER TO THE BUFFER
        } else {
            char letter[2];
            letter[0] = *curr;
            letter[1] = '\0';
            length += strlcat(buf, letter, bufsize) - buf_len;
            curr++;
        }
    }
    return length;
}


/*
 * This function serves as a publicly usable version of printf that allows a user-defined
 * buffer and buffer length to be inputted for later use. 
 */
int snprintf(char *buf, size_t bufsize, const char *format, ...) {
    va_list(args);
    va_start(args, format);
    int length = vsnprintf(buf, bufsize, format, args);
    va_end(args);
    return length;  
}


// ASSUME THE OUTPUT IS NEVER LONGER THAN 1024
#define MAX_OUTPUT_LEN 1024


/*
 * This function serves as a publicly usable version of printf, only allowing the format
 * strings and all arguments to be inputted and using uart to print the string. 
 */
int printf(const char *format, ...) {
    char array[MAX_OUTPUT_LEN];
    va_list(args);
    va_start(args, format);
    int length = vsnprintf(array, MAX_OUTPUT_LEN, format, args);
    va_end(args);

    uart_putstring(array);

    return length;
}


/* From here to end of file is some sample code and suggested approach
 * for those of you doing the disassemble extension. Otherwise, ignore!
 *
 * The struct insn bitfield is declared using exact same layout as bits are organized in
 * the encoded instruction. Accessing struct.field will extract just the bits
 * apportioned to that field. If you look at the assembly the compiler generates
 * to access a bitfield, you will see it simply masks/shifts for you. Neat!
 */
/*
static const char *reg_names[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
                                    "s0/fp", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
                                    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
                                    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6" };

struct insn  {
    uint32_t opcode: 7;
    uint32_t reg_d:  5;
    uint32_t funct3: 3;
    uint32_t reg_s1: 5;
    uint32_t reg_s2: 5;
    uint32_t funct7: 7;
};

void sample_use(unsigned int *addr) {
    struct insn in = *(struct insn *)addr;
    printf("opcode is 0x%x, reg_dst is %s\n", in.opcode, reg_names[in.reg_d]);
}
*/
