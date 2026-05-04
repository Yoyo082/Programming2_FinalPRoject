#ifndef ROLE_H
#define ROLE_H

#include "player.h"

// 基礎神職與狼人
void werewolf_kill(Player* target);
bool witch_save(Player* players, int witch_id, int killed_id, bool* has_antidote);
bool witch_poison(Player* target, bool* has_poison);
int seer_investigate(Player* players, int target_id, int alive_wolf_count);
bool guard_protect(Player* target, int* last_guarded_id);

// 死亡結算
void finalize_night_results(Player* players, int player_count);

// 進階版型
bool knight_duel(Player* players, int knight_id, int target_id);
bool white_wolf_king_explode(Player* players, int wwk_id, int target_id);
bool bear_roars(Player* players, int bear_id, int alive_wolf_count);
bool hidden_wolf_awakens(int total_alive_wolves, Player* hw_player);
void idiot_reveal(Player* target);

#endif