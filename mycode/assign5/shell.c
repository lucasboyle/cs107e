/* Lucas Boyle
 * CS 107e
 * 2/19/26
 * 
 * File: shell.c
 * -------------
 * This file implements shell.h, incorporating functionality for creating
 * a 107e that has custom commands, recieved from a PS2 keyboard. 
 */


#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "malloc.h"
#include "strings.h"
#include "mango.h"
#include "ps2_keys.h"
#include "keyboard.h"


// DEFINE THE LINE LENGTH
#define LINE_LEN 80


// MODULE LEVEL GLOBAL VARIABLES FOR SHELL
static struct {
    input_fn_t shell_read;
    formatted_fn_t shell_printf;
} module;


// CONSTANT TO HOLD THE NUMBER OF COMMANDS CREATED
static const int NUM_COMMANDS = 9;


// ADDITIONAL COMMAND FUNCTIONS FOR EXTENSION
static int cmd_history(int argc, const char *argv[]);
static int cmd_repeat_last(int argc, const char *argv[]);
static int cmd_man(int argc, const char *argv[]);


// CONSTANT ARRAY OF COMMANDS; HOLDS USAGE, DESCRIPTION, AND FUNCTION
static const command_t commands[] = {
    {"help",   "help [cmd]",        "print command usage and description", cmd_help},
    {"echo",   "echo [args]",       "print arguments", cmd_echo},
    {"clear",  "clear",             "clear screen (if your terminal supports it)", cmd_clear},
    {"reboot", "reboot",            "reboot the Mango Pi", cmd_reboot},
    {"peek",   "peek [addr]",       "print contents of memory at address", cmd_peek},
    {"poke",   "poke [addr] [val]", "store value into memory at address", cmd_poke},
    {"history","history",           "display history of 10 most recent commands", cmd_history},
    {"man",    "man [cmd]",         "displays the manual for a command", cmd_man},
    {"!!",     "!!",                "repeat last command", cmd_repeat_last},
};


// GLOBAL VARIABLES TO HOLD THE COMMAND HISTORY, SIZE, AND CURRENT VALUE
static char *command_history[10];
static int history_size = 0;
static int curr_command = -1;

/*
 * This function implements the echo command for the shell. 
 */
int cmd_echo(int argc, const char *argv[]) {
    for (int i = 1; i < argc; ++i)
        module.shell_printf("%s ", argv[i]);
    module.shell_printf("\n");
    return 0;
}


/*
 * This function implements the help command for the shell. 
 */
int cmd_help(int argc, const char *argv[]) {
    
    // PRINT ALL COMMANDS IF NO ARGUMENTS PASSED IN
    if (argc == 1) {
        for (int i=0; i<NUM_COMMANDS; i++) {
            module.shell_printf("%s\t\t%s\n", commands[i].usage, commands[i].description);
        }
        return 0;

    // PRINT INFO FOR THE COMMAND PASSED IN
    } else {
        for (int i=0; i<NUM_COMMANDS; i++) {
            if (strcmp(argv[1], commands[i].name) == 0){
                module.shell_printf("%s\t\t%s\n", commands[i].usage, commands[i].description);
                return 0;
            }
        }
    }

    // CALL AN ERROR IF NO COMMAND FOUND
    module.shell_printf("error: no such command '%s'\n", argv[1]);
    return -1;
}


/*
 * This function implements the clear command for the shell.
 */
int cmd_clear(int argc, const char* argv[]) {
    module.shell_printf("\f");
    return 0;
}


/*
 * This function implements the reboot command for the shell. 
 */
int cmd_reboot(int argc, const char* argv[]) {
    module.shell_printf("Rebooting...");

    for (int i=0; i<history_size; i++) {
        free(command_history[i]);
    }

    mango_reboot();
    return -1;
}


/*
 * This function implements the peek command for the shell. 
 */
