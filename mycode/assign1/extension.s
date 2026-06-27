/* Lucas Boyle
 * CS 107E
 * 1/19/26
 *
 * File: extension.s
 * --------------
 * This program fulfills the extension for the Larson Scanner. 8 LEDs are configured in PB0-PB7 as 
 * output and a button is configured in PC0 as input. Just like the core assignment, a1 holds a 
 * single 1 bit representing the location of the middle LED in the scanner. Bitshifting and 
 * other bitwise operations are then used to determine the sequence of bits to represent 
 * combinations of the outer, inner, and center LED. 
 * 
 * To get different levels of brightness, a few steps occur:
 *      1. All 5 LEDs are turned on.
 *      2. The outer LEDs are turned off. 
 *      3. The inner LEDs are turned off.
 *      4. Repeat.
 *
 * In between each of these steps, a small break occurs to allow for the brightness differnce to
 * be seen. Due to the above steps, the center LED will be on for 3x as long as the outer LEDs and
 * 1.5x as long as the inner LEDs, which represents the difference in brightness. 
 */


lui         a0,0x2000                  # a0 holds base addr PB group = 0x2000000

addi        a1,zero,1                  # a1 holds middle LED (starts as 1)

li          a2,0x11111111              # a2 holds 8 bits of 1 (change # of bits to # of LEDs)
sw          a2,0x30(a0)                # config PB0-PB7  as output

sw          zero,0x60(a0)              # config PC0 as input

addi        a3,zero,0x8                # a3 used to count LED on/off actions remaining in the
                                       # movement, holds 8 to start

addi        a4,zero,0                  # a4 holds movement:  0 = move right, 1 = move left


loop:
    slli    s0,a1,1
    srli    s1,a1,1
    or      s0,s0,s1                   # s0 holds inner LEDs

    slli    s1,a1,2
    srli    s2,a1,2
    or      s1,s1,s2                   # s1 holds outer LEDs

    or      s2,a1,s0
    or      s2,s2,s1                   # s2 holds all LEDs
  
    or      s3,a1,s0                   # s3 holds middle and inner LEDs


    lui     a5,30                      # initialize a5 as a countdown
delay:
    addi    a5,a5,-1                   # decrement a5 by 1


    sw      s2,0x40(a0)                # turn on all LEDs
    call    dimming_wait               # NOTICE: call is used so we return back here
    sw      s3,0x40(a0)                # turn off outer LEDs
    call    dimming_wait
    sw      a1,0x40(a0)                # turn off inner LEDs
    call    dimming_wait 

                                        
    bge     a5,zero,delay               # keep counting down until a5 is zero
    
    lw      s4,0x70(a0)                 # load value from PC0 into s4
    andi    s4,s4,1                     # s4 holds zero if the button is pressed
    beq     s4,zero,delay               # if the button if pressed, loop back to the delay

    addi    a3,a3,-1                    # now that delay is over, decrement a3 by 1


move:
    blt     a3,zero,change_direction    # if the end of the row has been reached, change directions
    beq     a4,zero,move_right          # if a4 holds 0, move right
    j       move_left                   # else, move left


change_direction:   
    addi    a3,zero,0x7                 # reset a3 to hold 7 (one less than original value
                                        # since the end light is already lit)

    xori    a4,a4,1                     # alternate a4 (the movement direction)
    j move                              # jump to move


move_right:
    slli    a1,a1,1                     # shift a1 left by 1 bit (move right by one LED)
    j       loop                        # jump to the loop


move_left:
    srli    a1,a1,1                     # shift a1 right by 1 bit (move left by one LED)
    j       loop                        # jump to the loop


dimming_wait:
    addi    a7,zero,0xf                 # inititalize countdown to hold 0xf
countdown:
    addi    a7,a7,-1                    # decrement countdown
    bne     a7,zero,countdown           # loop until zero is reached
    ret                                 # return to where this was called

