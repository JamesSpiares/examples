examples/propeller
==================

An example program which runs on Parallax's Propeller Professional Development Board.

To run the emulation, follow these steps:
1.   Make sure that “Parallax Serial Terminal.spin” is in the same directory as “project.spin”.
2.   Open “project.spin” in Parallax’s Propeller Tool.
3.   Set the constants in the second section of “project.spin” to the desired values.
4.   Ensure that all connections listed in the first section of “project.spin” are made on the Propeller board.
5.   Ensure that the Propeller board is on and connected via USB.
6.   Press F11 in Propeller Tool to run the program.
7.   Press F12 in Propeller Tool to open the Parallax Serial Terminal.
8.   Ensure that the baud rate in the Parallax Serial Terminal matches that in “project.spin” and click “Enable”.
9.   Read the prompts and enter an integer followed by the “Return” key after each.
10.  Repeat step 8 until the program has completed.

To comprehend the emulation, understand the following notes:
• The signals sent to each machine are represented using the onboard LEDs.
• Each machine is represented by a group of three LEDs.
   o The top LED represents the rolling mechanism.
   o The middle LED represents the cutting mechanism.
   o The bottom LED represents the roll-replacement mechanism.
• When the program is complete, each machine will report:
   o The number of times the roll of fabric was replaced
   o The length of time for which the machine ran (in seconds)
   o The number of cuts which the machine made
   o The length of fabric which was wasted (in meters) 
