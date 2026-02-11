#include "common.h"
#include "levels.h"
#include "logic.h"
#include "render.h"
#include "utils.h"

void cleanup_game(GameState *game) {
  if (game->map) {
    map_free(game->map);
  }

  for (int i = 0; i < MAX_EEPERS; i++) {
    if (game->eepers[i].path) {
      path_free(game->eepers[i].path, game->eepers[i].path_rows);
    }
  }
}

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(0, 0, "PHASE SHIFT");
  ToggleFullscreen();
  SetTargetFPS(144);

  InitAudioDevice();
  SetMasterVolume(1.0f);

  /* Load assets */
  for (int i = 0; i < 4; i++) {
    footstep_sounds[i] = LoadSound("assets/sounds/footsteps.mp3");
    SetSoundPitch(footstep_sounds[i], 1.7f - i * 0.1f);
  }
  blast_sound = LoadSound("assets/sounds/blast.ogg");
  key_pickup_sound = LoadSound("assets/sounds/key-pickup.wav");
  bomb_pickup_sound = LoadSound("assets/sounds/bomb-pickup.ogg");
  checkpoint_sound = LoadSound("assets/sounds/checkpoint.ogg");
  phase_shift_sound = LoadSound("assets/sounds/popup-show.wav");

  ambient_music = LoadMusicStream("assets/sounds/ambient.wav");
  PlayMusicStream(ambient_music);
  SetMusicVolume(ambient_music, 1.0f);

  game_font = GetFontDefault();

  init_palette();

  GameState game;
  memset(&game, 0, sizeof(GameState));

  load_level(&game, 0);
  init_intro_dialogs(&game);

  while (!WindowShouldClose()) {
    UpdateMusicStream(ambient_music);

    float dt = GetFrameTime();

    /* Map zoom/pan controls */
    if (IsKeyDown(KEY_EQUAL))
      game.camera.zoom += 1.0f * dt;
    if (IsKeyDown(KEY_MINUS))
      game.camera.zoom -= 1.0f * dt;
    if (game.camera.zoom < 0.1f)
      game.camera.zoom = 0.1f;

    /* Camera follow */
    Vector2 target = vec2_scale(ivec2_to_vec2(game.player.position), CELL_SIZE);
    target.x += CELL_SIZE * 0.5f;
    target.y += CELL_SIZE * 0.5f;

    game.camera.target.x += (target.x - game.camera.target.x) * dt * 4.0f;
    game.camera.target.y += (target.y - game.camera.target.y) * dt * 4.0f;

    game.camera.offset =
        (Vector2){GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};

    /* Animation update */
    if (game.turn_animation > 0.0f) {
      game.turn_animation -= dt * (1.0f / BASE_TURN_DURATION);
      if (game.turn_animation < 0.0f)
        game.turn_animation = 0.0f;
    }

    if (game.game_over) {
      if (IsKeyPressed(KEY_ENTER)) {
        load_level(&game, game.current_level);
      }
    }

    if (game.level_transition_timer > 0.0f) {
      game.level_transition_timer -= dt * 0.5f;
      if (game.level_transition_timer < 0.0f) {
        game.level_transition_timer = 0.0f;
        game.state_kind = GAME_STATE_PLAYING;
      }
    }

    switch (game.state_kind) {
    case GAME_STATE_DIALOG: {
      DialogSystem *d = &game.dialog;
      if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        d->current_page++;
        if (d->current_page >= d->page_count) {
          bool was_intro = (d->page_count > 1);
          d->active = false;
          game.state_kind = GAME_STATE_PLAYING;
          if (was_intro && game.current_level == 0 && !game.shown_level_intro) {
            show_level_dialog(&game);
            game.state_kind = GAME_STATE_DIALOG;
            game.shown_level_intro = true;
          }
        }
      }
      break;
    }

    case GAME_STATE_PLAYING: {
      if (game.player.dead) {
        if (GetTime() - game.player.death_time > 2.0) {
          game.game_over = true;
        }
      } else {
        if (check_level_complete(&game)) {
          game.state_kind = GAME_STATE_LEVEL_TRANSITION;
          game.level_transition_timer = 1.0f;
          if (game.current_level < MAX_LEVELS - 1) {
            load_level(&game, game.current_level + 1);
            show_level_dialog(&game);
            game.state_kind = GAME_STATE_DIALOG;
          } else {
            game.state_kind = GAME_STATE_WIN;
          }
        }

        /* Input - FIXED: All keys properly separated */
        if (game.turn_animation <= 0.0f) {
          Command cmd = {0};
          bool input = false;

          if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            cmd.kind = CMD_STEP;
            cmd.dir = DIR_RIGHT;
            input = true;
          } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            cmd.kind = CMD_STEP;
            cmd.dir = DIR_LEFT;
            input = true;
          } else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
            cmd.kind = CMD_STEP;
            cmd.dir = DIR_UP;
            input = true;
          } else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
            cmd.kind = CMD_STEP;
            cmd.dir = DIR_DOWN;
            input = true;
          } else if (IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_E)) {
            /* Z or E: Phase Change / Superposition */
            cmd.kind = CMD_PHASE_CHANGE;
            input = true;
          } else if (IsKeyPressed(KEY_SPACE)) {
            /* Space: Superposition */
            cmd.kind = CMD_PHASE_CHANGE;
            input = true;
          } else if (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_LEFT_SHIFT)) {
            /* X or Shift: Plant Bomb */
            cmd.kind = CMD_PLANT;
            input = true;
          } else if (IsKeyPressed(KEY_T) || IsKeyPressed(KEY_PERIOD)) {
            /* T or Period: Wait (FIXED!) */
            cmd.kind = CMD_WAIT;
            input = true;
          }

          if (input) {
            execute_turn(&game, cmd);
          }
        }
      }
      break;
    }

    case GAME_STATE_WIN: {
      if (IsKeyPressed(KEY_ENTER)) {
        game.current_level = 0;
        load_level(&game, 0);
        init_intro_dialogs(&game);
        game.state_kind = GAME_STATE_DIALOG;
      }
      break;
    }

    case GAME_STATE_LEVEL_TRANSITION:
    default:
      break;
    }

    BeginDrawing();
    ClearBackground(PALETTE[0]);

    BeginMode2D(game.camera);

    if (game.state_kind == GAME_STATE_WIN) {
      render_win_screen();
    } else {
      render_grid_lines(&game);
      render_exit_glow(&game);
      render_game_cells(&game);
      render_button_markers(&game);
      render_tunnels(&game);
      render_items(&game);
      render_bombs(&game);
      render_eepers(&game);
      render_player(&game);
      render_quantum_effects(&game);
    }

    EndMode2D();

    render_dark_effects(&game);
    render_hud(&game);

    if (game.state_kind == GAME_STATE_DIALOG) {
      render_dialog(&game);
    }
    if (game.state_kind == GAME_STATE_LEVEL_TRANSITION) {
      render_level_transition(&game);
    }

    EndDrawing();
  }

  cleanup_game(&game);

  for (int i = 0; i < 4; i++) {
    UnloadSound(footstep_sounds[i]);
  }
  UnloadSound(blast_sound);
  UnloadSound(key_pickup_sound);
  UnloadSound(bomb_pickup_sound);
  UnloadSound(checkpoint_sound);
  UnloadSound(phase_shift_sound);
  UnloadMusicStream(ambient_music);

  CloseAudioDevice();
  CloseWindow();

  return 0;
}