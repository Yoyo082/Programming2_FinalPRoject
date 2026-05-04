#include "role.h"

//創建玩家
Player CreatePlayer(int id, const char* name) {
    Player p;
    p.id         = id;
    p.isAlive    = 1;
    p.isRevealed = 0;
    p.role       = ROLE_NONE;
    strncpy(p.name, name, sizeof(p.name) - 1);
    p.name[sizeof(p.name) - 1] = '\0';  
    return p;
}


