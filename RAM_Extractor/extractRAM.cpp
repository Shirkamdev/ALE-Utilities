/*
  This program was indented to help any ALE developer.

  Made by Shirkamdev
    Debugging and small fixes by Reyeselda95

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

#include <memory>
#include <stdexcept>

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

#define KEY_UP 72 //8 del teclado numerico
#define KEY_DOWN 80 //2 del teclado numerico
#define KEY_LEFT 75 //4 del teclado numerico
#define KEY_RIGHT 77 //6 del teclado numerico
#define KEY_FIRE 57 //SPACE

////////////////////////////
// END OF KEY DEFINITIONS //
////////////////////////////

#define RAM_FILE "ramExtract_"
#define RAM_FILE_BINARY "binRAMExtract_"
#define RAM_FILE_0_TO_1 "per1Extract_"
#define FILE_TERMINATION ".csv"


static const char *const evval[3] = {
    "RELEASED",
    "PRESSED ",
    "REPEATED"
};

#define INPUT_KEYBOARD "/dev/input/" 

const int MAX_STEPS = 99500;
int previousRam[NUM_BYTES_RAM];
int previousRAMFrames[NUM_BYTES_RAM];
ALEInterface alei;

bool keys[N_KEYS];
short delayToFPS;

ofstream file, fileBin, filePer1;

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

void ramToFile(std::string name) {
  
  //Printing in HEX (0x)
	file.open(RAM_FILE+name+FILE_TERMINATION,ios::out | ios::app);
	for(int i = 0 ;i < NUM_BYTES_RAM; ++i) {
	  int valor = alei.getRAM().get(i);
	  file << valor;
	  //file << std::hex << valor;
	  if(i != NUM_BYTES_RAM-1) {
	     file << ";";
	  }
	  else {
	  //   file << "\n"; //Last value to print (CSV format)
	  }
	}
	file.close();


	//Printing in binnary
	/*fileBin.open(RAM_FILE_BINARY+name+FILE_TERMINATION, ios::out | ios::app);
	for(int i = 0 ;i < NUM_BYTES_RAM; ++i) {
	  int valor = alei.getRAM().get(i);
	  fileBin << std::bitset<8>(valor);
	  if(i != NUM_BYTES_RAM-1) {
	     fileBin << ",";
	  }
	  else {
	     fileBin << "\n"; //Last value to print (CSV format)
	  }
	}
	fileBin.close();*/

  filePer1.open(RAM_FILE_0_TO_1+name+FILE_TERMINATION, ios::out | ios::app);
  for(int i = 0 ;i < NUM_BYTES_RAM; ++i) {
    int valor = alei.getRAM().get(i);
    filePer1 << (double) valor/255.0;
    if(i != NUM_BYTES_RAM-1) {
       filePer1 << ";";
    }
    else {
    //   filePer1 << "\n"; //Last value to print (CSV format)
    }
  }
  filePer1.close();

}

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    result=result.substr(0,result.find("\n"));
    return result;
}

