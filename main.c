#include "player.h"
#include "role.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#define CLEAR_CMD "cls"
#include <windows.h>

#else
#define CLEAR_CMD "clear"
#endif

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

int main()
{

#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    int play_again = 0;
    do
    {
        Player players[13];
        int last_guarded_id = -1; //-1代表還為守護過
        bool witch_has_antidote = true;
        bool witch_has_poison = true;
        int alive_wolf_count = 4;

        int version_choice = 0;

        printf("==========================================\n");
        printf("         🐺 歡迎來到狼人殺 🐺         \n");
        printf("==========================================\n");
        printf("請選擇您今天要遊玩的版本：\n");
        printf("[1] 白狼王騎士局 \n");
        printf("[2] 熊隱狼局 \n");
        printf("------------------------------------------\n");
        printf("請輸入數字 (1-2): ");

        scanf("%d", &version_choice);
        if (version_choice == 1)
        {
            RoleType role_deck[13] = {
                ROLE_VILLAGER, // 0號牌不使用
                ROLE_WHITE_WOLF_KING, ROLE_WEREWOLF, ROLE_WEREWOLF, ROLE_WEREWOLF,
                ROLE_SEER, ROLE_WITCH, ROLE_GUARD, ROLE_KNIGHT,
                ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER, ROLE_VILLAGER};

            srand(time(NULL));

            // 身份deck洗牌
            for (int i = 12; i > 1; i--)
            {
                int j = (rand() % i) + 1;

                RoleType temp = role_deck[i];
                role_deck[i] = role_deck[j];
                role_deck[j] = temp;
            }

            for (int i = 1; i <= 12; i++)
            {
                players[i].id = i;
                players[i].is_alive = true;
                players[i].can_vote = true;
                players[i].can_speak = true;
                players[i].is_knifed = false;
                players[i].is_saved = false;
                players[i].is_poisoned = false;
                players[i].is_guarded = false;

                players[i].role = role_deck[i]; // 發洗好的牌

                // 判斷是好人還是壞人
                if (players[i].role == ROLE_WEREWOLF || players[i].role == ROLE_WHITE_WOLF_KING)
                {
                    players[i].faction = FACTION_WOLF;
                }
                else
                {
                    players[i].faction = FACTION_GOOD;
                }
            }

            // 玩家輪流看牌
            printf("\n=> 法官：發牌完畢！接下來請依照順序查看身分底牌。\n");

            int dummy = 0; // 用來卡住畫面

            for (int i = 1; i <= 12; i++)
            {
                system(CLEAR_CMD);

                printf("\n==========================================\n");
                printf("       現在輪到【 %d 號玩家 】查看身分       \n", i);
                printf("==========================================\n");
                printf("如果你是 %d 號玩家本人，請輸入 1 並按 Enter 翻牌：", i);
                scanf("%d", &dummy);

                printf("\n------------------------------------------\n");
                printf("你的身分是：");
                switch (players[i].role)
                {
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
                default: printf("其他神祕身分\n"); break;
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

            bool game_over = false;
            int days = 1;

            while (!game_over)
            {

                for (int i = 1; i <= 12; i++)
                {
                    // 這些是「暫時標記」，每晚都要清空
                    players[i].is_knifed = false;
                    players[i].is_saved = false;
                    players[i].is_poisoned = false;
                    players[i].is_guarded = false;
                }

                system(CLEAR_CMD);
                printf("\n================ 第 %d 天 ================\n", days);
                printf("天黑請閉眼\n");

                // 守衛行動
                printf("\n守衛請睜眼\n");
                printf("請選擇要守護的玩家號碼 (空守請輸入 0)：");

                int guard_target;
                scanf("%d", &guard_target);

                if (guard_target >= 1 && guard_target <= 12)
                {
                    bool success = guard_protect(&players[guard_target], &last_guarded_id);

                    if (!success)
                    {
                        printf("[系統提示] 無效操作：不能連續兩晚守護同一名玩家！請重新紀錄。\n");
                    }
                }
                else
                {
                    last_guarded_id = -1;
                }

                printf("\n確認後請輸入 0 並按 Enter 閉眼：");
                scanf("%d", &dummy);
                system(CLEAR_CMD);

                // 狼人行動
                printf("\n狼人請睜眼\n");
                printf("狼人請睜眼，白狼請比大拇指（不殺人請輸入0）：");

                int wolf_target;
                scanf("%d", &wolf_target);

                if (wolf_target >= 1 && wolf_target <= 12)
                {
                    werewolf_kill(&players[wolf_target]);
                }

                printf("\n確認後請輸入 0 並按 Enter 閉眼：");
                scanf("%d", &dummy);
                system(CLEAR_CMD);

                // --- 女巫行動 ---
                printf("\n女巫請睜眼\n");

                // 尋找女巫的座位號 (witch_id)
                int witch_id = -1;
                for (int i = 1; i <= 12; i++)
                {
                    if (players[i].role == ROLE_WITCH)
                    {
                        witch_id = i;
                        break;
                    }
                }

                // 1. 如果雙藥都沒了，直接結束這回合
                if (!witch_has_antidote && !witch_has_poison)
                {
                    printf("你的解藥與毒藥皆已用盡。\n");
                }
                else
                {
                    // 2. 播報刀口 (有解藥才看得到)
                    if (witch_has_antidote)
                    {
                        if (wolf_target == 0)
                        {
                            printf("今晚平安夜，沒有人被殺死。\n");
                        }
                        else
                        {
                            printf("今晚被殺死的是 【 %d 號玩家 】。\n", wolf_target);
                        }
                    }
                    else
                    {
                        printf("你的解藥已用盡，無法得知今晚誰被殺死。\n");
                    }

                    // 3. 動態顯示選單
                    printf("\n請選擇你要進行的操作：\n");
                    if (witch_has_antidote)
                    {
                        printf("[1] 使用解藥\n");
                    }
                    if (witch_has_poison)
                    {
                        printf("[2] 使用毒藥\n");
                    }
                    printf("[0] 什麼都不做\n");
                    printf("你的選擇是：");

                    int choice;
                    scanf("%d", &choice);

                    // 4. 根據選擇執行技能與系統回饋
                    if (choice == 1 && witch_has_antidote)
                    {
                        if (wolf_target == 0)
                        {
                            printf("今晚無人死亡，無法使用解藥。\n");
                        }
                        else
                        {
                            // 呼叫 role.c 的 witch_save
                            bool success = witch_save(players, witch_id, wolf_target, &witch_has_antidote);
                            if (success)
                            {
                                printf("成功使用解藥，救活了 %d 號玩家。\n", wolf_target);
                            }
                            else
                            {
                                printf("使用解藥失敗。\n");
                            }
                        }
                    }
                    else if (choice == 2 && witch_has_poison)
                    {
                        printf("你要毒誰呢？(請輸入玩家編號)：");
                        int poison_target;
                        scanf("%d", &poison_target);

                        if (poison_target >= 1 && poison_target <= 12)
                        {
                            // 呼叫 role.c 的 witch_poison
                            bool success = witch_poison(&players[poison_target], &witch_has_poison);
                            if (success)
                            {
                                printf("成功對 %d 號玩家使用毒藥。\n", poison_target);
                            }
                        }
                        else
                        {
                            printf("無效的玩家編號。\n");
                        }
                    }
                    else if (choice == 0)
                    {
                        printf("[系統提示] 選擇保留藥水，什麼都不做。\n");
                    }
                    else
                    {
                        printf("[系統提示] 無效的輸入選項。\n");
                    }
                }

                printf("\n女巫請閉眼。確認後請輸入 0 並按 Enter 閉眼：");
                scanf("%d", &dummy);
                system(CLEAR_CMD);

                // --- 4. 預言家行動 ---
                printf("\n預言家請睜眼\n");
                printf("請輸入你要查驗的玩家號碼 (不查驗請輸入 0)：");

                int seer_target;
                scanf("%d", &seer_target);

                if (seer_target >= 1 && seer_target <= 12)
                {
                    // 呼叫 role.c 的 seer_investigate (回傳 0:好人, 1:狼人, -1:失敗)
                    int investigate_result = seer_investigate(players, seer_target, alive_wolf_count);

                    if (investigate_result == -1)
                    {
                        printf("查驗失敗！\n");
                    }
                    else if (investigate_result == FACTION_GOOD)
                    {
                        printf("這個人是 【 好人 】。\n");
                    }
                    else
                    {
                        printf("這個人是 【 狼人 】！\n");
                    }
                }
                else if (seer_target == 0)
                {
                    printf("預言家選擇不查驗。\n");
                }
                else
                {
                    printf("無效的玩家編號。\n");
                }

                printf("\n預言家請閉眼。確認後請輸入 0 並按 Enter 閉眼：");
                scanf("%d", &dummy);
                system(CLEAR_CMD);

                // ==========================================
                //                 黑夜結束，進入白天
                // ==========================================
                printf("\n================ 天亮請睜眼 ================\n");

                // 1. 呼叫死亡結算引擎
                finalize_night_results(players, 13);

                // 2. 播報昨晚的死訊
                int death_count = 0;
                int dead_ids[12]; // 用來暫存昨晚死掉的人，方便一次播報

                for (int i = 1; i <= 12; i++)
                {
                    // 檢查是否為「昨晚剛死」的人
                    if (!players[i].is_alive && (players[i].is_knifed || players[i].is_poisoned || (players[i].is_saved && players[i].is_guarded)))
                    {
                        dead_ids[death_count] = i;
                        death_count++;
                    }
                }

                // 3. 根據死亡人數決定播報內容
                if (death_count == 0)
                {
                    printf("昨晚是平安夜，無人死亡。\n");
                }
                else
                {
                    printf("昨晚死亡的是：");
                    for (int i = 0; i < death_count; i++)
                    {
                        printf("【 %d 號 】 ", dead_ids[i]);
                    }
                    printf("\n");
                }

                int win_status = check_win_condition(players);
                if (win_status != 0)
                {
                    // 如果有人贏了，印出誰贏，然後直接 break 掉 while(!game_over)
                    if (win_status == 1)
                        printf("\n好人陣營獲勝\n");
                    else
                        printf("\n狼人陣營獲勝\n");

                    game_over = true;
                    break; // 這裡一跳出去，就不會執行下面的發言環節了
                }

                printf("\n請輸入 0 繼續進入發言環節：");
                scanf("%d", &dummy);
                system(CLEAR_CMD);

                // --- 發言環節開始 ---
                bool skip_voting = false;
                

                if (win_status == 0)
                {
                    printf("\n請存活玩家按順序發言。\n");
                    printf("請問此時是否有玩家發動技能？\n");
                    printf("[1] 騎士翻牌決鬥\n");
                    printf("[2] 白狼王自爆\n");
                    printf("[0] 無人發動，進入投票\n");
                    printf("請選擇：");

                    int day_choice;
                    scanf("%d", &day_choice);

                    if (day_choice == 1)
                    { // 騎士行動
                        int k_id, target_id;
                        printf("請輸入騎士編號：");
                        scanf("%d", &k_id);
                        printf("請輸入決鬥目標：");
                        scanf("%d", &target_id);

                        // 呼叫knight_duel
                        // 回傳 true 代表決鬥狼人成功（白天結束）；false 代表決鬥好人失敗（白天繼續）
                        skip_voting = knight_duel(players, k_id, target_id);
                        win_status = check_win_condition(players);
                        if (win_status != 0)
                        {
                            if (win_status == 1)
                                printf("\n好人陣營獲勝\n");
                            else
                                printf("\n狼人陣營獲勝\n");
                            game_over = true;
                            break;
                        }

                        if (skip_voting)
                            printf("狼人出局，白天強制結束！\n");
                        else
                            printf("騎士以死謝罪，白天繼續。\n");
                    }
                    else if (day_choice == 2)
                    { // 白狼王行動
                        int wwk_id, target_id;
                        printf("請輸入白狼王編號：");
                        scanf("%d", &wwk_id);
                        printf("請輸入要帶走的目標：");
                        scanf("%d", &target_id);

                        // 呼叫 white_wolf_king_explode，回傳 true 代表成功自爆
                        skip_voting = white_wolf_king_explode(players, wwk_id, target_id);
                        win_status = check_win_condition(players);
                        if (win_status != 0)
                        {
                            if (win_status == 1)
                                printf("\n好人陣營獲勝\n");
                            else
                                printf("\n狼人陣營獲勝\n");
                            game_over = true;
                            break;
                        }
                        if (skip_voting)
                            printf("白狼王自爆，強行帶走 %d 號，白天結束！\n", target_id);
                    }

                    // 技能發動後，再次檢查勝負
                    win_status = check_win_condition(players);
                }

                // --- 投票環節 ---
                if (win_status == 0 && !skip_voting)
                {
                    printf("\n請進行公投，輸入出局者編號 (不投票請輸 0)：");
                    int vote_id;
                    scanf("%d", &vote_id);
                    if (vote_id > 0 && vote_id < 13)
                    {
                        players[vote_id].is_alive = false;
                        printf("%d 號玩家被公投出局。\n", vote_id);

                        win_status = check_win_condition(players);
                        if (win_status != 0)
                        {
                            if (win_status == 1)
                                printf("\n好人陣營獲勝\n");
                            else
                                printf("\n狼人陣營獲勝\n");
                            game_over = true;
                            break;
                        }
                    }
                    // 投票完，檢查勝負
                    win_status = check_win_condition(players);
                }

                // --- 判斷這一局是否結束 ---
                if (win_status != 0)
                {
                    if (win_status == 1)
                        printf("\n好人陣營獲勝\n");
                    else
                        printf("\n狼人陣營獲勝\n");
                    game_over = true; // 準備跳出 while 迴圈
                }
                else
                {
                    printf("\n請輸入 0 準備進入黑夜...");
                    scanf("%d", &dummy);
                    days++;
                }
            }
            printf("\n==========================================\n");
            printf("遊戲結束！要再玩一局嗎？\n");
            printf("[1] 再來一局 (重新選版本/發牌)\n");
            printf("[0] 結束程式\n");
            printf("請選擇：");
            scanf("%d", &play_again);
            system(CLEAR_CMD);
        }
    } while (play_again == 1);

    printf("感謝遊玩，下次見！\n");
    return 0;
}