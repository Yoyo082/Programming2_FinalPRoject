#pragma once

//所有角色
typedef enum {
    ROLE_NONE,
    ROLE_WEREWOLF,
    ROLE_VILLAGER,
    ROLE_SEER,
    ROLE_WITCH,
    ROLE_HUNTER
} Role;

//玩家資料
typedef struct {
    int        id;
    char       name[32];
    Role       role;
    int        isAlive;     // 1 = 存活, 0 = 死亡 
    int        isRevealed;  // 1 = 角色已公開 
} Player;



Player      CreatePlayer(int id, const char* name);
