## Title of your project
Rubik's Cube Solver

## Team members
Daniel Casas and Lucas Boyle

## Project goals
Our goal is to build a machine that is capable of turning each face of a Rubiks cube
in order to solve it. In order for the computer to know how the cube is scrambled, the 
user will have to identify the color of each square. This will be done on a monitor and 
requires the use of a mouse to select each square and its color. Using this information, 
the computer will identify the steps required to solve the cube, physically turning each 
face of the cube to solve it. 

Stretch Goal: If we have time, we will also display the cube being solved in a 3D graphical 
representation on the monitor. 

Fallback: No algorithm, preset patterns to solve. 

## Resources, budget
Stepper Motors (x6) - lab64 or Amazon
Motor Controllers (x6) - lab64 or Amazon
Rubik's Cube - Amazon
Acrylic and PLA for mechanism - PRL

We understand that this will probably be over budget and are fine with it. 

## Tasks, member responsibilities
Lucas - CAD and manufacturing for mechanism

Daniel - Circuit/wiring for motors

Both - UI/console interface and algorithm

## Schedule, midpoint milestones
### Hardware:
Order parts (3/4)
Test control of a single motor (3/7)
Design the physical mechanism (3/8)
Build the physical mechanism (3/11)
Test with pre-defined moves (3/13)
Find the fastest possible move speed (3/13)

### Software:
Decide how the cube will be represented in code (3/3)
Design a graphics interface for selecting a scrambled pattern with a mouse (3/12)
Design the UI for multiple modes (3/14)
Make an algorithm for solving a cube (3/15)
Test the algorithm by hand solving (3/15)
Incorporate 3D graphics or some other peripherals (3/19)

### Integration: 
Test full system (3/17)
Make presentation (3/19)

## Additional resources, issues
Most of our risks are associated with the mechanical components. Possible issues
include tolerances, strength, and stability. We also need to get a solid mouse
driver created for this project. 