int cmd_peek(int argc, const char* argv[]) {
    
    // THROW AN ERROR IF NO ARGUMENTS ARE PASSED IN
    if (argc == 1) {
        module.shell_printf("error: peek expects 1 argument [addr]\n");
        return -1;
    }
    
    // CONVERT THE ARGUMENT TO A NUMBER
    const char *end;
    unsigned long num = strtonum(argv[1], &end);

    // ERROR CHECKING
    if (num == 0 && strcmp(argv[1], "0") != 0) {
        module.shell_printf("error: peek cannot convert '%s'\n", argv[1]);
        return -1;
    } else if (end != (argv[1] + strlen(argv[1]))) {
        module.shell_printf("error: peek cannot convert '%s'\n", argv[1]);
        return -1;
    } else if (num % 4 != 0) {
        module.shell_printf("error: peek address must be 4-byte aligned\n");
        return -1;
    }

    // ACCESS DATA AT ADDRESS, PRINT IT OUT
    module.shell_printf("%s:\t%8x\n", argv[1], *(unsigned int *)num);
    return 0;
}


/*
 * This function implements the poke command for the shell. 
 */
int cmd_poke(int argc, const char* argv[]) {
    
    // MAKE SURE THERE ARE 2 ARGUMENTS
    if (argc < 3) {
        module.shell_printf("error: poke expects 2 arguments [addr] and [val]\n");
        return -1;
    }

    // CONVERT ADDRESS AND VALUE TO NUMBERS
    const char *addr_end;
    unsigned long addr = strtonum(argv[1], &addr_end);
    const char *val_end;
    unsigned long val = strtonum(argv[2], &val_end);

    // ERROR CHECKING
    if (addr == 0 && strcmp(argv[1], "0") != 0) {
        module.shell_printf("error: poke cannot convert '%s'\n", argv[1]);
        return -1;
    } else if (addr_end != (argv[1] + strlen(argv[1]))) {
        module.shell_printf("error: poke cannot convert '%s'\n", argv[1]);
        return -1;
    } else if (val_end != (argv[2] + strlen(argv[2]))) {
        module.shell_printf("error: poke cannot convert '%s'\n", argv[1]);
        return -1;
    } else if (val == 0 && strcmp(argv[2], "0") != 0) {
        module.shell_printf("error: poke cannot convert '%s'\n", argv[2]);
        return -1;
    } else if (addr % 4 != 0) {
        module.shell_printf("error: poke address must be 4-byte aligned\n");
        return -1;
    }

    // STORE NEW VALUE
    *(unsigned int *)addr = val;
    return 0;
}


/*
 * This function implements the history command for the shell. 
 */
int cmd_history(int argc, const char* argv[]) {
    
    // LOOP THROUGH COMMAND HISTORY, PRINTING OUT EACH COMMAND STORED
    for (int i=0; i<history_size; i++) {
        module.shell_printf("%d %s\n", i+1, command_history[i]);
    }
    return 0;
}


/*
 * This function implements the !! command for the shell. 
 */
static int cmd_repeat_last(int argc, const char *argv[]) {
    
    // ENSURE HISTORY ISN'T THE ONLY COMMAND THAT HAS BEEN STORED
    if (history_size <= 1) {
        module.shell_printf("error: no command to repeat\n");
        return -1;
    }

    // CALL LAST NON-HISTORY COMMAND
    return shell_evaluate(command_history[history_size-2]);
}


/*
 * This function implements the man command for the shell. 
 */
static int cmd_man(int argc, const char *argv[]) {

    // CALL AN ERROR IF ASCII IS NOT THE ARGUMENT GIVEN
    if (argc == 1) {
        module.shell_printf("error: man expects one argumente [cmd]\n");
        return -1;
    } else if (strcmp(argv[1], "ascii") != 0) {
        module.shell_printf("error: command not supported\n");
        return -1;
    }

    // PRINT DECIMAL ASCII SET
    module.shell_printf("\nDecimal Set:\n");
    for (int i='!'; i<='~'; i++) {
        module.shell_printf("%3d %c  ", i, i);

        if ((i-'!'+1)%8 == 0) {
            module.shell_printf("\n");
        }
    }
    module.shell_printf("\n\n");

    // PRINT HEX ASCII SET
    module.shell_printf("Hexadecimal Set:\n");
    for (int i='!'; i<='~'; i++) {
        module.shell_printf(" %2x %c  ", i, i);

        if ((i-'!'+1)%8 == 0) {
            module.shell_printf("\n");
        }
    }
    module.shell_printf("\n\n");

    return 0;
}


/*
 * This function initializes the shell, storing the read and print
 * functions. 
 */
void shell_init(input_fn_t read_fn, formatted_fn_t print_fn) {
    module.shell_read = read_fn;
    module.shell_printf = print_fn;
}


/*
 * This function uses uart to play an audio/visual cue to be used
 * during error checks for user input. 
 */
