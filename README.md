# DOOM64-RE

Welcome to the complete reverse engineering of Doom 64 by [GEC], this effort took about 1 year and a half although it had not advanced almost nothing, I restarted the whole process from June of this year, theoretically 5 months to complete it from scratch.

## Installation

You need to download and install the N64 SDK https://n64squid.com/homebrew/n64-sdk/ide/
You can also go to this link if it is of your own interest https://n64.dev/

To compile it is required to use Windows xp, you can also use virtual machine for such purposes

Once the N64SDK is installed, copy the "doom64" folder to the root "C:" of your hard drive.

For now it is compatible with the USA version of Doom 64.
Of course, before compiling you need the data of the original Doom64 (DOOM64.WAD | DOOM64.WMD | DOOM64.WSD | DOOM64.WDD).
To obtain them, go to the Tools folder and extract the content of the Doom64Extractor.zip file, "source code included".
Edit the file ExtraerDatos.bat and you change the text "Doom 64 (Usa) .z64" by the name of the rom you have, it is very important that it is in "z64" format later you execute the file ExtraerDatos.bat and copy the extracted data in the folder "Data".
If you can't get the rom in "z64" format there is a file in the Tools folder that is Tool64_v1.11.zip extract them and you can convert the "n64 and v64" formats to "z64".
Finally you run the MAKE_ROM.bat file to compile and obtain a file called doom64.n64

## Notes
The project was created with CodeBlocks, although it does not serve to compile, but to have the code in order and verification.

You can also use the WESSLIB.obj from the original Mortal Kombat Trilogy n64 and not use the code rebuilt by me.
For this go to the file "wessarc.h" and remove the slashes from the text "//#define NOUSEWESSCODE" it should look like this "#define NOUSEWESSCODE".
Then go to the Makefile and remove the "#" from the following line (WESSLIB = #wesslib.obj) it should look like this (WESSLIB = wesslib.obj) and I added the "#" in the following line (ASMFILES = wessint_s.s) it should look like this (ASMFILES = #wessint_s.s) and you proceed to compile the Rom.

Special thanks to my brothers for the help to the community in DoomWorld and Kaiser since he is the only one to see the progress of my work and helps me in several occasions.
GEC Team Discord:  https://discord.gg/aEZD4Y7

## News
* Features: SECURITY KEYS "locked from the original game, give all the keys available in the level"
* Features: WALL BLOCKING "locked from the original game, I consider it as Noclip"
* Features: LOCK MONSTERS "locked from the original game, stops the movement of enemies like Doom 64 Ex"
* Features: MUSIC TEST "locked from the original game, you can play the music available from the game"
* Features: Colors "new feature for me, turn colors on and off making the game experience more interesting"
* Features: FULL BRIGHT "new feature for me, all colors will be completely clear"
* Features: FILTER "new feature for me, activates and deactivates the 3-point filter of the game"
