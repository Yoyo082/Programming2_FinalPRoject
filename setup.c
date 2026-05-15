#include "setup.h"
#include "ui.h"
#include <stdlib.h>
#include <time.h>

// 將妳的發牌邏輯包裝在這個函式裡面
void initialize_game(Player *players, GameState *state, int version_choice) {

    // 1. 初始化狀態機 (GameState) 的預設數值
    state->current_board = version_choice;
    state->day_count = 1;
    state->game_result = 0;
    state->is_game_over = false;
    state->last_guarded_id = -1;
    state->witch_has_antidote = true;
    state->witch_has_poison = true;
    state->knight_has_dueled = false;
    state->alive_wolf_count = 4; // 目前兩個版子剛好都是4狼
    state->is_night = true;
    state->killed_last_night = -1;

    // 2. 發牌並設定基礎狀態
    if (version_choice == 1) {
        RoleType role_deck[13] = {
            ROLE_VILLAGER, // 0號牌不使用
            ROLE_WHITE_WOLF_KING, ROLE_WEREWOLF, ROLE_WEREWOLF, ROLE_WEREWOLF,
            ROLE_SEER, ROLE_WITCH, ROLE_GUARD, ROLE_KNIGHT,
            ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER};

        srand((unsigned int)time(NULL));

        // 身份deck洗牌
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
            players[i].faction = (players[i].role == ROLE_WEREWOLF || players[i].role == ROLE_WHITE_WOLF_KING)
                                     ? FACTION_WOLF
                                     : FACTION_GOOD;
        }
    } else if (version_choice == 2) {
        RoleType role_deck[13] = {
            ROLE_VILLAGER,
            ROLE_HIDDEN_WOLF, ROLE_WEREWOLF, ROLE_WEREWOLF, ROLE_WEREWOLF,
            ROLE_BEAR, ROLE_WITCH, ROLE_HUNTER, ROLE_IDIOT,
            ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER};

        srand((unsigned int)time(NULL));

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
            players[i].faction = (players[i].role == ROLE_WEREWOLF || players[i].role == ROLE_HIDDEN_WOLF)
                                     ? FACTION_WOLF
                                     : FACTION_GOOD;
        }
    }

    // 3. GUI 發牌與查看身份流程
    UI_ClearLog();
    UI_Log("發牌完畢！接下來請依序查看身份。");
    UI_WaitContinue();

    for (int i = 1; i <= 12; i++) {
        UI_ClearLog();
        UI_Log("現在輪到【 %d 號玩家 】查看身份。", i);
        UI_Log("如果你是 %d 號玩家本人，請按「翻牌」。", i);
        UI_WaitChoice((const char *[]){"翻牌"}, 1);

        UI_ClearLog();
        UI_Log("你的身份是：");
        switch (players[i].role) {
        case ROLE_WHITE_WOLF_KING:
            UI_Log("白狼王");
            break;
        case ROLE_WEREWOLF:
            UI_Log("狼人");
            break;
        case ROLE_SEER:
            UI_Log("預言家");
            break;
        case ROLE_WITCH:
            UI_Log("女巫");
            break;
        case ROLE_GUARD:
            UI_Log("守衛");
            break;
        case ROLE_KNIGHT:
            UI_Log("騎士");
            break;
        case ROLE_HIDDEN_WOLF:
            UI_Log("隱狼");
            break;
        case ROLE_BEAR:
            UI_Log("熊");
            break;
        case ROLE_HUNTER:
            UI_Log("獵人");
            break;
        case ROLE_IDIOT:
            UI_Log("白痴");
            break;
        case ROLE_VILLAGER:
            UI_Log("平民");
            break;
        default:
            UI_Log("其他神祕身份");
            break;
        }
        UI_Log("");
        UI_Log("請確實記住你的身份！");
        UI_Log("看完後按「蓋牌」，交給下一位玩家。");
        UI_WaitChoice((const char *[]){"蓋牌"}, 1);
    }

    UI_ClearLog();
    UI_Log("所有人皆已確認身份！進入黑夜！");
    UI_WaitContinue();
}
