#include <stdio.h>
#include <stdbool.h>
#include "player.h"
#include "role.h"
#include <stdlib.h>
#include <time.h>

int main(){
    Player players[12];
    int last_guarded_id=-1; //-1代表還為守護過
    bool witch_has_antidote=true;
    bool witch_has_poison=true;
    int alive_wolf_count=4;

    int version_choice=0;

    printf("==========================================\n");
    printf("         🐺 歡迎來到狼人殺 🐺         \n");
    printf("==========================================\n");
    printf("請選擇您今天要遊玩的版本：\n");
    printf("[1] 白狼王騎士局 \n");
    printf("[2] 熊隱狼局 \n");
    printf("------------------------------------------\n");
    printf("請輸入數字 (1-2): ");

    scanf("%d", &version_choice);
    if(version_choice==1){
        RoleType role_deck[12]={
            ROLE_WHITE_WOLF_KING, ROLE_WEREWOLF, ROLE_WEREWOLF, ROLE_WEREWOLF, 
            ROLE_SEER, ROLE_WITCH, ROLE_GUARD, ROLE_KNIGHT, 
            ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER
        };

        srand(time(NULL));

        //身份deck洗牌
        for(int i=11 ; i>0 ; i--){
            int j=rand()%(i+1);

            RoleType temp=role_deck[i];
            role_deck[i]=role_deck[j];
            role_deck[j]=temp;
        }

        for (int i = 0; i < 12; i++) {
            players[i].id=i;
            players[i].is_alive=true;
            players[i].can_vote=true;
            players[i].can_speak=true;
            players[i].is_knifed=false;
            players[i].is_saved=false;
            players[i].is_poisoned=false;
            players[i].is_guarded=false;

            players[i].role=role_deck[i]; //發洗好的牌

            //判斷是好人還是壞人
            if (players[i].role==ROLE_WEREWOLF || players[i].role==ROLE_WHITE_WOLF_KING) {
                players[i].faction=FACTION_WOLF;
            } else {
                players[i].faction=FACTION_GOOD;
            }
        }
    }

    

}