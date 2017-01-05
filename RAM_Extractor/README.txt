###############################
## CREATED BY: SHIRKAMDEV 	 ##
## DEBUGGING BY: REYESELDA95 ##
###############################

-- RAM EXTRACTOR --


LICENSES
-This program uses a library called STDPijo, under GPL V3.0 license.

DISCLAIMER
To run this program you MUST be ROOT, as it reads from raw device events, to avoid lag.

DESCRIPTION
This program uses STDPijo to show the RAM of the game you are playing with ALE. It reads from keyboard, in one sepparate thread.
You can customize all your controls in the code, as well as the FPS number. 

USAGE
[with ROOT privileges] ./extractRAM <..ROMFILE..>
Also, to run it, you must have in your LD_LIBRARY_PATH the path to libale.so, so it can load it.

COMPILING
First, you must have in your LIBRARY_PATH variable, the path to libale.so, wherever you have compiled it. Once done it, just run make and wait, not much, to the compilation.
