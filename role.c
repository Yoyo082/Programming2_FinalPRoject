#include "role.h" // 必須引入自己的標頭檔
#include <stddef.h> // 為了使用 NULL

// --- 基礎角色邏輯 ---

// 狼人刀人邏輯
void werewolf_kill(Player* target) {
    if (target != NULL && target->is_alive) {
        target->is_knifed = true;
    }
}

// 女巫救人：告知死者號碼並決定是否使用解藥
// 回傳 true 代表成功使用，false 代表不滿足條件（如自救、藥已用過、死者為 NULL）
bool witch_save(Player* players, int witch_id, int killed_id, bool* has_antidote) {
    // 必須有解藥、死者必須存在
    if (!(*has_antidote) || killed_id == -1) return false;

    // 規則判定：女巫被刀不能自救
    if (killed_id == witch_id) {
        return false; 
    }

    players[killed_id].is_saved = true;
    *has_antidote = false; // 標記解藥已使用
    return true;
}

// 女巫：使用毒藥
bool witch_poison(Player* target, bool* has_poison) {
    if (!(*has_poison) || !target->is_alive) return false;
    target->is_poisoned = true;
    *has_poison = false;
    return true;
}

// 守衛：守護 (傳入目標與昨晚守護的ID)
bool guard_protect(Player* target, int* last_guarded_id) {
    if (target->id == *last_guarded_id) return false; // 不能連守
    target->is_guarded = true;
    *last_guarded_id = target->id; // 更新昨晚守護紀錄
    return true;
}

// 預言家查驗
// 回傳 0 代表好人，1 代表狼人，-1 代表查驗失敗（如目標已死）
int seer_investigate(Player* players, int target_id, int alive_wolf_count) {
    if (!players[target_id].is_alive) return -1;

    // 隱狼：只要還有其他狼隊友活著，查驗結果永遠為好人 (0)
    if (players[target_id].role == ROLE_HIDDEN_WOLF && alive_wolf_count > 1) {
        return FACTION_GOOD;
    }

    return players[target_id].faction;
}

// 獵人判定：是否可以開槍
// 回傳 true 代表可以開槍，false 代表不能開槍 (如：被毒死)
bool hunter_can_shoot(Player* hunter) {
    // 防呆：確認該角色真的是獵人
    if (hunter->role != ROLE_HUNTER) return false;
    
    // 規則判定：如果身上有被女巫毒殺的標記，則無法開槍
    if (hunter->is_poisoned) return false;
    
    // 其餘死亡情況（被刀、被票死）皆可開槍
    return true;
}

// 獵人開槍帶人
void hunter_shoot(Player* target) {
    if (target != NULL && target->is_alive) {
        target->is_alive = false;
        target->can_vote = false;
        target->can_speak = false; // 直接剝奪發言與投票權
    }
}

// --- 進階版型邏輯 ---

// 騎士決鬥 (回傳 true 代表白天強制結束)
bool knight_duel(Player* players, int knight_id, int target_id) {
    if (!players[knight_id].is_alive || !players[target_id].is_alive) return false;

    // 騎士決鬥到狼人陣營 (包含白狼王、隱狼等)
    if (players[target_id].faction == FACTION_WOLF) {
        players[target_id].is_alive = false;
        players[target_id].can_vote = false;
        players[target_id].can_speak = false;
        return true; // 狼死，直接進入黑夜
    } 
    // 騎士決鬥到好人 (以死謝罪)
    else {
        players[knight_id].is_alive = false;
        players[knight_id].can_vote = false;
        players[knight_id].can_speak = false;
        return false; // 騎士死，白天繼續
    }
}

// 白狼王自爆 (回傳 true 代表自爆成功，白天強制結束)
bool white_wolf_king_explode(Player* players, int wwk_id, int target_id) {
    // 被毒死不能發動技能 (需仰賴第一部分在白天結算時保留 is_poisoned 狀態)
    if (players[wwk_id].is_poisoned || !players[wwk_id].is_alive) return false;
    
    // 帶走目標
    if (players[target_id].is_alive) {
        players[target_id].is_alive = false;
        players[target_id].can_vote = false;
        players[target_id].can_speak = false;
    }
    
    // 白狼王自己出局
    players[wwk_id].is_alive = false;
    players[wwk_id].can_vote = false;
    players[wwk_id].can_speak = false;
    
    return true; // 雙死，直接進入黑夜
}

