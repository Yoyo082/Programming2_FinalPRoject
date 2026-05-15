#ifndef ROLE_H
#define ROLE_H

#include "player.h"
#include "state.h"  // 新增這行

// --- 基礎神職與狼人 ---
void werewolf_kill(Player* target);

bool witch_save(Player* players, int witch_id, int killed_id, GameState* state);
bool witch_poison(Player* target, GameState* state);
int seer_investigate(Player* players, int target_id, GameState* state);
bool guard_protect(Player* target, GameState* state);
bool hunter_can_shoot(Player* hunter);
void hunter_shoot(Player* target);

// --- 死亡結算 ---
void finalize_night_results(Player* players, int player_count);
int check_win_condition(Player *players);

// --- 進階版型 ---
int knight_duel(Player* players, int knight_id, int target_id, GameState* state);
bool white_wolf_king_explode(Player* players, int wwk_id, int target_id);
bool bear_roars(Player* players, int bear_id, GameState* state);
bool hidden_wolf_awakens(GameState* state, Player* hw_player);
void idiot_reveal(Player* target);

#endif