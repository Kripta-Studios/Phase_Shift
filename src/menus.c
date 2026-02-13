#include "common.h"
#include "levels.h" // For load_level
#include "persistence.h"
#include "render.h"

// Helper: Reset game to level 0 (New Game)
void start_new_game(GameState *game) {
  load_level(game, 0);      // Start at level 1 (index 0)
  init_intro_dialogs(game); // Reset dialogs if needed
  game->state_kind = GAME_STATE_DIALOG;
  game->game_over = false;
  // Reset other global stats if needed
}

// Helper: Continue Game (Load highest unlocked)
void continue_game(GameState *game) {
  // If no save exists, start new game
  // Logic: load_level can handle index 0.
  // We should check highest_level_unlocked first.
  // If highest_level_unlocked > 0, load that level?
  // Usually "Continue" loads the *latest* progress, here "highest unlocked"
  // implies level select? Let's assume Continue picks up at
  // 'highest_level_unlocked'.

  int level_to_load = game->highest_level_unlocked;
  if (level_to_load >= MAX_LEVELS)
    level_to_load = MAX_LEVELS - 1;

  load_level(game, level_to_load);
  // Maybe skip intro dialog if continuing?
  // Let's assume yes, directly into playing or level dialog.
  game->state_kind = GAME_STATE_PLAYING;
  game->game_over = false;
}

void update_main_menu(GameState *game) {
  // Simple Menu Logic
  if (IsKeyPressed(KEY_ENTER)) {
    // "Start Game" / "Continue"
    // For now, let's just have one action: Start/Continue based on save?
    // Let's make it robust:
    // Text says "ENTER to Start".
    // If save exists, maybe load it?
    // Let's just always Continue for now as default behavior, or New Game if 0.

    load_game(game); // Ensure we have latest save data
    if (game->highest_level_unlocked > 0) {
      continue_game(game);
    } else {
      start_new_game(game);
    }
  }

  // Debug: Reset Save with R?
  if (IsKeyPressed(KEY_R)) {
    // FULL RESET
    memset(&game->player, 0, sizeof(PlayerState));
    memset(game->encyclopedia, 0, sizeof(game->encyclopedia));
    game->highest_level_unlocked = 0;
    game->encyclopedia_count = 0;

    // Save the reset state immediately so next load sees it
    save_game(game);
    spawn_floating_text(game, (IVector2){10, 10}, "PROGRESS RESET", RED);
  }
}

void update_pause_menu(GameState *game) {
  if (IsKeyPressed(KEY_ESCAPE)) {
    // Resume
    game->state_kind = GAME_STATE_PLAYING;
  }

  if (IsKeyPressed(KEY_Q)) {
    // Quit to Main Menu
    game->state_kind = GAME_STATE_MAIN_MENU;
    // Save progress before quitting?
    save_game(game);
  }

  if (IsKeyPressed(KEY_R)) {
    // Restart Level
    load_level(game, game->current_level);
    game->state_kind = GAME_STATE_PLAYING;
  }
}
