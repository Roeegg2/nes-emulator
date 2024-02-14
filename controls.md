# PPU Controls

These are pretty straight forward.

# for myself

NES has 2 controller ports, and a rarely used 48 pin expansion port underneath

**NES IO ports**
1 output port, 3 bits, accesible by writing the bottom 3 bits of 4016

2 input ports, each is 5 bits wide. accesible by reading the bottom 5 bits of 4016 and 4017
reading these 2 ports activate pins /OE1 and /OE2

**the standard NES controller dataline**

```
0 - A
1 - B
2 - Select
3 - Start
4 - Up
5 - Down
6 - Left
7 - Right
```

after ? reads all the 8 bits, all the bits will turn into 1. (3rd party and other controllers might report differently here)

**input (4016 write)**

while bit 0 (S) (strobe) is 1, the controller shift regs are reloaded from the button states. reading 4016/4017 will return the current state of the first button A.
when bit 0 (S) turns 0, reloading will be stopped

**output 4016/4017**
in order to get the status of controller 1 and controller 2, you must write to 4016 and then read 4016 (for controller 1) and read 4017 (for controller 2)

first 8 reads will indicate which buttons/directions are pressed (1 for pressed, 0 for not pressed). all other reads will return 1 on the official NES controllers, but other 3rd party controllers report 0.

the status for each controller is return as 1 byte (8 bits), in the following order: A,B,Select,Start,Up,Down,Left,Right

when no controller is connected, all bits return 0.

---------------------------------------

Write 1 to $4016 to signal the controller to poll its input
Write 0 to $4016 to finish the poll
Read polled data one bit at a time from $4016 or $4017
