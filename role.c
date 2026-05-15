#include "role.h" 
#include <stddef.h> 

// --- 基礎角色邏輯 ---

// 狼人刀人邏輯
void werewolf_kill(Player* target) {
    if (target != NULL && target->is_alive) {
        target->is_knifed = true;
    }
}

bool witch_save(Player* players, int witch_id, int killed_id, GameState* state) {
    // 防呆：從 state 中確認是否有解藥
    if (!(state->witch_has_antidote) || killed_id == -1) return false;
    if (killed_id == witch_id) return false; 

    players[killed_id].is_saved = true;
    state->witch_has_antidote = false; //  更新狀態機，解藥用掉了
    return true;
}

bool witch_poison(Player* target, GameState* state) {
    // 防呆：從 state 中確認是否有毒藥
    if (!(state->witch_has_poison) || !target->is_alive) return false;
    
    target->is_poisoned = true;
    state->witch_has_poison = false; //  更新狀態機，毒藥用掉了
    return true;
}

bool guard_protect(Player* target, GameState* state) {
    //  透過 state 讀取昨晚守護紀錄
    if (target->id == state->last_guarded_id) return false; 
    
    target->is_guarded = true;
    state->last_guarded_id = target->id; // 更新狀態機裡的紀錄
    return true;
}

int seer_investigate(Player* players, int target_id, GameState* state) {
    if (!players[target_id].is_alive) return -1;
    return players[target_id].faction;
}

// 獵人判定：是否可以開槍
bool hunter_can_shoot(Player* hunter) {
    if (hunter->role != ROLE_HUNTER) return false;
    if (hunter->is_poisoned) return false;
    return true;
}

// 獵人開槍帶人
void hunter_shoot(Player* target) {
    if (target != NULL && target->is_alive) {
        target->is_alive = false;
        target->can_vote = false;
        target->can_speak = false; 
    }
}

// --- 進階版型邏輯 ---

// 騎士決鬥 (回傳 1:殺死狼, 0:殺錯好人, -1:發動無效)
int knight_duel(Player* players, int knight_id, int target_id, GameState* state) {
    // 1. 防呆攔截：技能已用過、騎士已死、目標已死、發動者根本不是騎士
    if (state->knight_has_dueled) return -1;
    if (!players[knight_id].is_alive || !players[target_id].is_alive) return -1;
    if (players[knight_id].role != ROLE_KNIGHT) return -1;

    // 2. 鎖上技能 (這局遊戲不能再用了)
    state->knight_has_dueled = true;

    // 3. 判定決鬥結果
    if (players[target_id].faction == FACTION_WOLF) {
        players[target_id].is_alive = false;
        players[target_id].can_vote = false;
        players[target_id].can_speak = false;
        return 1; // 決鬥成功，目標死亡
    } else {
        players[knight_id].is_alive = false;
        players[knight_id].can_vote = false;
        players[knight_id].can_speak = false;
        return 0; // 決鬥失敗，騎士自己死亡
    }
}

// 白狼王自爆
bool white_wolf_king_explode(Player* players, int wwk_id, int target_id) {
    if (players[wwk_id].is_poisoned || !players[wwk_id].is_alive) return false;
    
    if (players[target_id].is_alive) {
        players[target_id].is_alive = false;
        players[target_id].can_vote = false;
        players[target_id].can_speak = false;
    }
    
    players[wwk_id].is_alive = false;
    players[wwk_id].can_vote = false;
    players[wwk_id].can_speak = false;
    
    return true; 
}

// 修改：接收 GameState* state
bool hidden_wolf_awakens(GameState* state, Player* hw_player) {
    //  從狀態機讀取活狼數
    if (state->alive_wolf_count == 1 && hw_player->role == ROLE_HIDDEN_WOLF) {
        return true; 
    }
    return false;
}

// 修改：接收 GameState* state
bool bear_roars(Player* players, int bear_id, GameState* state) {
    if (!players[bear_id].is_alive) return false; 

    int left = (bear_id - 1 + 12) % 12;
    int right = (bear_id + 1) % 12;

    while (!players[left].is_alive && left != bear_id) {
        left = (left - 1 + 12) % 12;
    }
    while (!players[right].is_alive && right != bear_id) {
        right = (right + 1) % 12;
    }

    // 判斷時改用 state->alive_wolf_count
    bool is_left_wolf = (players[left].faction == FACTION_WOLF) && 
                        !(players[left].role == ROLE_HIDDEN_WOLF && state->alive_wolf_count > 1);
                        
    bool is_right_wolf = (players[right].faction == FACTION_WOLF) && 
                         !(players[right].role == ROLE_HIDDEN_WOLF && state->alive_wolf_count > 1);

    return (is_left_wolf || is_right_wolf);
}

// 白痴被投票出局時觸發
void idiot_reveal(Player* target) {
    if (target->role == ROLE_IDIOT) {
        target->is_alive = false;  
        target->can_vote = false;  
        target->can_speak = true;  
    }
}

// --- 死亡結算引擎 --- 
void finalize_night_results(Player* players, int player_count) {
    for (int i = 0; i < player_count; i++) {
        if (!players[i].is_alive) continue;

        bool will_die = false;

        if (players[i].is_knifed) {
            if (players[i].is_guarded && players[i].is_saved) {
                will_die = true; 
            } else if (players[i].is_guarded || players[i].is_saved) {
                will_die = false; 
            } else {
                will_die = true; 
            }
        }

        if (players[i].is_poisoned) {
            will_die = true; 
        }

        if (will_die) {
            players[i].is_alive = false;
            players[i].can_vote = false;
            players[i].can_speak = false; 
        }
    }
}

// 檢查遊戲勝負條件
int check_win_condition(Player *players) {
    int alive_wolves = 0;
    int alive_villagers = 0;
    int alive_gods = 0;

    for (int i = 1; i <= 12; i++) {
        if (players[i].is_alive) {
            if (players[i].faction == FACTION_WOLF) {
                alive_wolves++;
            } else if (players[i].role == ROLE_VILLAGER) {
                alive_villagers++;
            } else {
                alive_gods++;
            }
        }
    }

    if (alive_wolves == 0) return 1; 
    if (alive_villagers == 0 || alive_gods == 0) return 2; 

    return 0; 
}