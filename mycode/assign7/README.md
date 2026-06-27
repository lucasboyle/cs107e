To primarily test my interrupts, I ran the console application and used my 
`man ascii` command. Since this takes longer to run, you are able to easily 
type more things onto the keyboard and watch them continue to be typed after
the command was run. 

For the extension, I implemented mouse.c and added ps2_write(). I have provided 
a test that prints out each attribute of a single mouse event, but check out the
paint application for a cool use case! In this application, the mouse will start
in the center of the screen (indicated by the black square cursor). You will also
see the current paint color in the upper left corner. To paint, simply hold down  
the left mouse button and move around! To shuffle through colors, press the right
button. You will see the current color selected in the upper left corner as the
program runs. Finally, hold down the scroll wheel to use the eraser. Enjoy!