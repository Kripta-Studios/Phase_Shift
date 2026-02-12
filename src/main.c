#include "common.h"
#include "levels.h"
#include "logic.h"
#include "persistence.h"
#include "render.h"
#include "utils.h"

void cleanup_game(GameState *game) {
  if (game->map) {
    map_free(game->map);
  }

  for (int i = 0; i < MAX_COLAPSORES; i++) {
    if (game->colapsores[i].path) {
      path_free(game->colapsores[i].path, game->colapsores[i].path_rows);
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

  /* Load all remaining sound assets */
  guard_step_sound = LoadSound("assets/sounds/guard-step.ogg");
  open_door_sound = LoadSound("assets/sounds/open-door.wav");
  plant_bomb_sound = LoadSound("assets/sounds/plant-bomb.wav");

  /* Map unloaded externs to existing sound files */
  teleport_sound = LoadSound("assets/sounds/popup-show.wav");
  SetSoundPitch(teleport_sound, 1.3f);
  measurement_sound = LoadSound("assets/sounds/checkpoint.ogg");
  SetSoundPitch(measurement_sound, 0.8f);
  entangle_sound = LoadSound("assets/sounds/popup-show.wav");
  SetSoundPitch(entangle_sound, 0.7f);
  qubit_rotate_sound = LoadSound("assets/sounds/popup-show.wav");
  SetSoundPitch(qubit_rotate_sound, 1.5f);
  oracle_sound = LoadSound("assets/sounds/checkpoint.ogg");
  SetSoundPitch(oracle_sound, 1.2f);
  ice_slide_sound = LoadSound("assets/sounds/guard-step.ogg");
  SetSoundPitch(ice_slide_sound, 1.4f);
  mirror_reflect_sound = LoadSound("assets/sounds/blast.ogg");
  SetSoundPitch(mirror_reflect_sound, 1.8f);
  decoherence_sound = LoadSound("assets/sounds/blast.ogg");
  SetSoundPitch(decoherence_sound, 0.6f);
  portal_activate_sound = LoadSound("assets/sounds/popup-show.wav");
  SetSoundPitch(portal_activate_sound, 0.9f);
  level_complete_sound = LoadSound("assets/sounds/checkpoint.ogg");
  SetSoundPitch(level_complete_sound, 1.1f);

  ambient_music = LoadMusicStream("assets/sounds/ambient.wav");
  PlayMusicStream(ambient_music);
  SetMusicVolume(ambient_music, 1.0f);

  game_font = GetFontDefault();

  init_palette();
  init_post_shader();

  GameState game;
  memset(&game, 0, sizeof(GameState));
  game.pending_next_level = -1;

  game.state_kind = GAME_STATE_MAIN_MENU;
  init_encyclopedia(&game);
  load_game(&game);
  init_atmosphere(&game);

  load_level(&game, 0);

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

    case GAME_STATE_MAIN_MENU: {
      update_atmosphere(&game);
      update_main_menu(&game);
      break;
    }

    case GAME_STATE_PLAYING: {
      /* Pause toggle */
      if (IsKeyPressed(KEY_ESCAPE)) {
        game.state_kind = GAME_STATE_PAUSE;
        break;
      }

      if (game.player.dead) {
        if (GetTime() - game.player.death_time > 2.0) {
          game.game_over = true;
        }
      } else {
        if (check_level_complete(&game)) {
          PlaySound(level_complete_sound);
          save_game(&game);
          if (game.current_level < MAX_LEVELS - 1) {
            if (game.current_level + 1 > game.highest_level_unlocked) {
              game.highest_level_unlocked = game.current_level + 1;
            }
            game.pending_next_level = game.current_level + 1;
            game.state_kind = GAME_STATE_LEVEL_TRANSITION;
            game.level_transition_timer = 1.0f;
          } else {
            game.state_kind = GAME_STATE_WIN;
          }
        }

        /* Input - FIXED: All keys properly separated */
        update_particles(&game);
        update_atmosphere(&game);
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
          } else if (IsKeyPressed(KEY_Z)) {
            /* Z: Instant Phase Change */
            cmd.kind = CMD_PHASE_CHANGE;
            input = true;
          } else if (IsKeyPressed(KEY_SPACE)) {
            /* Space: Superposition / Create Echo */
            cmd.kind = CMD_SUPERPOSITION;
            input = true;
          } else if (IsKeyPressed(KEY_E)) {
            /* E: Interact (Teleport/Entangle) */
            cmd.kind = CMD_INTERACT;
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

          /* Encyclopedia toggle */
          if (IsKeyPressed(KEY_H)) {
            game.encyclopedia_active = !game.encyclopedia_active;
          }
          if (game.encyclopedia_active) {
            if (IsKeyPressed(KEY_RIGHT)) {
              game.encyclopedia_page++;
              if (game.encyclopedia_page >= game.encyclopedia_count)
                game.encyclopedia_page = 0;
            }
            if (IsKeyPressed(KEY_LEFT)) {
              game.encyclopedia_page--;
              if (game.encyclopedia_page < 0)
                game.encyclopedia_page = game.encyclopedia_count - 1;
            }
          }

          if (input && !game.encyclopedia_active) {
            execute_turn(&game, cmd);
          }
        }

#ifdef DEBUG_MODE
        if (IsKeyPressed(KEY_F5)) {
          if (game.current_level + 1 > game.highest_level_unlocked) {
            game.highest_level_unlocked = game.current_level + 1;
          }
          int next = game.current_level + 1;
          if (next >= MAX_LEVELS) {
            game.state_kind = GAME_STATE_WIN;
          } else {
            load_level(&game, next);
            show_level_dialog(&game);
            game.state_kind = GAME_STATE_DIALOG;
          }
        }
#endif
      }
      break;
    }

    case GAME_STATE_PAUSE: {
      update_pause_menu(&game);
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

    case GAME_STATE_LEVEL_TRANSITION: {
      if (IsKeyPressed(KEY_ENTER)) {
        if (game.pending_next_level >= 0) {
          int next = game.pending_next_level;
          game.pending_next_level = -1;
          load_level(&game, next);
          show_level_dialog(&game);
          game.state_kind = GAME_STATE_DIALOG;
        } else {
          game.state_kind = GAME_STATE_PLAYING;
        }
      }
      break;
    }
    default:
      break;
    }

    /* === RENDER WITH POST-PROCESSING SHADER === */
    if (post_shader_ready) {
      begin_post_processing();
    } else {
      BeginDrawing();
    }
    ClearBackground(PALETTE[0]);

    BeginMode2D(game.camera);

    // Apply Screen Shake
    if (game.screen_shake > 0.0f) {
      float offset_x = (float)(rand() % 10 - 5) * game.screen_shake;
      float offset_y = (float)(rand() % 10 - 5) * game.screen_shake;
      game.camera.target.x -= offset_x;
      game.camera.target.y -= offset_y;
      // Note: We modify camera temporarily? Or need to reset?
      // Raylib BeginMode2D applies the matrix. Modifying 'game.camera' here
      // persists? Yes. We should probably reset it or use a temp camera.
      // Actually, logic updates camera target to follow player.
      // If we modify it here, logic will overwrite it next frame?
      // Logic updates camera in update loop?
      // Let's check where camera is updated.
      // Usually 'update_camera(&game)'.
      // If logic overwrites it, we can modify it just before BeginMode2D and
      // rely on logic to reset it. BUT logic might run BEFORE this.
    }

    if (game.state_kind == GAME_STATE_MAIN_MENU) {
      EndMode2D();
      render_main_menu(&game);
    } else if (game.state_kind != GAME_STATE_WIN) {
      render_grid_lines(&game);
      render_exit_glow(&game);
      render_game_cells(&game);
      render_button_markers(&game);
      render_tunnels(&game);
      render_portals(&game);
      render_items(&game);
      render_oracles(&game);
      render_bombs(&game);
      render_colapsores(&game);
      render_player(&game);
      render_particles(&game);
      render_quantum_effects(&game);

      EndMode2D();

      render_dark_effects(&game);
      render_hud(&game);
      render_encyclopedia(&game);

      if (game.state_kind == GAME_STATE_PAUSE) {
        render_pause_menu(&game);
      }
    } else {
      EndMode2D();
      render_win_screen();
    }

    if (game.state_kind == GAME_STATE_DIALOG) {
      render_dialog(&game);
    }
    if (game.state_kind == GAME_STATE_LEVEL_TRANSITION) {
      render_level_transition(&game);
    }

    if (post_shader_ready) {
      end_post_processing(&game);
      /* HUD overlay rendered AFTER shader so it stays crisp */
      if (game.state_kind != GAME_STATE_MAIN_MENU) {
        render_hud(&game);
      }
      if (game.state_kind == GAME_STATE_DIALOG) {
        render_dialog(&game);
      }
      if (game.state_kind == GAME_STATE_LEVEL_TRANSITION) {
        render_level_transition(&game);
      }
    }

    EndDrawing();
  }

  unload_post_shader();
  cleanup_game(&game);

  for (int i = 0; i < 4; i++) {
    UnloadSound(footstep_sounds[i]);
  }
  UnloadSound(blast_sound);
  UnloadSound(key_pickup_sound);
  UnloadSound(bomb_pickup_sound);
  UnloadSound(checkpoint_sound);
  UnloadSound(phase_shift_sound);
  UnloadSound(guard_step_sound);
  UnloadSound(open_door_sound);
  UnloadSound(plant_bomb_sound);
  UnloadSound(teleport_sound);
  UnloadSound(measurement_sound);
  UnloadSound(entangle_sound);
  UnloadSound(qubit_rotate_sound);
  UnloadSound(oracle_sound);
  UnloadSound(ice_slide_sound);
  UnloadSound(mirror_reflect_sound);
  UnloadSound(decoherence_sound);
  UnloadSound(portal_activate_sound);
  UnloadSound(level_complete_sound);
  UnloadMusicStream(ambient_music);

  CloseAudioDevice();
  CloseWindow();

  return 0;
}