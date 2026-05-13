#include "setup.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 處理不同作業系統的清空螢幕指令
#ifdef _WIN32
#define CLEAR_CMD "cls"
#else
#define CLEAR_CMD "clear"
#endif

// 將妳的發牌邏輯包裝在這個函式裡面
void initialize_game(Player* players, GameState* state, int version_choice) {
    
    // 1. 初始化狀態機 (GameState) 的預設數值
    state->current_board = version_choice;
    state->day_count = 1;
    state->game_result = 0;
    state->is_game_over = false;
    state->last_guarded_id = -1;
    state->witch_has_antidote = true;
    state->witch_has_poison = true;
    state->alive_wolf_count = 4; // 目前兩個版子剛好都是4狼
    state->is_night = true;
    state->killed_last_night = -1;

    // 2. 開始發牌與看牌邏輯
    if (version_choice == 1) {
        RoleType role_deck[13] = {
            ROLE_VILLAGER, // 0號牌不使用
            ROLE_WHITE_WOLF_KING, ROLE_WEREWOLF, ROLE_WEREWOLF, ROLE_WEREWOLF,
            ROLE_SEER, ROLE_WITCH, ROLE_GUARD, ROLE_KNIGHT,
            ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER};

        srand(time(NULL));

        // 身份deck洗牌
        for (int i = 12; i > 1; i--) {
            int j = (rand() % i) + 1;
            RoleType temp = role_deck[i];
            role_deck[i] = role_deck[j];
            role_deck[j] = temp;
        }

        // 發牌並設定基礎狀態
        for (int i = 1; i <= 12; i++) {
            players[i].id = i;
            players[i].is_alive = true;
            players[i].can_vote = true;
            players[i].can_speak = true;
            players[i].is_knifed = false;
            players[i].is_saved = false;
            players[i].is_poisoned = false;
            players[i].is_guarded = false;
            players[i].role = role_deck[i]; 

            if (players[i].role == ROLE_WEREWOLF || players[i].role == ROLE_WHITE_WOLF_KING)
                players[i].faction = FACTION_WOLF;
            else
                players[i].faction = FACTION_GOOD;
        }

        // 玩家輪流看牌
        printf("\n=> 法官：發牌完畢！接下來請依照順序查看身分底牌。\n");
        int dummy = 0; 
        for (int i = 1; i <= 12; i++) {
            system(CLEAR_CMD);
            printf("\n==========================================\n");
            printf("       現在輪到【 %d 號玩家 】查看身分       \n", i);
            printf("==========================================\n");
            printf("如果你是 %d 號玩家本人，請輸入 1 並按 Enter 翻牌：", i);
            scanf("%d", &dummy);

            printf("\n------------------------------------------\n");
            printf("你的身分是：");
            switch (players[i].role) {
                case ROLE_WHITE_WOLF_KING: printf("白狼王\n"); break;
                case ROLE_WEREWOLF: printf("狼人\n"); break;
                case ROLE_SEER: printf("預言家\n"); break;
                case ROLE_WITCH: printf("女巫\n"); break;
                case ROLE_GUARD: printf("守衛\n"); break;
                case ROLE_KNIGHT: printf("騎士\n"); break;
                case ROLE_VILLAGER: printf("平民\n"); break;
                default: printf("其他神祕身分\n"); break;
            }
            printf("------------------------------------------\n");
            printf("\n請確實記住你的身分！\n");
            printf("看完後，請輸入 0 並按 Enter 蓋牌，然後交給下一位玩家：");
            scanf("%d", &dummy);
        }
        
        system(CLEAR_CMD);
        printf("\n==========================================\n");
        printf("=> 法官：所有人皆已確認身分！進入黑夜！\n");
        printf("==========================================\n");

    } else if (version_choice == 2) {
        
        // 【熊隱狼局發牌邏輯】
        RoleType role_deck[13] = {
            ROLE_VILLAGER,
            ROLE_HIDDEN_WOLF, ROLE_WEREWOLF, ROLE_WEREWOLF, ROLE_WEREWOLF,
            ROLE_BEAR, ROLE_WITCH, ROLE_HUNTER, ROLE_IDIOT,
            ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER};

        srand(time(NULL));

        for (int i = 12; i > 1; i--) {
            int j = (rand() % i) + 1;
            RoleType temp = role_deck[i];
            role_deck[i] = role_deck[j];
            role_deck[j] = temp;
        }

        for (int i = 1; i <= 12; i++) {
            players[i].id = i;
            players[i].is_alive = true;
            players[i].can_vote = true;
            players[i].can_speak = true;
            players[i].is_knifed = false;
            players[i].is_saved = false;
            players[i].is_poisoned = false;
            players[i].is_guarded = false;
            players[i].role = role_deck[i];

            if (players[i].role == ROLE_WEREWOLF || players[i].role == ROLE_HIDDEN_WOLF)
                players[i].faction = FACTION_WOLF;
            else
                players[i].faction = FACTION_GOOD;
        }

        // 玩家輪流看牌
        printf("\n=> 法官：發牌完畢！接下來請依照順序查看身分底牌。\n");
        int dummy = 0;
        for (int i = 1; i <= 12; i++) {
            system(CLEAR_CMD);
            printf("\n==========================================\n");
            printf("       現在輪到【 %d 號玩家 】查看身分       \n", i);
            printf("==========================================\n");
            printf("如果你是 %d 號玩家本人，請輸入 1 並按 Enter 翻牌：", i);
            scanf("%d", &dummy);

            printf("\n------------------------------------------\n");
            printf("你的身分是：");
            switch (players[i].role) {
                case ROLE_HIDDEN_WOLF: printf("隱狼\n"); break;
                case ROLE_WEREWOLF: printf("狼人\n"); break;
                case ROLE_BEAR: printf("熊\n"); break;
                case ROLE_WITCH: printf("女巫\n"); break;
                case ROLE_HUNTER: printf("獵人\n"); break;
                case ROLE_IDIOT: printf("白痴\n"); break;
                case ROLE_GUARD: printf("守衛\n"); break;
                case ROLE_SEER: printf("預言家\n"); break;
                case ROLE_VILLAGER: printf("平民\n"); break;
                default: printf("其他神祕身分\n"); break;
            }
            printf("------------------------------------------\n");
            printf("\n請確實記住你的身分！\n");
            printf("看完後，請輸入 0 並按 Enter 蓋牌，然後交給下一位玩家：");
            scanf("%d", &dummy);
        }

        system(CLEAR_CMD);
        printf("\n==========================================\n");
        printf("=> 法官：所有人皆已確認身分！進入黑夜！\n");
        printf("==========================================\n");
    }
}