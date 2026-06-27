## Project title
Rubik's Cube Solver

## Team members
Lucas Boyle and Daniel Casas

## Project description
We built a mechanical Rubik's Cube solver with a user interface that allows a user 
to scramble the cube and enter its starting configuration. The user will use a mouse
to click buttons displayed on the screen that will turn corresponding faces of the cube.
The user can then select the color of each tile on the cube to indicate its starting 
configuration, or they can click the `Autofill` button if the cube started the program
in its solved configuration. 

Using the information inputted on the UI, our solving algorithm will determine a way 
to solve the cube and will then proceed to execute the movements and quickly solve the 
cube. Physically, each face of the cube is connected to a stepper motors which allows 
for quick, precise turns using our stepper motor module. 

FOR USING AGAIN REFER TO THIS DOC: https://docs.google.com/document/d/171OI34wzGzihEIF-X0iULA1a6UNChS4RMk1GUSWYJEs/edit?usp=sharing

## Member contribution
Lucas: designed, manufactured, and built the physical Rubik's Cube Solver mechanism; 
developed the UI; expanded on mouse extension to allow for button clicks and graphics
interactions.

Daniel: designed and implemented electrical system and circuits for motor control;
developed and implemented code for stepper motor modules; researched and integrated 
solving algorithm; worked to debug physical and coding problems; helped with interfacing
with motors in the ui buttons and alignment

## References

MANY SOURCES

Kociemba algorithm overview:
https://kociemba.org/twophase.htm

Two-phase algorithm details:
https://kociemba.org/math/imptwophase.htm

Pruning table notes:
https://kociemba.org/math/pruning.htm

Rubik’s Cube optimal solutions with pattern databases (Korf 1997 PDF):
https://www.cs.princeton.edu/courses/archive/fall06/cos402/papers/korfrubik.pdf

kociemba (C+Python ports) README with cube-string order and example:
https://github.com/muodov/kociemba

Assisted by AI for a lot of synthesizing materials for the alogrithm and porting
and understanding the algorithm in general

Much of my own contribution was built on top of these other sources through debugging
and working with all these sources to create a working algorithm in c that also works
on our specific architecture

EVERYTHING ELSE:
https://www.pololu.com/product/2133


## Self-evaluation
We were able to execute our plan very well, hitting all of our goals and even getting a 
working algorithm, something that we thought might be a stretch at first. In the end, 
we think that we created a very robust and elegant project that showcases a lot of what
we learning in this class. It is so satisfying to see the system solve the cube as 
quickly as it does. 

Some of the most trying moments that we faced involved getting the stepper motors and 
their electrical systems to work well (and not smoke!). Within this there were many optimization,
calibration, and adjustment steps to bring the motors to a working state. Another important issue
when it came to power supply issues in which we were supplying power directly from the wall at some
point overloading some motors and motor controllers due to faulty capacitors and an interesting issue
with enabling and disabling the motors. There were also countless issues
getting the mouse to behave properly with the UI and getting each of the buttons to properly
react. Finally, porting the algorithm over to C in a way that works on our bare-metal system
was a challenge all on its own. But in the end, we mastered these challenges!

Some things that we learned included creating our own modules and header files, interfacing
with more hardware (motors specifically!) and the mechanical/electrical systems involved, 
learning real world constraints for electrical systems, creating full circuit designs from data sheets 
working with the PS2 mouse and its implications on the graphics library, and general
programming style and techniques that allowed us to be effective partners. 

We are particularly proud of how polished our final product looks, from the mechanical 
system, to the wiring, to the UI. Combined with our algorithm, this has turned into
a project that is not only cool to look at and interact with, but also extremely
effective. We are also super proud of how refined we got our stepper motor module to work
and how quick we were able to make them and we really love the robustness and execution
of this project.

## Photos
Photos and videos are uploaded to this link: 
https://drive.google.com/drive/folders/1qCdV7qVXTm3deZmZjIyUzwLP7rW20aY6?usp=sharing. 
Check them out for videos of the project in action and seeing our progress along the way. 