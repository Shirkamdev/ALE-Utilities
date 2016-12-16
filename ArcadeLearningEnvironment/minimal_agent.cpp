#include <iostream>
#include <cmath>
#include "src/ale_interface.hpp"

// Global vars
const int maxSteps = 7500;
int lastLives;
float totalReward;
ALEInterface alei;

///////////////////////////////////////////////////////////////////////////////
/// Get info from RAM
///////////////////////////////////////////////////////////////////////////////
int getPlayerX() {
   return alei.getRAM().get(72) + ((rand() % 3) - 1);
}

int getBallX() {
   return alei.getRAM().get(99) + ((rand() % 3) - 1);
}

///////////////////////////////////////////////////////////////////////////////
/// Do Next Agent Step
///////////////////////////////////////////////////////////////////////////////
float agentStep() {
   static int wide = 9;
   float reward = 0;

   if (alei.lives() != lastLives) {
      --lastLives;
      alei.act(PLAYER_A_FIRE);
   }

   // Apply rules.
   int playerX = getPlayerX();
   int ballX = getBallX();
   if (ballX < playerX + wide) {
      reward += alei.act(PLAYER_A_LEFT);
   } else if (ballX > playerX + wide) {
      reward += alei.act(PLAYER_A_RIGHT);
   } 
   
   return (reward + alei.act(PLAYER_A_NOOP));
}

///////////////////////////////////////////////////////////////////////////////
/// Print usage and exit
///////////////////////////////////////////////////////////////////////////////
void usage(char* pname) {
   std::cerr
      << "\nUSAGE:\n" 
      << "   " << pname << " <romfile>\n";
   exit(-1);
}

///////////////////////////////////////////////////////////////////////////////
/// MAIN PROGRAM
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
   // Check input parameter
   if (argc != 2)
      usage(argv[0]);

   // Create alei object.
   alei.setInt("random_seed", 0);
   alei.setFloat("repeat_action_probability", 0);
   alei.setBool("display_screen", true);
   alei.setBool("sound", true);
   alei.loadROM(argv[1]);

   // Init
   srand(time(NULL));
   lastLives = alei.lives();
   totalReward = .0f;

   // Main loop
   alei.act(PLAYER_A_FIRE);
   int step;
   for (step = 0; 
        !alei.game_over() && step < maxSteps; 
        ++step) 
   {
      totalReward += agentStep();
   }

   std::cout << "Steps: " << step << std::endl;
   std::cout << "Reward: " << totalReward << std::endl;

   return 0;
}
