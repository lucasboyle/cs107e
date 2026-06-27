/* Lucas Boyle
 * CS 107E
 * 1/19/26
 *
 * File: larson.s
 * --------------
 * This file contains the assembly code to create a Larson Scanner using eight
 * LEDs (ability to change number of LEDs noted in comments below). PB0-7 are 
 * configured as the outputs for the LEDs. 
 * 
 * Register a1 is a maximum of eight bits, with only a single bit being set to 
 * a value of 1 at a given time. During each iteration of the loop, the bit 
 * is shifted left or right to change the LED being lit. 
 */


lui         a0,0x2000                   # a0 holds base addr PB group = 0x2000000
addi        a1,zero,1                   # a1 used to set LED on/off, holds 1 to start 

li          a2,0x11111111               # a2 holds 8 bits of 1 (change # of bits to # of LEDs)
sw          a2,0x30(a0)                 # config PB0-PB7  as output

addi        a3,zero,0x8                 # a3 used to count LED on/off actions remaining in the
                                        # movement, holds 8 to start

addi        a4,zero,0                   # a4 holds movement:  0 = move right, 1 = move left


loop:
    sw	    a1,0x40(a0)                 # turn on the current LED, based on a1


    lui     a5,5500                     # a5 = init countdown value
delay:
    addi    a5,a5,-1                    # decrement a5 by 1
    bne     a5,zero,delay               # keep counting down until a5 is zero

    addi    a3,a3,-1                    # now that delay is over, decrement a3 by 1


move:
    beq     a3,zero,change_direction    # if the end of the row has been reached, change directions
    beq     a4,zero,move_right		    # if a4 holds 0, move right
    j       move_left                   # else, move left


change_direction:
    addi    a3,zero,0x7                 # reset a3 to hold 7 (one less than original value
                                        # since the end light is already lit)

    xori    a4,a4,1                     # alternate a4 (the movement direction)
    j move                              # jump to move


move_right:
    slli     a1,a1,1                    # shift a1 left by 1 bit (move right by one LED)
    j        loop                       # jump to the loop


move_left:
    srli     a1,a1,1                    # shift a1 right by 1 bit (move left by one LED)
    j        loop                       # jump to the loop
