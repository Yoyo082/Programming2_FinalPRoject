#include <stdio.h>
#include <stdbool.h>
#include "player.h"
#include "role.h"
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
    #define CLEAR_CMD "cls"
    #include <windows.h>
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#else
    #define CLEAR_CMD "clear"
#endif


int main(){
    

    Player players[13];
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
        RoleType role_deck[13]={
            ROLE_VILLAGER, //0號牌不使用
            ROLE_WHITE_WOLF_KING, ROLE_WEREWOLF, ROLE_WEREWOLF, ROLE_WEREWOLF, 
            ROLE_SEER, ROLE_WITCH, ROLE_GUARD, ROLE_KNIGHT, 
            ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER
        };

        srand(time(NULL));

        //身份deck洗牌
        for(int i=12 ; i>1 ; i--){
            int j=(rand()%i)+1;

            RoleType temp=role_deck[i];
            role_deck[i]=role_deck[j];
            role_deck[j]=temp;
        }

        for (int i=1; i<=12; i++) {
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

        //玩家輪流看牌
        printf("\n=> 法官：發牌完畢！接下來請依照順序查看身分底牌。\n");
        
        int dummy=0; //用來卡住畫面

        for(int i=1 ; i<=12 ; i++){
            system(CLEAR_CMD);

            printf("\n==========================================\n");
            printf("       現在輪到【 %d 號玩家 】查看身分       \n", i);
            printf("==========================================\n");
            printf("如果你是 %d 號玩家本人，請輸入 1 並按 Enter 翻牌：", i);
            scanf("%d", &dummy);

            printf("\n------------------------------------------\n");
            printf("你的身分是：");
            switch (players[i].role) {
                case ROLE_WHITE_WOLF_KING: 
                    printf("白狼王\n"); 
                    break;
                case ROLE_WEREWOLF:        
                    printf("狼人\n"); 
                    break;
                case ROLE_SEER:            
                    printf("預言家\n"); 
                    break;
                case ROLE_WITCH:           
                    printf("女巫\n"); 
                    break;
                case ROLE_GUARD:           
                    printf("守衛\n"); 
                    break;
                case ROLE_KNIGHT:          
                    printf("騎士\n"); 
                    break;
                case ROLE_VILLAGER:        
                    printf("平民\n"); 
                    break;
            }
            printf("------------------------------------------\n");

            // 蓋牌等待換人
            printf("\n請確實記住你的身分！\n");
            printf("看完後，請輸入 0 並按 Enter 蓋牌，然後交給下一位玩家：");
            scanf("%d", &dummy);
        }

        // 所有人看完後，最後清空一次螢幕，準備進入夜晚
        system(CLEAR_CMD);
        printf("\n==========================================\n");
        printf("=> 法官：所有人皆已確認身分！進入黑夜！\n");
        printf("==========================================\n");
            
    }

}