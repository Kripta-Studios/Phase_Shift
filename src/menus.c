#include "common.h"
#include "levels.h" // For load_level
#include "persistence.h"
#include "render.h"

// Ayuda: Reiniciar juego a nivel 0 (Juego Nuevo)
void start_new_game(GameState *game) {
  load_level(game, 0);      // Empezar en nivel 1 (índice 0)
  init_intro_dialogs(game); // Reiniciar diálogos si es necesario
  game->state_kind = GAME_STATE_DIALOG;
  game->game_over = false;
  // Reiniciar otras estadísticas globales si hace falta
}

// Ayuda: Continuar Juego (Cargar más alto desbloqueado)
void continue_game(GameState *game) {
  // Si no hay guardado, iniciar juego nuevo
  // Lógica: load_level puede manejar índice 0.
  // Deberíamos comprobar highest_level_unlocked primero.
  // Si highest_level_unlocked > 0, ¿cargar ese nivel?
  // Usualmente "Continuar" carga el *último* progreso, aquí "más alto
  // desbloqueado" ¿implica selección de nivel? Asumamos que Continuar retoma en
  // 'highest_level_unlocked'.

  int level_to_load = game->highest_level_unlocked;
  if (level_to_load >= MAX_LEVELS)
    level_to_load = MAX_LEVELS - 1;

  load_level(game, level_to_load);
  // ¿Quizás saltar diálogo intro si continuamos?
  // Asumamos sí, directo a jugar o diálogo de nivel.
  game->state_kind = GAME_STATE_PLAYING;
  game->game_over = false;
}

void update_main_menu(GameState *game) {
  // Lógica simple de menú
  if (IsKeyPressed(KEY_ENTER)) {
    // "Iniciar Juego" / "Continuar"
    // Por ahora, solo una acción: ¿Iniciar/Continuar basado en guardado?
    // Hagámoslo robusto:
    // Texto dice "ENTER para Empezar".
    // Si hay guardado, ¿cargarlo?
    // Siempre Continuar por defecto, o Juego Nuevo si 0.

    load_game(game); // Asegurar que tenemos últimos datos guardados
    if (game->highest_level_unlocked > 0) {
      continue_game(game);
    } else {
      start_new_game(game);
    }
  }

  // Depuración: ¿Reiniciar guardado con R?
  if (IsKeyPressed(KEY_R)) {
    // REINICIO TOTAL
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
