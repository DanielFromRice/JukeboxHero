# JukeboxHero
ELEC327 Final Project

Team CLARG:
Mac Carr
Kevin Lata
Michael Angino
Daniel Rothfusz
Elise Gibney

In this project, we are using MSP430G2553 microcontrollers to play a 
version of Guitar Hero.

There are 3 distinct PCBs:
	1. Controller Board
	2. Speaker Boards
	3. Break Beam Boards

1. Controller Board:
This board functions as a central controller module. It facilitates song 
selection, loading data from the SD card, managing a timing module, and 
commmunicating data to the other modules. The SD card uses SPI communication
implementing the PetitFs library (http://elm-chan.org/fsw/ff/00index_p.html)
Song selection is done using 3 buttons, implementing up, down, and select.
Support for a display module is intended for future development.

2. Speaker Boards:
These boards basically consist of a speaker and an MSP430. They wait to 
receive a signal from the controller over I2C, and when they do, they 
change their frequency with the new data they have received.

3. Break Beam Boards: 
This board facilitates communication to the DotStar LED strips and 
to the breakbeam sensors. This board is preloaded with break beam song data
once a song has been selected, and then proceeds to advance the lights and 
track a user's score as it receives notice from the central controller.

Overall, the game should function similarly to Guitar Hero. Lights descend 
along the DotStar strips, signaling which beam break sensor the player should
trip. The order of these loosely correspond to a song that is playing, which
produces multiple tones over a collection of piezzo buzzers. 

To run:
Once boards are wired up with the appropriate hardware, open 3 projects in 
Code Composer Studio. Load all the code from ControllerBoardCode to the 
Controller board, load code from the Break_Beam folder to the Break Beam 
Board, and load the code from SpeakerBoardCode to each speaker board, 
ensuring that each one gets a different I2C address.