void shell_bell(void) {
    uart_putchar('\a');
}


/*
 * This function inserts a char at a specific index in a string. It is assumed
 * that there is room for this new character. 
 */
static void insert_char(char buf[], char ch, size_t index, size_t length) {
    for (int i = length; i > index; i--) {
        buf[i] = buf[i-1];
    }
    buf[index] = ch;

    buf[length + 1] = '\0';
}


/*
 * This funciton removes a char at a specific index in a string.
 */
static void remove_char(char buf[], size_t index, size_t length) {
    for (int i = index; i < length - 1; i++) {
        buf[i] = buf[i+1];
    }
    buf[length-1] = '\0';
}


/*
 * This function clears the entire shell line, and prints the contents of the buffer, ending
 * with the cursor in the correct place. 
 */
static void big_print(char buf[], size_t old_cursor, size_t cursor, size_t old_length, size_t length) {
    char back_a[old_cursor + 1];
    memset(back_a, '\b', old_cursor);
    back_a[old_cursor] = '\0';
    
    char space[old_length + 1];
    memset(space, ' ', old_length);
    space[old_length] = '\0';

    char back_b[old_length + 1];
    memset(back_b, '\b', old_length);
    back_b[old_length] = '\0';

    char back_c[length - cursor + 1];
    memset(back_c, '\b', length - cursor);
    back_c[length - cursor] = '\0';

    module.shell_printf("%s%s%s%s%s", back_a, space, back_b, buf, back_c);
}


/*
 * This function reads a single line of input from the user in the shell,
 * storing it in the buffer. The line will end once the user enters a new 
 * line. 
 */
void shell_readline(char buf[], size_t bufsize) {
    
    // INITIALIZE THE BUFFER AS AN EMPTY STRING
    memset(buf, '\0', bufsize);

    // INITIALIZE LENGTH, CURSOR, AND GET FIRST CHARACTER
    size_t length = 0;
    size_t cursor = 0;
    size_t old_cursor = 0;
    size_t old_length = 0;
    char ch = module.shell_read();
    

    // LOOP WHILE USER DOES NOT ENTER NEW LINE
    while (ch != '\n') {

        // STORE THE OLD INFORMATION
        old_cursor = cursor;
        old_length = length;
        
        // IF THE USER WANTS TO BACKSPACE, CHECK FOR ERRORS AND BACKSPACE
        if (ch == '\b') {
            if (length > 0 && cursor > 0) {
                remove_char(buf, cursor-1, length);
                length--;
                cursor--;
            } else {
                shell_bell();
            }

        // IF THE USER SELECTS CTRL-A, SET THE CURSOR TO THE FRONT
        } else if (ch == ('a' - 'a' + 1)) {
            cursor = 0;

        // IF THE USER SELECTS CTRL-E, SET THE CURSOR TO THE END
        } else if (ch == ('e' - 'a' + 1)) {
            cursor = length;

        // IF THE USER SELECTS CTRL-D, FORWARD DELETE CHARACTER IF ABLE
        } else if (ch == ('d' - 'a' + 1)) {
            if (cursor == length) {
                shell_bell();
            } else {
                remove_char(buf, cursor, length);
                length--;   
            }

        // IF THE USER SELECTS THE LEFT ARROW, SHIFT THE CURSOR LEFT IF ABLE
        } else if (ch == PS2_KEY_ARROW_LEFT) {
            if (cursor > 0) {
                cursor--;
            } else {
                shell_bell();
            }

        // IF THE USER SELECTS THE RIGHT ARROW, SHIFT THE CURSOR RIGHT IF ABLE
        } else if (ch == PS2_KEY_ARROW_RIGHT) {
            if (cursor < length) {
                cursor++;
            } else {
                shell_bell();
            }

        // IF THE USER SELECTS THE UP ARROW, DISPLAY THE PREVIOUS COMMAND IF ABLE
        } else if (ch == PS2_KEY_ARROW_UP) {
            if (curr_command >= 0) {
                size_t command_len = strlen(command_history[curr_command]);
                memcpy(buf, command_history[curr_command], command_len+1);
                cursor = command_len;
                length = command_len;
                curr_command--;
            } else {
                shell_bell();
            }

        // IF THE USER SELECTS THE DOWN ARROW, DISPLAY THE NEXT COMMAND IF ABLE
        } else if (ch == PS2_KEY_ARROW_DOWN) {
            if (curr_command < history_size - 1) {
                curr_command++;
                if (curr_command < history_size - 1) {
                    size_t command_len = strlen(command_history[curr_command + 1]);
                    memcpy(buf, command_history[curr_command + 1], command_len+1);
                    cursor = command_len;
                    length = command_len;
                } else {
                    memset(buf, '\0', bufsize);
                    cursor = 0;
                    length = 0;
                }
            } else {
                shell_bell();
            }

        // IF MAX LENGTH IS REACHED, CALL shell_bell()
        } else if (length == (bufsize-1)) {
            shell_bell();

        // STORE CHARACTER NORMALLY IF IT FITS
        } else if (ch <= 0x7f) {
            insert_char(buf, ch, cursor, length);
            length++;
            cursor++;
        }

        // PRINT OUTPUT TO THE SHELL
        big_print(buf, old_cursor, cursor, old_length, length);

        // GET NEXT CHARACTER
        ch = module.shell_read();
    }

    // GO TO NEW LINE, END THE BUFFER WITH A NULL TERMINATOR
    module.shell_printf("\n");
    buf[length] = '\0';
}