// 隱狼覺醒判定 (第一部分的同學每晚狼人殺人前要呼叫此函式)
bool hidden_wolf_awakens(int total_alive_wolves, Player* hw_player) {
    // 只有自己一隻狼活著，且自己是隱狼
    if (total_alive_wolves == 1 && hw_player->role == ROLE_HIDDEN_WOLF) {
        return true; // 可以開始刀人
    }
    return false;
}

// 熊咆哮演算法
// 回傳 true 代表咆哮 (有狼)，false 代表沒咆哮
bool bear_roars(Player* players, int bear_id, int alive_wolf_count) {
    if (!players[bear_id].is_alive) return false; // 熊死了不叫

    int left = (bear_id - 1 + 12) % 12;
    int right = (bear_id + 1) % 12;

    // 往左找最近的活人
    while (!players[left].is_alive && left != bear_id) {
        left = (left - 1 + 12) % 12;
    }
    // 往右找最近的活人
    while (!players[right].is_alive && right != bear_id) {
        right = (right + 1) % 12;
    }

    // 判斷該活人是否為會觸發咆哮的狼 (隱狼且未覺醒時不算)
    bool is_left_wolf = (players[left].faction == FACTION_WOLF) && 
                        !(players[left].role == ROLE_HIDDEN_WOLF && alive_wolf_count > 1);
                        
    bool is_right_wolf = (players[right].faction == FACTION_WOLF) && 
                         !(players[right].role == ROLE_HIDDEN_WOLF && alive_wolf_count > 1);

    return (is_left_wolf || is_right_wolf);
}

// 白痴被投票出局時觸發
void idiot_reveal(Player* target) {
    if (target->role == ROLE_IDIOT) {
        target->is_alive = false;  // 視為死亡，狼人不需要追刀
        target->can_vote = false;  // 失去投票權
        target->can_speak = true;  // 保留發言權 (這是重點！)
    }
}

// --- 死亡結算引擎 --- 
// 主遊戲流程在「天亮宣佈死訊」前，必須呼叫這個函式
void finalize_night_results(Player* players, int player_count) {
    for (int i = 0; i < player_count; i++) {
        if (!players[i].is_alive) continue;

        bool will_die = false;

        // 狼刀判定與奶穿 (同守同救)
        if (players[i].is_knifed) {
            if (players[i].is_guarded && players[i].is_saved) {
                will_die = true; // 奶穿，必死
            } else if (players[i].is_guarded || players[i].is_saved) {
                will_die = false; // 單守或單救，活下來
            } else {
                will_die = true; // 沒人救沒人守，被刀死
            }
        }

        // 毒藥絕對優先級 (無視守衛)
        if (players[i].is_poisoned) {
            will_die = true; 
        }

        // 執行死亡
        if (will_die) {
            players[i].is_alive = false;
            players[i].can_vote = false;
            players[i].can_speak = false; // 真死，不能發言
        }
    }
}

// 檢查遊戲勝負條件 (屠邊局規則)
// 回傳值: 0 代表遊戲繼續，1 代表好人贏，2 代表狼人贏
int check_win_condition(Player *players)
{
    int alive_wolves = 0;
    int alive_villagers = 0;
    int alive_gods = 0;

    for (int i = 1; i <= 12; i++)
    {
        if (players[i].is_alive)
        {
            if (players[i].faction == FACTION_WOLF)
            {
                alive_wolves++;
            }
            else if (players[i].role == ROLE_VILLAGER)
            {
                alive_villagers++;
            }
            else
            {
                // 只要不是狼也不是平民，就是神職
                alive_gods++;
            }
        }
    }

    if (alive_wolves == 0)
        return 1; // 狼死光，好人贏
    if (alive_villagers == 0 || alive_gods == 0)
        return 2; // 神或民任一方死光，狼人贏

    return 0; // 都還沒死光，遊戲繼續
}