void keyboardRead() {
	//Obtain the kbd event route

 	std::string aout=INPUT_KEYBOARD;
 	//aout+="event2";
 	aout+=exec("cat /proc/bus/input/devices | awk '/keyboard/{for(a=0;a>=0;a++){getline;{if(/kbd/==1){ print \n$NF;exit 0;}}}}'| cut -d' ' -f4"); 

    const char *dev = aout.c_str();
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

inline const char * const BoolToString(bool b)
{
  return b ? "1" : "0";
}

void agentStep(std::string name){
   static int wide = 9;
   float reward = 0;
   
   std::string transform;
   transform+=BoolToString(keys[KEY_UP]);
   transform+=BoolToString(keys[KEY_DOWN]);
   transform+=BoolToString(keys[KEY_LEFT]);
   transform+=BoolToString(keys[KEY_RIGHT]);
   transform+=BoolToString(keys[KEY_FIRE]);

   int eval=atoi(transform.c_str());

   //String gets from the order above [UP,DOWN,LEFT,RIHT,FIRE]
   
   switch(eval){
   	case 10000:
   		alei.act(PLAYER_A_UP);
   		break;
   	case 1000:
   		alei.act(PLAYER_A_DOWN);
   		break;
   	case 100:
   		alei.act(PLAYER_A_LEFT);
   		break;
   	case 10:
   		alei.act(PLAYER_A_RIGHT);
   		break;
   	case 10100:
   		alei.act(PLAYER_A_UPLEFT);
   		break;
   	case 10010:
   		alei.act(PLAYER_A_UPRIGHT);
   		break;
   	case 1100:
   		alei.act(PLAYER_A_DOWNLEFT);
   		break;
   	case 1010:
   		alei.act(PLAYER_A_DOWNRIGHT);
   		break;
   	
   	case 11100:
   		alei.act(PLAYER_A_LEFT);
   		break;
   	case 11010:
   		alei.act(PLAYER_A_RIGHT);
   		break;
   	case 10110:
   		alei.act(PLAYER_A_UP);
   		break;
   	case 1110:
   		alei.act(PLAYER_A_DOWN);
   		break;


   	case 1:
   		alei.act(PLAYER_A_FIRE);
   		break;

   	case 10001:
   		alei.act(PLAYER_A_UPFIRE);
   		break;
   	case 1001:
   		alei.act(PLAYER_A_DOWNFIRE);
   		break;
   	case 101:
   		alei.act(PLAYER_A_LEFTFIRE);
   		break;
   	case 11:
   		alei.act(PLAYER_A_RIGHTFIRE);
   		break;
   	case 10101:
   		alei.act(PLAYER_A_UPLEFTFIRE);
   		break;
   	case 10011:
   		alei.act(PLAYER_A_UPRIGHTFIRE);
   		break;
   	case 1101:
   		alei.act(PLAYER_A_DOWNLEFTFIRE);
   		break;
   	case 1011:
   		alei.act(PLAYER_A_DOWNRIGHTFIRE);
   		break;

	case 11101:
   		alei.act(PLAYER_A_LEFTFIRE);
   		break;
   	case 11011:
   		alei.act(PLAYER_A_RIGHTFIRE);
   		break;
   	case 10111:
   		alei.act(PLAYER_A_UPFIRE);
   		break;
   	case 1111:
   		alei.act(PLAYER_A_DOWNFIRE);
   		break;
   	default:
	   	alei.act(PLAYER_A_NOOP);
   		break;
   }
   
   	ramToFile(name);
	file.open(RAM_FILE+name+FILE_TERMINATION,ios::out | ios::app);
	file <<";"<<BoolToString(keys[KEY_UP]) << ";" << BoolToString(keys[KEY_DOWN])<<";"<<BoolToString(keys[KEY_LEFT])<<";"<<BoolToString(keys[KEY_RIGHT])<<";"<<BoolToString(keys[KEY_FIRE]) << "\n";
	file.close();

	/*file.open(RAM_FILE_BINARY+name+FILE_TERMINATION,ios::out | ios::app);
	file <<";"<<BoolToString(keys[KEY_UP]) << ";" << BoolToString(keys[KEY_DOWN])<<";"<<BoolToString(keys[KEY_LEFT])<<";"<<BoolToString(keys[KEY_RIGHT])<<";"<<BoolToString(keys[KEY_FIRE]) << "\n";
	file.close();*/

	file.open(RAM_FILE_0_TO_1=+name+FILE_TERMINATION,ios::out | ios::app);
	file <<";"<<BoolToString(keys[KEY_UP]) << ";" << BoolToString(keys[KEY_DOWN])<<";"<<BoolToString(keys[KEY_LEFT])<<";"<<BoolToString(keys[KEY_RIGHT])<<";"<<BoolToString(keys[KEY_FIRE]) << "\n";
	file.close();

   //HERE we read from the key buffer that gives us the thread
   /*
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
	*/   
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
	std::string name= parseName(nameOfGame);

	remove( (RAM_FILE+name+FILE_TERMINATION).c_str() );//it erases the file with previous file with the same name of the game
	remove( (RAM_FILE_BINARY+name+FILE_TERMINATION).c_str() );
}

int main(int argc, char **argv) {
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
	std::string name= parseName(argv[1]);
	int step;
	while (!alei.game_over()) 
	{

		printRam();
		agentStep(name);

		std::this_thread::sleep_for (std::chrono::milliseconds(delayToFPS));
	}

	lector.detach();
	STDP::Terminar();

	std::cout << "-- PROGRAM FINISHED --\n";
	return 0;
}