/*
 * This function returns a pointer to a new null-terminated string 
 * containing at most n bytes copied from the string pointed to by src.
 */
static char *strndup(const char *src, size_t n) {
    char *result = malloc(n+1);
    memcpy(result, src, n);
    result[n] = '\0';
    return result;
}


/*
 * This function returns a bool determining if a char is a space character.
 */
static bool isspace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n';
}


/*
 * This function will tokenize a string, splitting it by the spacing between
 * the characters and ignorning all white space. At most, max tokens will be 
 * written to the array. 
 */
static int tokenize(const char *line, const char *array[], int max) {
    int ntokens = 0;
    const char *cur = line;

    while (ntokens < max) {

        // SKIP SPACES
        while (isspace(*cur)) cur++;

        // END WHEN THERE ARE NO MORE NON-SPACE CHARS
        if (*cur == '\0') break;
        const char *start = cur;

        // ADVANCE TO THE END, ADDING TO ARRAY AFTER EACH SPACE
        while (*cur != '\0' && !isspace(*cur)) cur++;
        array[ntokens++] = strndup(start, cur - start); 
    }
    return ntokens;
}


/*
 * This function takes in a line written by the user in the shell and
 * evaluates/calls the correct command function for the shell. 
 */
int shell_evaluate(const char *line) {
    
    // TOKENIZE THE LINE
    int max_len = strlen(line);
    const char *tokens[max_len];
    int num_tokens = tokenize(line, tokens, max_len);

    // IF THERE ARE NO TOKENS, RETURN -1
    if (num_tokens == 0) {
        return -1;
    }

    // IF THERE IS STILL ROOM IN THE HISTORY, ADD THE LINE TO THE ARRAY
    if (history_size < 10) {
        char *line_copy = malloc(strlen(line) + 1);
        memcpy(line_copy, line, strlen(line) + 1);
        command_history[history_size] = line_copy;
        history_size++;
        curr_command++;

    // ELSE, FREE THE EARLIEST COMMAND CALLED, SHIFT THE CONTENTS, AND INSERT
    // NEW CALL AT THE END
    } else {
        free(command_history[0]);
        for (int j=0; j<9; j++) {
            command_history[j] = command_history[j+1];
        }
        char *line_copy = malloc(strlen(line) + 1);
        memcpy(line_copy, line, strlen(line) + 1);
        command_history[9] = line_copy;
    }

    // LOOP THROUGH EACH COMMAND, CALL THE MATCHING ONE
    for (int i=0; i<NUM_COMMANDS; i++) {
        if (strcmp(tokens[0], commands[i].name) == 0) {        
            curr_command = history_size - 1;
            return commands[i].fn(num_tokens, tokens);
        }
    }

    // IF NO COMMANDS WERE FOUND, CALL AN ERROR
    module.shell_printf("error: no such command '%s'\n", tokens[0]);
    return -1;
}


/*
 * This function runs the shell, asking the user to continuously input a line
 * and then executing it. 
 */
void shell_run(void) {
    module.shell_printf("Welcome to the CS107E shell.\nRemember to type on your PS/2 keyboard!\n");
    while (1) {
        char line[LINE_LEN];

        module.shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}
