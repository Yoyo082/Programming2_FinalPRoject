#ifndef PLAYER_H
#define PLAYER_H
#include <stdbool.h> //後續判斷是否活著，使用bool(true/false)

typedef enum {
    villager=1, // 平民
    werewolf=2, // 狼人
    witch=3, // 女巫
    seer=4, // 預言家
    guard=5, // 守衛
    hunter=6 // 獵人
} role_type;

typedef struct{
    int id; //玩家座號
    role_type role; //玩家角色
    bool is_alive; //玩家活著還是死亡

    //黑夜狀態
    bool is_guarded; //玩家是否被守衛守護
    bool is_killed; //玩家是否被狼刀
    bool is_saved; //玩家是否被女巫救
    bool is_poisoned; //玩家是否被女巫投毒藥
} Player;

#endif
