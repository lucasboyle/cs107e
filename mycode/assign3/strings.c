/* Lucas Boyle
 * 2/3/26
 * CS 107e
 * 
 * File: strings.c
 * ---------------
 * This file implements the strings.h header, incorporating functionality for copying
 * strings, setting values, determining the length of a string, comparing strings,
 * concatenating strings, and converting strings to numbers. 
 */

#include "strings.h"

/* 
 * This function copies the contents from a source to a destination one byte at a time, 
 * copying over n bytes.  
 */
void *memcpy(void *dst, const void *src, size_t n) {
    char *d = dst;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}


/*
 * This function sets n bytes of the destination to the value, getting only the least 
 * significant byte if necessary. 
 */
void *memset(void *dst, int val, size_t n) {
    char *d = dst;

    // GET LEAST SIGNIFICANT BYTE (8 BITS)
    unsigned char lsb = val & 0xFF;

    // SET DESTINATION
    while (n--) {
        *d++ = lsb;
    }
    return dst;
}


/*
 * This function determines the length of a string, looping through the characters until the 
 * null terminator is reached. 
 */
size_t strlen(const char *str) {
    size_t n = 0;
    while (str[n] != '\0') {
        n++;
    }
    return n;
}


/*
 * This function compares two strings lexicographically. If s1 is greater than s2, a positive
 * number will be retured. If s1 is less than s2, a negative number will be returned. If the 
 * strings are equal, 0 will be returned. 
 */
int strcmp(const char *s1, const char *s2) {
    size_t n = 0;

    // LOOK FOR DIFFERENCES IN STRINGS
    while (s1[n] != '\0' && s2[n] != '\0') {
        if (s1[n] != s2[n]) {
            return (int)(s1[n] - s2[n]);
        }
        n++;
    }

    // IF THE LENGTHS ARE EQUAL, THE STRINGS MUST BE THE SAME
    if (strlen(s1) == strlen(s2)) {
        return 0;
    }

    // RETURN THE DIFFERENCE IN LENGTHS
    return strlen(s1) - strlen(s2);
}


/*
 * This function will concatenate two strings within a specified destination size, reducing
 * the length if needed. If the entire source string can be concatenated and fit within
 * the size, the length of this new string will be returned. If not, the length of the string
 * assuming adequate size will be returned. 
 */
size_t strlcat(char *dst, const char *src, size_t dstsize) {
    
    // IF THE DESTINATION IS ALREADY LARGER THAN ALLOWED, RETURN IDEALIZED LENGTH
    if (strlen(dst) >= dstsize) {
        return dstsize + strlen(src);
    }

    // DETERMINE LENGTH TO RETURN
    size_t length = strlen(dst) + strlen(src);

    // COPY THE STRING OVER, LEAVING THE LAST CHARACTER FOR THE NULL TERMINATOR
    memcpy(dst+(strlen(dst)), src, dstsize - strlen(dst) - 1);
    memcpy(dst+(dstsize-1), "\0", 1);
    return length;
}


/*
 * This function will convert a string to a number. It will work for both decimal and 
 * hexadecimal numbers. If an endptr is passed in, it will be set to the location of the 
 * next non-number character in the source string. 
 */
unsigned long strtonum(const char *str, const char **endptr) {
    unsigned long total = 0;
    size_t n = 0;
    size_t base = 10;

    // DETERMINE IF BASE 16 IS REQUIRED
    if ((strlen(str) > 2) && (str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X'))) {
        base = 16;
    }

    // MAKE PROPER CONVERSIONS USING ASCII
    if (base == 10) {
        while (str[n] >= '0' && str[n] <= '9') {
            total = total * base + (str[n] - '0');
            n++;
        }

    } else {
        n = 2;
        while ((str[n] >= '0' && str[n] <= '9') || (str[n] >= 'A' && str[n] <= 'F') || 
               (str[n] >= 'a' && str[n] <= 'f')) {
            
            if (str[n] >= '0' && str[n] <= '9') {
                total = total * base + (str[n] - '0');
            } else if (str[n] >= 'A' && str[n] <= 'F') {
                total = total * base + (str[n] - 'A' + 10);
            } else {
                total = total * base + (str[n] - 'a' + 10);
            }
            n++;
        }
    }

    // IF AN endptr WAS PASSED IN, SET IT TO THE LOCATION OF THE NEXT NON-DIGIT
    if(endptr != NULL) {
        *endptr = str + n;
    }
    return total;
}
