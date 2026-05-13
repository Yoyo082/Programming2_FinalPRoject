#include "ui.h"
#include "player.h"
#include "role.h"
#include "state.h"
#include "setup.h"
#include "game_loop.h"

int main(void) {
    UI_Init("狼人殺法官模擬器");

    int play_again;
    do {
        UI_ClearLog();
        Player    players[13];
        GameState state;

        // 主選單
        UI_Log("==========================================");
        UI_Log("         歡迎來到狼人殺！         ");
        UI_Log("==========================================");
        UI_Log("[1] 白狼王騎士局");
        UI_Log("[2] 熊隱狼局");

        const char *ver_opts[] = {"[1] 白狼王騎士局", "[2] 熊隱狼局"};
        int version_choice = UI_WaitChoice(ver_opts, 2) + 1;  // 回傳 0/1 → 轉成 1/2

        initialize_game(players, &state, version_choice);

        while (!state.is_game_over) {
            run_night_phase(players, &state);
            if (state.is_game_over) break;
            run_day_phase(players, &state);
        }

        // 結果
        UI_ClearLog();
        UI_Log("==========================================");
        if (state.game_result == GAME_OVER_GOOD)
            UI_Log("        遊戲結束：好人陣營獲勝！");
        else
            UI_Log("        遊戲結束：狼人陣營獲勝！");
        UI_Log("==========================================");

        const char *again_opts[] = {"再來一局", "結束"};
        play_again = (UI_WaitChoice(again_opts, 2) == 0) ? 1 : 0;

    } while (play_again == 1);

    UI_Log("感謝遊玩，下次見！");
    UI_WaitContinue();
    UI_Close();
    return 0;
}