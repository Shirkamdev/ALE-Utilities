/*
  This program was indented to help ani ALE developer.

  Copyright (C) 2016 Shirkamdev Xnetrix 

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#include <thread>
#include <stdlib.h>
#include <chrono>
#include <signal.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>
#include <stdio.h>

#include <fstream>

#include <bitset>
#include <iostream>

#include "StdPijo.h"
#include "ale_interface.hpp"

#define ON true
#define OFF false
#define SCREEN ON
#define SOUND OFF
#define SEED 0

#define NUM_BYTES_RAM 128

#define FPS 30

#define N_KEYS 128

/////////////////////
// KEY DEFINITIONS //
/////////////////////

#define KEY_UP 17 //W
#define KEY_DOWN 31 //S
#define KEY_LEFT 30 //A
#define KEY_RIGHT 32 //D
#define KEY_FIRE 57 //SPACE

////////////////////////////
// END OF KEY DEFINITIONS //
////////////////////////////

#define RAM_FILE "ramExtract_"
#define RAM_FILE_BINARY "binRAMExtract_"
#define FILE_TERMINATION ".csv"

#define INPUT_KEYBOARD "/dev/input/event3"

static const char *const evval[3] = {
    "RELEASED",
    "PRESSED ",
    "REPEATED"
};

const int MAX_STEPS = 7500;
int previousRam[NUM_BYTES_RAM];
int previousRAMFrames[NUM_BYTES_RAM];
ALEInterface alei;

bool keys[N_KEYS];
short delayToFPS;

ofstream file, fileBin;

std::thread lector;

void ctrlCHandler(int s){
   lector.detach();
   STDP::Terminar();

   std::cout << "-- PROGRAM FINISHED --\n";
   exit(1);
}

void usage(char* pname) {
   std::cerr
      << "\nUSAGE:\n" 
      << "   " << pname << " <romfile>\n";
   exit(-1);
}

void printRam(){
   ostringstream convert;
   string result;

   clear();
   STDP::PonCursor(0, 0);
   STDP::CambiaColor(STDP_A_NEGRITA, STDP_C_VERDE, STDP_C_NEGRO);
   STDP::EscribeStr("-- RAM LECTOR --");
   STDP::Refrescar();
   STDP::PonCursor(0, 1); //Put cursor on 0, 0 
   STDP::CambiaColor(STDP_A_NORMAL, STDP_C_VERDE, STDP_C_NEGRO);

   //Write super indexes
   STDP::EscribeCh(' ');
   for(int i=0; i<16; ++i) {
      convert.str("");
      convert.clear();
      convert << " "<< setfill('0') << setw(2) << std::hex << i;
      result = convert.str();
      STDP::EscribeStr(result.c_str());
   }

   int pos = 2;
   STDP::PonCursor(0, pos);
   for(int i = 0 ;i < NUM_BYTES_RAM; ++i){
      if(previousRam[i] != -1) {
         if(i % 16 == 0) {
            STDP::PonCursor(0, pos);
            convert.str("");
            convert.clear();
            convert << std::hex << (pos-2) << " ";
            result = convert.str();
            STDP::EscribeStr(result.c_str());
            ++pos;
         }

         if(!(previousRam[i] == alei.getRAM().get(i) && previousRAMFrames[i] < 1)) { //To maintain format
            STDP::CambiaColor(STDP_A_NORMAL, STDP_C_AZUL, STDP_C_NEGRO);
         }

         convert.str("");
         convert.clear();
         convert << setfill('0') << setw(2) << std::hex << (int)alei.getRAM().get(i) << " ";
         result = convert.str();
         STDP::EscribeStr(result.c_str());

         STDP::CambiaColor(STDP_A_NORMAL, STDP_C_VERDE, STDP_C_NEGRO);
      }

      previousRam[i] = alei.getRAM().get(i);
   }

   STDP::Refrescar();
}

void ramToFile() {

    //Printing in HEX (0x)
   for(int i = 0 ;i < NUM_BYTES_RAM; ++i) {
      int valor = alei.getRAM().get(i);

      file << std::hex << valor;
      if(i != NUM_BYTES_RAM-1) {
         file << ",";
      }
      else {
         file << "\n"; //Last value to print (CSV format)
      }
   }

   //Printing in binnary
   for(int i = 0 ;i < NUM_BYTES_RAM; ++i) {
      int valor = alei.getRAM().get(i);

      fileBin << std::bitset<8>(valor);
      if(i != NUM_BYTES_RAM-1) {
         fileBin << ",";
      }
      else {
         fileBin << "\n"; //
      }
   }
}

void keyboardRead() {
   const char *dev = INPUT_KEYBOARD;
    struct input_event ev;
    ssize_t n;
    int fd;

    fd = open(dev, O_RDONLY);
    if (fd == -1) {
        printw("Cannot open keyboard device %s: %s.\n", dev, strerror(errno));
    }

    while (1) {
        n = read(fd, &ev, sizeof ev);
        if (n == (ssize_t)-1) {
            if (errno == EINTR)
                continue;
            else
                break;
        } else
        if (n != sizeof ev) {
            errno = EIO;
            break;
        }

        if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2) {

            //Update key array
            if(ev.value == 0) { //Key released
               keys[ev.code] = false;
            }  
            else if (ev.value == 1)  { //Key pressed
               keys[ev.code] = true;
            }       
        }
    }

    fflush(stdout);
    fprintf(stderr, "%s.\n", strerror(errno));
}


void agentStep(){
   static int wide = 9;
   float reward = 0;
   
   //HERE we read from the key buffer that gives us the thread

   if(keys[KEY_DOWN] && keys[KEY_LEFT] && keys[KEY_FIRE]) {
    alei.act(PLAYER_A_DOWNLEFTFIRE);
   }
   else if(keys[KEY_DOWN] && keys[KEY_RIGHT] && keys[KEY_FIRE]) {
    //PLAYER_A_DOWNRIGHTFIRE
    alei.act(PLAYER_A_DOWNRIGHTFIRE);
   }
   else if(keys[KEY_UP] && keys[KEY_LEFT] && keys[KEY_FIRE]) {
    alei.act(PLAYER_A_UPLEFTFIRE);
   }
   else if(keys[KEY_UP] && keys[KEY_RIGHT] && keys[KEY_FIRE]) {
    alei.act(PLAYER_A_UPRIGHTFIRE);
   }
   else if(keys[KEY_DOWN] && keys[KEY_FIRE]) {
    alei.act(PLAYER_A_DOWNFIRE);
   }
   else if(keys[KEY_LEFT] && keys[KEY_FIRE]) {
    alei.act(PLAYER_A_LEFTFIRE);
   }
   else if(keys[KEY_RIGHT] && keys[KEY_FIRE]) {
    alei.act(PLAYER_A_RIGHTFIRE);
   }
   else if(keys[KEY_UP] && keys[KEY_FIRE]) {
    alei.act(PLAYER_A_UPFIRE);
   }
   else if(keys[KEY_DOWN] && keys[KEY_LEFT]) {
    alei.act(PLAYER_A_DOWNLEFT);
   }
   else if(keys[KEY_DOWN] && keys[KEY_RIGHT]) {
    alei.act(PLAYER_A_DOWNRIGHT);
   }
   else if(keys[KEY_UP] && keys[KEY_LEFT]) {
    alei.act(PLAYER_A_UPLEFT);
   }
   else if(keys[KEY_UP] && keys[KEY_RIGHT]) {
    alei.act(PLAYER_A_UPRIGHT);
   }
   else if(keys[KEY_DOWN]) {
    alei.act(PLAYER_A_DOWN);
   }
   else if(keys[KEY_LEFT]) {
    alei.act(PLAYER_A_LEFT);
   }
   else if(keys[KEY_RIGHT]) {
    alei.act(PLAYER_A_RIGHT);
   }
   else if(keys[KEY_UP]) {
    alei.act(PLAYER_A_UP);
   }
   else if(keys[KEY_FIRE]) {
    alei.act(PLAYER_A_FIRE);
   }

   //Last, a requirement of the library, to prevent strange behaviors
   alei.act(PLAYER_A_NOOP);
}

void initRamVector() {
   for(unsigned i=0; i<128; i++) {
      previousRam[i] = -1;
      previousRAMFrames[i] = -1;
   }
}

string parseName(char* nameOfGame) {
  string aux = nameOfGame;
  int dotIndex = aux.size();
  int slashindex = 0;

  for(int i=dotIndex; i > 0; --i) {
    if(aux[i] == '.')
      dotIndex = i;
    else if(aux[i] == '/') {
      slashindex = i;
      break; //We have found wat we needed
    }
  }

  return aux.substr(slashindex+1, dotIndex-slashindex-1);
}

void initialize(char* nameOfGame) {
   struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = ctrlCHandler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);

   for(int i=0; i<N_KEYS; ++i) {
      keys[i] = false;
   }

   delayToFPS = 1000 / FPS;

   initRamVector();

   std::string name = parseName(nameOfGame);

   file.open(RAM_FILE+name+FILE_TERMINATION);
   fileBin.open(RAM_FILE_BINARY+name+FILE_TERMINATION);
}



int main(int argc, char **argv) {
   ///////

   // Check input parameter
   if (argc != 2) {
      usage(argv[0]);
      return -1;
   }

   initialize(argv[1]);

   // Create alei object.
   alei.setInt("random_seed", SEED);
   alei.setFloat("repeat_action_probability", 0);
   alei.setBool("display_screen", SCREEN);
   alei.setBool("sound", SOUND);
   alei.loadROM(argv[1]);   

   STDP::Inicializar();

   //Starting to read keyboard
   lector = std::thread(keyboardRead);
   
   int step;
   for (step = 0; 
        !alei.game_over() && step < MAX_STEPS; 
        ++step) 
   {
      printRam();
      agentStep();
      ramToFile();

      std::this_thread::sleep_for (std::chrono::milliseconds(delayToFPS));
   }
   
   lector.detach();
   STDP::Terminar();

   std::cout << "-- PROGRAM FINISHED --\n";
}