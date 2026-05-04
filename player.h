#ifndef PLAYER_H
#define PLAYER_H
#include <stdbool.h>

// 定義陣營
#define FACTION_GOOD 0
#define FACTION_WOLF 1

// 定義角色 ID
typedef enum {
    ROLE_VILLAGER, 
    ROLE_SEER, 
    ROLE_WITCH, 
    ROLE_HUNTER, 
    ROLE_GUARD, 
    ROLE_IDIOT,
    ROLE_WEREWOLF, 
    ROLE_KNIGHT, 
    ROLE_WHITE_WOLF_KING, 
    ROLE_BEAR, 
    ROLE_HIDDEN_WOLF
} RoleType;

// 玩家結構體
typedef struct {
    int id;                 // 玩家座位號 (0~11)
    RoleType role;          // 真實角色
    int faction;            // 陣營 (FACTION_GOOD 或 FACTION_WOLF)
    
    // 基礎狀態
    bool is_alive;          // 是否存活 (白痴翻牌後設為 false)
    bool can_vote;          // 是否能投票
    bool can_speak;         // 是否能發言
    
    // 夜晚技能標記 (每晚天黑前必須全部清為 false)
    bool is_knifed;         // 今晚是否被狼刀
    bool is_saved;          // 今晚是否被女巫救
    bool is_poisoned;       // 今晚是否被女巫毒
    bool is_guarded;        // 今晚是否被守衛守
} Player;

#endif