//專門處理發牌、洗牌、設定玩家初始狀態
#ifndef SETUP_H
#define SETUP_H
#include "player.h"
#include "state.h"

// 負責發牌與設定玩家初始狀態
void initialize_game(Player* players, GameState* state, int version_choice);

#endif