typedef struct {
    int current_board;      // 1: 白狼王騎士, 2: 熊隱狼
    int day_count;          // 第幾天
    int alive_wolf_count;   // 活著的狼人數 (解決妳們的 Bug!)
    bool game_over;
    int last_guarded_id;
    // 女巫藥水狀態等...
} GameState;