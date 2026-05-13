#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include "player.h"
#include "state.h"

void run_night_phase(Player* players, GameState* state);
void run_day_phase(Player* players, GameState* state);

#endif