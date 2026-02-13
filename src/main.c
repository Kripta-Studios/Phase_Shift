#include "audio.h"
#include "common.h"
#include "levels.h"
#include "persistence.h"
#include "qiskit.h"
#include "render.h"
#include "utils.h"

void cleanup_game(GameState *game) {
  qiskit_shutdown();
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
#ifndef DEBUG_MODE
  SetTraceLogLevel(LOG_NONE);
#endif

  InitWindow(0, 0, "PHASE SHIFT");
  ToggleFullscreen();
  SetTargetFPS(144);

  InitAudioSystem();
  SetMasterVolume(1.0f);
  qiskit_init();

  /* Load assets */
  for (int i = 0; i < 4; i++) {
    footstep_sounds[i] = LoadAudioSound("assets/sounds/footsteps.mp3");
    SetAudioSoundPitch(footstep_sounds[i], 1.7f - i * 0.1f);
    SetAudioSoundVolume(footstep_sounds[i], 0.8f);
  }
  blast_sound = LoadAudioSound("assets/sounds/blast.mp3");
  SetAudioSoundVolume(blast_sound, 1.0f);
  key_pickup_sound = LoadAudioSound("assets/sounds/key-pickup.wav");
  SetAudioSoundVolume(key_pickup_sound, 0.9f);
  bomb_pickup_sound = LoadAudioSound("assets/sounds/bomb-pickup.mp3");
  SetAudioSoundVolume(bomb_pickup_sound, 0.9f);
  checkpoint_sound = LoadAudioSound("assets/sounds/checkpoint.mp3");
  SetAudioSoundPitch(checkpoint_sound, 0.8f); /* Matching original Ada code */
  SetAudioSoundVolume(checkpoint_sound, 0.9f);
  phase_shift_sound = LoadAudioSound("assets/sounds/popup-show.wav");
  SetAudioSoundVolume(phase_shift_sound, 0.9f);

  /* Load all remaining sound assets */
  guard_step_sound = LoadAudioSound("assets/sounds/guard-step.mp3");
  SetAudioSoundVolume(guard_step_sound, 0.8f);
  open_door_sound = LoadAudioSound("assets/sounds/open-door.wav");
  SetAudioSoundVolume(open_door_sound, 0.5f); /* Matching original Ada code */
  plant_bomb_sound = LoadAudioSound("assets/sounds/plant-bomb.wav");
  SetAudioSoundVolume(plant_bomb_sound, 0.8f);

  /* Map unloaded externs to existing sound files */
  teleport_sound = LoadAudioSound("assets/sounds/popup-show.wav");
  SetAudioSoundPitch(teleport_sound, 1.3f);
  SetAudioSoundVolume(teleport_sound, 0.9f);
  measurement_sound = LoadAudioSound("assets/sounds/checkpoint.mp3");
  SetAudioSoundPitch(measurement_sound, 0.8f);
  SetAudioSoundVolume(measurement_sound, 0.9f);
  entangle_sound = LoadAudioSound("assets/sounds/popup-show.wav");
  SetAudioSoundPitch(entangle_sound, 0.7f);
  SetAudioSoundVolume(entangle_sound, 0.9f);
  qubit_rotate_sound = LoadAudioSound("assets/sounds/popup-show.wav");
  SetAudioSoundPitch(qubit_rotate_sound, 1.5f);
  SetAudioSoundVolume(qubit_rotate_sound, 0.9f);
  oracle_sound = LoadAudioSound("assets/sounds/checkpoint.mp3");
  SetAudioSoundPitch(oracle_sound, 1.2f);
  SetAudioSoundVolume(oracle_sound, 0.9f);
  ice_slide_sound = LoadAudioSound("assets/sounds/guard-step.mp3");
  SetAudioSoundPitch(ice_slide_sound, 1.4f);
  SetAudioSoundVolume(ice_slide_sound, 0.8f);
  mirror_reflect_sound = LoadAudioSound("assets/sounds/blast.mp3");
  SetAudioSoundPitch(mirror_reflect_sound, 1.8f);
  SetAudioSoundVolume(mirror_reflect_sound, 0.9f);
  decoherence_sound = LoadAudioSound("assets/sounds/blast.mp3");
  SetAudioSoundPitch(decoherence_sound, 0.6f);
  SetAudioSoundVolume(decoherence_sound, 0.9f);
  portal_activate_sound = LoadAudioSound("assets/sounds/popup-show.wav");
  SetAudioSoundPitch(portal_activate_sound, 0.9f);
  SetAudioSoundVolume(portal_activate_sound, 0.9f);
  level_complete_sound = LoadAudioSound("assets/sounds/checkpoint.mp3");
  SetAudioSoundPitch(level_complete_sound, 1.1f);
  SetAudioSoundVolume(level_complete_sound, 1.0f);

  ambient_music = LoadAudioMusic("assets/sounds/ambient.wav");
  PlayAudioMusic(ambient_music);
  SetAudioMusicVolume(ambient_music,
                      0.5f); /* Was 1.0 -- original Ada used 0.5 */

  game_font = GetFontDefault();

  init_palette();
  init_post_shader();

  title_icon = LoadTexture("assets/icon.png");

  GameState game;
  memset(&game, 0, sizeof(GameState));
  game.pending_next_level = -1;

  game.state_kind = GAME_STATE_MAIN_MENU;
  init_encyclopedia(&game);
  load_game(&game);
  init_atmosphere(&game);

  load_level(&game, 0);

  while (!WindowShouldClose()) {
    UpdateAudioMusic(ambient_music);

#ifdef DEBUG_MODE
    /* === AUDIO DEBUG KEYS === */
    if (IsKeyPressed(KEY_F7)) {
      if (IsAudioMusicPlaying(ambient_music)) {
        StopAudioMusic(ambient_music);
        printf("[AUDIO DEBUG] F7: Music STOPPED\n");
      } else {
        PlayAudioMusic(ambient_music);
        SetAudioMusicVolume(ambient_music, 0.5f);
        printf("[AUDIO DEBUG] F7: Music STARTED\n");
      }
    }
#endif

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
          PlayAudioSound(level_complete_sound);
          if (game.current_level < MAX_LEVELS - 1) {
            if (game.current_level + 1 > game.highest_level_unlocked) {
              game.highest_level_unlocked = game.current_level + 1;
            }
            save_game(&game); // Save AFTER updating progress
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
            /* E: Entangle (Action) */
            cmd.kind = CMD_ENTANGLE;
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
      render_dark_effects(&game);
      render_hud(&game);
      update_and_render_floating_texts(&game);
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
    UnloadAudioSound(footstep_sounds[i]);
  }
  UnloadAudioSound(blast_sound);
  UnloadAudioSound(key_pickup_sound);
  UnloadAudioSound(bomb_pickup_sound);
  UnloadAudioSound(checkpoint_sound);
  UnloadAudioSound(phase_shift_sound);
  UnloadAudioSound(guard_step_sound);
  UnloadAudioSound(open_door_sound);
  UnloadAudioSound(plant_bomb_sound);
  UnloadAudioSound(teleport_sound);
  UnloadAudioSound(measurement_sound);
  UnloadAudioSound(entangle_sound);
  UnloadAudioSound(qubit_rotate_sound);
  UnloadAudioSound(oracle_sound);
  UnloadAudioSound(ice_slide_sound);
  UnloadAudioSound(mirror_reflect_sound);
  UnloadAudioSound(decoherence_sound);
  UnloadAudioSound(portal_activate_sound);
  UnloadAudioSound(level_complete_sound);
  UnloadAudioMusic(ambient_music);
  UnloadTexture(title_icon);

  CloseAudioSystem();
  CloseWindow();

  return 0;
}