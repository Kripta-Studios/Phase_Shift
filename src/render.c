#include "render.h"
#include <stdio.h> // for sprintf

/* === POST-PROCESSING SHADER SYSTEM === */
Shader post_shader = {0};
RenderTexture2D post_target = {0};
bool post_shader_ready = false;

static int loc_time = -1;
static int loc_resolution = -1;

// Interference Shader
Shader interference_shader = {0};
static int loc_int_time = -1;
static int loc_int_res = -1;

void init_interference_shader(void) {
  interference_shader = LoadShader(0, "assets/shaders/interference.fs");
  if (interference_shader.id > 0) {
    loc_int_time = GetShaderLocation(interference_shader, "time");
    loc_int_res = GetShaderLocation(interference_shader, "resolution");
  }
}

void init_post_shader(void) {
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  if (sw <= 0)
    sw = SCREEN_WIDTH;
  if (sh <= 0)
    sh = SCREEN_HEIGHT;

  post_target = LoadRenderTexture(sw, sh);
  post_shader = LoadShader(0, "assets/shaders/quantum_glow.fs");

  if (post_shader.id > 0) {
    loc_time = GetShaderLocation(post_shader, "time");
    loc_resolution = GetShaderLocation(post_shader, "resolution");
    post_shader_ready = true;
  } else {
    post_shader_ready = false;
  }

  // Load Interference Shader
  init_interference_shader();
}

void unload_post_shader(void) {
  if (post_shader_ready) {
    UnloadShader(post_shader);
    UnloadRenderTexture(post_target);
    post_shader_ready = false;
  }
  if (interference_shader.id > 0) {
    UnloadShader(interference_shader);
  }
}

void begin_post_processing(void) {
  if (!post_shader_ready)
    return;

  /* Handle window resize */
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  if (sw != post_target.texture.width || sh != post_target.texture.height) {
    UnloadRenderTexture(post_target);
    post_target = LoadRenderTexture(sw, sh);
  }

  BeginTextureMode(post_target);
  ClearBackground((Color){5, 5, 12, 255});
}

void end_post_processing(GameState *game) {
  if (!post_shader_ready)
    return;
  EndTextureMode();

  float t = (float)GetTime();
  float res[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
  SetShaderValue(post_shader, loc_time, &t, SHADER_UNIFORM_FLOAT);
  SetShaderValue(post_shader, loc_resolution, res, SHADER_UNIFORM_VEC2);

  BeginDrawing();
  ClearBackground(BLACK);
  BeginShaderMode(post_shader);
  /* Draw render texture flipped vertically */
  DrawTextureRec(post_target.texture,
                 (Rectangle){0, 0, (float)post_target.texture.width,
                             -(float)post_target.texture.height},
                 (Vector2){0, 0}, WHITE);
  EndShaderMode();
}

Vector2 interpolate_positions(IVector2 prev, IVector2 curr, float t) {
  Vector2 p = vec2_scale(ivec2_to_vec2(prev), CELL_SIZE);
  Vector2 c = vec2_scale(ivec2_to_vec2(curr), CELL_SIZE);
  float factor = 1.0f - t * t;
  return Vector2LerpCustom(p, c, factor);
}

void draw_eyes(Vector2 start, Vector2 size, float angle, EyesKind kind) {
  Vector2 dir = {cosf(angle), sinf(angle)};
  Vector2 eyes_size = {size.x * 0.2f, size.y * 0.35f};
  Vector2 center = {start.x + size.x * 0.5f, start.y + size.y * 0.5f};
  Vector2 position = {center.x + dir.x * eyes_size.x * 0.6f,
                      center.y + dir.y * eyes_size.x * 0.6f};

  Vector2 left_eye = {position.x - eyes_size.x * 0.5f,
                      position.y - eyes_size.y * 0.5f};
  Vector2 right_eye = {position.x + eyes_size.x * 0.5f,
                       position.y - eyes_size.y * 0.5f};

  float eye_height = eyes_size.y;
  if (kind == EYES_CLOSED) {
    eye_height = eyes_size.y * 0.2f;
  } else if (kind == EYES_SURPRISED) {
    eye_height = eyes_size.y * 1.3f;
  }

  DrawRectangleV(left_eye, (Vector2){eyes_size.x, eye_height}, PALETTE[13]);
  DrawRectangleV(right_eye, (Vector2){eyes_size.x, eye_height}, PALETTE[13]);
}

void render_game_cells(GameState *game) {
  PhaseKind phase = game->player.phase_system.current_phase;
  bool in_superposition =
      game->player.phase_system.state == PHASE_STATE_SUPERPOSITION;

  for (int y = 0; y < game->map->rows; y++) {
    for (int x = 0; x < game->map->cols; x++) {
      Vector2 pos = {x * CELL_SIZE, y * CELL_SIZE};
      Cell cell = game->map->data[y][x];
      Color color = get_cell_color(cell, phase, in_superposition);
      DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE}, color);

      /* Overlay for special cells */
      if (cell == CELL_ICE) {
        DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE},
                       (Color){0, 255, 255, 50});
        DrawLine(pos.x, pos.y, pos.x + CELL_SIZE, pos.y + CELL_SIZE,
                 (Color){200, 255, 255, 100});
      } else if (cell == CELL_MIRROR) {
        DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE},
                       (Color){200, 200, 220, 255});
        DrawLineEx((Vector2){pos.x, pos.y + CELL_SIZE},
                   (Vector2){pos.x + CELL_SIZE, pos.y}, 3.0f, WHITE);
        DrawLineEx((Vector2){pos.x, pos.y + CELL_SIZE},
                   (Vector2){pos.x + CELL_SIZE, pos.y}, 1.0f,
                   (Color){0, 0, 0, 100});
      } else if (cell >= CELL_ONEWAY_UP && cell <= CELL_ONEWAY_RIGHT) {
        Vector2 center = {pos.x + CELL_SIZE * 0.5f, pos.y + CELL_SIZE * 0.5f};
        Color arrow_col = (Color){255, 255, 255, 150};
        float offset = CELL_SIZE * 0.25f;

        if (cell == CELL_ONEWAY_UP) {
          DrawTriangle((Vector2){center.x, center.y - offset},
                       (Vector2){center.x - offset / 2, center.y + offset},
                       (Vector2){center.x + offset / 2, center.y + offset},
                       arrow_col);
        } else if (cell == CELL_ONEWAY_DOWN) {
          DrawTriangle((Vector2){center.x, center.y + offset},
                       (Vector2){center.x + offset / 2, center.y - offset},
                       (Vector2){center.x - offset / 2, center.y - offset},
                       arrow_col);
        } else if (cell == CELL_ONEWAY_LEFT) {
          DrawTriangle((Vector2){center.x - offset, center.y},
                       (Vector2){center.x + offset, center.y + offset / 2},
                       (Vector2){center.x + offset, center.y - offset / 2},
                       arrow_col);
        } else if (cell == CELL_ONEWAY_RIGHT) {
          DrawTriangle((Vector2){center.x + offset, center.y},
                       (Vector2){center.x - offset, center.y - offset / 2},
                       (Vector2){center.x - offset, center.y + offset / 2},
                       arrow_col);
        }
      } else if (cell == CELL_DECOHERENCE_ZONE) {
        // Purple semi-transparent overlay with random noise in future?
        DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE},
                       Fade(PURPLE, 0.3f));
        // Draw some "static" lines/points
        for (int i = 0; i < 3; i++) {
          int offX = rand() % (int)CELL_SIZE;
          int offY = rand() % (int)CELL_SIZE;
          DrawPixel(pos.x + offX, pos.y + offY, PURPLE);
        }
      } else if (cell == CELL_MEASUREMENT_ZONE) {
        // White grid or eye
        DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE}, Fade(WHITE, 0.1f));
        DrawRectangleLines(pos.x + 4, pos.y + 4, CELL_SIZE - 8, CELL_SIZE - 8,
                           Fade(WHITE, 0.5f));
        // Eye symbol
        DrawCircle(pos.x + CELL_SIZE / 2, pos.y + CELL_SIZE / 2, CELL_SIZE / 4,
                   Fade(BLACK, 0.5f));
        DrawCircle(pos.x + CELL_SIZE / 2, pos.y + CELL_SIZE / 2, CELL_SIZE / 6,
                   WHITE);
      }
    }
  }
}

void render_items(GameState *game) {
  for (int i = 0; i < MAX_ITEMS; i++) {
    Item *item = &game->items[i];
    if (item->kind == ITEM_NONE)
      continue;

    Vector2 pos = vec2_scale(ivec2_to_vec2(item->position), CELL_SIZE);
    Vector2 center = {pos.x + CELL_SIZE * 0.5f, pos.y + CELL_SIZE * 0.5f};

    switch (item->kind) {
    case ITEM_KEY:
      DrawCircleV(center, CELL_SIZE * 0.25f, PALETTE[4]);
      break;
    case ITEM_BOMB_REFILL:
      if (item->cooldown > 0) {
        DrawCircleV(center, CELL_SIZE * 0.5f,
                    ColorBrightness(PALETTE[6], -0.5f));
      } else {
        DrawCircleV(center, CELL_SIZE * 0.5f, PALETTE[6]);
      }
      break;
    case ITEM_CHECKPOINT:
      DrawRectangleV(
          (Vector2){center.x - CELL_SIZE * 0.25f, center.y - CELL_SIZE * 0.25f},
          (Vector2){CELL_SIZE * 0.5f, CELL_SIZE * 0.5f}, PALETTE[10]);
      break;
    case ITEM_COHERENCE_PICKUP:
      DrawCircleV(center, CELL_SIZE * 0.3f, PALETTE[13]);
      break;
    case ITEM_STABILIZER:
      DrawRectangleV(
          (Vector2){center.x - CELL_SIZE * 0.3f, center.y - CELL_SIZE * 0.3f},
          (Vector2){CELL_SIZE * 0.6f, CELL_SIZE * 0.6f}, PALETTE[5]);
      break;
    default:
      break;
    }
  }
}

void render_player(GameState *game) {
  Vector2 pos;
  if (game->turn_animation > 0.0f) {
    pos = interpolate_positions(game->player.prev_position,
                                game->player.position, game->turn_animation);
  } else {
    pos = vec2_scale(ivec2_to_vec2(game->player.position), CELL_SIZE);
  }

  DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE}, PALETTE[5]);
  draw_eyes(pos, (Vector2){CELL_SIZE, CELL_SIZE}, game->player.eyes_angle,
            game->player.eyes);
}

void render_colapsores(GameState *game) {
  for (int i = 0; i < MAX_COLAPSORES; i++) {
    ColapsarState *colapsor = &game->colapsores[i];
    if (colapsor->dead)
      continue;

    Vector2 pos;
    if (game->turn_animation > 0.0f) {
      pos = interpolate_positions(colapsor->prev_position, colapsor->position,
                                  game->turn_animation);
    } else {
      pos = vec2_scale(ivec2_to_vec2(colapsor->position), CELL_SIZE);
    }

    Vector2 size = vec2_scale(ivec2_to_vec2(colapsor->size), CELL_SIZE);

    switch (colapsor->kind) {
    case COLAPSOR_GUARD:
      DrawRectangleV(pos, size, PALETTE[8]);
      if (colapsor->health < 1.0f) {
        Vector2 health_bar_pos = {pos.x, pos.y - 15.0f};
        DrawRectangleV(health_bar_pos,
                       (Vector2){size.x * colapsor->health, 10.0f},
                       PALETTE[12]);
      }
      draw_eyes(pos, size, colapsor->eyes_angle, colapsor->eyes);
      break;
    case COLAPSOR_GNOME: {
      Vector2 gnome_size = {size.x * 0.7f, size.y * 0.7f};
      Vector2 gnome_pos = {pos.x + (size.x - gnome_size.x) * 0.5f,
                           pos.y + (size.y - gnome_size.y) * 0.5f};
      DrawRectangleV(gnome_pos, gnome_size, PALETTE[9]);
      draw_eyes(gnome_pos, gnome_size, colapsor->eyes_angle, colapsor->eyes);
      break;
    }
    default:
      break;
    }
  }
}

void render_bombs(GameState *game) {
  for (int i = 0; i < MAX_BOMBS; i++) {
    if (game->bombs[i].countdown > 0) {
      Vector2 pos =
          vec2_scale(ivec2_to_vec2(game->bombs[i].position), CELL_SIZE);
      Vector2 center = {pos.x + CELL_SIZE * 0.5f, pos.y + CELL_SIZE * 0.5f};
      DrawCircleV(center, CELL_SIZE * 0.5f, PALETTE[6]);

      char text[4];
      sprintf(text, "%d", game->bombs[i].countdown);
      int text_width = MeasureText(text, 32);
      DrawText(text, (int)(center.x - text_width * 0.5f), (int)(center.y - 16),
               32, PALETTE[7]);
    }
  }
}

void render_quantum_effects(GameState *game) {
  float time = (float)GetTime();

  if (interference_shader.id > 0) {
    float res[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    SetShaderValue(interference_shader, loc_int_time, &time,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(interference_shader, loc_int_res, res, SHADER_UNIFORM_VEC2);
    BeginShaderMode(interference_shader);
  }
  for (int i = 0; i < MAX_ECHOS; i++) {
    QuantumEcho *echo = &game->echos[i];
    if (!echo->active)
      continue;

    Vector2 pos = vec2_scale(ivec2_to_vec2(echo->position), CELL_SIZE);
    Color echo_color = (echo->phase == PHASE_RED) ? (Color){255, 50, 50, 255}
                                                  : (Color){50, 150, 255, 255};
    echo_color.a = (unsigned char)(echo->opacity * 255.0f);

    DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE}, echo_color);

    for (int j = 1; j <= 3; j++) {
      Vector2 trail = {pos.x + CELL_SIZE * 0.5f - j * 2.0f,
                       pos.y + CELL_SIZE * 0.5f - j * 2.0f};
      Color trail_color = echo_color;
      trail_color.a = (unsigned char)(echo->opacity * 128.0f / (j * 2));
      DrawCircleV(trail, 3.0f, trail_color);
    }
  }

  if (game->player.phase_system.state == PHASE_STATE_SUPERPOSITION) {
    Vector2 pos = vec2_scale(ivec2_to_vec2(game->player.position), CELL_SIZE);

    DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE},
                   (Color){255, 0, 0, 100});
    DrawRectangleV((Vector2){pos.x + 2, pos.y + 2},
                   (Vector2){CELL_SIZE, CELL_SIZE}, (Color){0, 100, 255, 100});

    double time = GetTime();
    for (int i = 0; i < 8; i++) {
      float angle = (float)i * (2.0f * PI / 8.0f) + (float)time * 5.0f;
      float radius = 30.0f;
      Vector2 particle = {pos.x + CELL_SIZE * 0.5f + cosf(angle) * radius,
                          pos.y + CELL_SIZE * 0.5f + sinf(angle) * radius};
      DrawCircleV(particle, 3.0f, (Color){255, 255, 0, 200});
    }
  }

  /* Draw Beams */
  for (int i = 0; i < MAX_DETECTORS; i++) {
    QuantumDetector *det = &game->detectors[i];
    if (!det->is_active)
      continue;

    // Draw Beam
    if (det->view_distance > 0) {
      IVector2 start = det->position;
      IVector2 end =
          ivec2_add(start, ivec2_scale(DIRECTION_VECTORS[det->direction],
                                       det->view_distance));

      Vector2 p1 = vec2_scale(ivec2_to_vec2(start), CELL_SIZE);
      Vector2 p2 = vec2_scale(ivec2_to_vec2(end), CELL_SIZE);
      Vector2 center_offset = {CELL_SIZE / 2, CELL_SIZE / 2};
      p1 = vec2_add(p1, center_offset);
      p2 = vec2_add(p2, center_offset);

      Color beam_col = (det->detects_phase == PHASE_RED) ? RED : BLUE;
      if (det->detects_phase == PHASE_GREEN)
        beam_col = GREEN;
      if (det->detects_phase == PHASE_YELLOW)
        beam_col = YELLOW;

      beam_col.a = (unsigned char)(100 + sinf(time * 10.0f) * 50);
      DrawLineEx(p1, p2, 4.0f + sinf(time * 5.0f) * 2.0f, beam_col);
    }

    // Draw Base
    Vector2 start = vec2_scale(ivec2_to_vec2(det->position), CELL_SIZE);
    Vector2 center = {start.x + CELL_SIZE * 0.5f, start.y + CELL_SIZE * 0.5f};
    DrawCircleV(center, 5.0f, (Color){100, 100, 100, 255});
    Color eye_col = (det->detects_phase == PHASE_RED) ? RED : BLUE;
    if (det->detects_phase == PHASE_GREEN)
      eye_col = GREEN;
    if (det->detects_phase == PHASE_YELLOW)
      eye_col = YELLOW;
    DrawCircleV(center, 3.0f, eye_col);
  }

  if (interference_shader.id > 0) {
    EndShaderMode();
  }

  if (game->glitch_intensity > 0.01f) {
    int glitch_lines = (int)(game->glitch_intensity * 10.0f);
    for (int i = 0; i < glitch_lines; i++) {
      int y_pos = rand() % SCREEN_HEIGHT;
      DrawRectangle(
          0, y_pos, SCREEN_WIDTH, 2,
          (Color){255, 0, 0, (unsigned char)(game->glitch_intensity * 100)});
    }
  }
}

void render_hud(GameState *game) {
  for (int i = 0; i < game->player.keys; i++) {
    DrawCircleV((Vector2){100.0f + i * CELL_SIZE, 100.0f}, CELL_SIZE * 0.25f,
                PALETTE[4]);
  }

  for (int i = 0; i < game->player.bomb_slots; i++) {
    float x = 100.0f + i * (CELL_SIZE + CELL_SIZE * 0.5f);
    Color bomb_color = (i < game->player.bombs)
                           ? PALETTE[6]
                           : ColorBrightness(PALETTE[6], -0.5f);
    DrawCircleV((Vector2){x, 200.0f}, CELL_SIZE * 0.5f, bomb_color);
  }

  float bar_width = 300.0f;
  float bar_height = 30.0f;
  float bar_x = 50.0f;
  float bar_y = 50.0f;
  float fill_width = bar_width * (game->player.coherence.current / 100.0f);

  Color bar_color;
  if (game->player.coherence.current > 80.0f) {
    bar_color = GREEN;
  } else if (game->player.coherence.current > 50.0f) {
    bar_color = YELLOW;
  } else {
    bar_color = RED;
  }

  DrawRectangle((int)bar_x, (int)bar_y, (int)bar_width, (int)bar_height,
                (Color){50, 50, 50, 200});
  DrawRectangle((int)bar_x, (int)bar_y, (int)fill_width, (int)bar_height,
                bar_color);
  DrawRectangleLines((int)bar_x, (int)bar_y, (int)bar_width, (int)bar_height,
                     WHITE);

  char coherence_text[50];
  sprintf(coherence_text, "COHERENCE: %d%%",
          (int)game->player.coherence.current);
  DrawTextEx(game_font, coherence_text, (Vector2){bar_x + 10, bar_y + 5}, 24, 2,
             WHITE);

  const char *phase_text;
  Color phase_color;
  switch (game->player.phase_system.current_phase) {
  case PHASE_RED:
    phase_text = "PHASE: RED";
    phase_color = (Color){255, 100, 100, 255};
    break;
  case PHASE_BLUE:
    phase_text = "PHASE: BLUE";
    phase_color = (Color){100, 100, 255, 255};
    break;
  case PHASE_GREEN:
    phase_text = "PHASE: GREEN";
    phase_color = PALETTE[20];
    break;
  case PHASE_YELLOW:
    phase_text = "PHASE: YELLOW";
    phase_color = PALETTE[21];
    break;
  default:
    phase_text = "PHASE: UNKNOWN";
    phase_color = WHITE;
    break;
  }

  DrawTextEx(game_font, phase_text,
             (Vector2){(float)(GetScreenWidth() - 220), 50}, 24, 2,
             phase_color);

  if (game->player.phase_system.state == PHASE_STATE_SUPERPOSITION) {
    char super_text[50];
    sprintf(super_text, "GRABANDO ECO: %d",
            game->player.phase_system.superposition_turns_left);
    DrawTextEx(game_font, super_text,
               (Vector2){(float)(GetScreenWidth() - 320), 80}, 24, 2, YELLOW);

    if (game->player.is_recording_echo) {
      /* Always show the prompt so user knows it exists */
      DrawTextEx(game_font, "PULSA [T] PARA ESPERAR",
                 (Vector2){(float)(GetScreenWidth() - 320), 112}, 22, 2,
                 (Color){200, 200, 200, 200});

      if (game->player.recording_frame > 0) {
        EchoAction *last =
            &game->player.current_recording[game->player.recording_frame - 1];
        if (last->action.kind == CMD_WAIT) {
          const char *wait_text = "ESPERANDO... (GRABANDO)";
          Vector2 wsz = MeasureTextEx(game_font, wait_text, 28, 2);
          DrawTextEx(game_font, wait_text,
                     (Vector2){(GetScreenWidth() - wsz.x) / 2,
                               (float)(GetScreenHeight() / 2 - 50)},
                     28, 2, (Color){255, 255, 0, 255});
        }
      }
    }
  }

  if (game->player.dead) {
    const char *death_text = "FUNCION DE ONDA COLAPSADA";
    Vector2 dtsz = MeasureTextEx(game_font, death_text, 48, 2);
    DrawTextEx(game_font, death_text,
               (Vector2){GetScreenWidth() / 2 - dtsz.x / 2 + 2,
                         (float)(GetScreenHeight() / 2 - 24 + 2)},
               48, 2, (Color){0, 0, 0, 200});
    DrawTextEx(game_font, death_text,
               (Vector2){GetScreenWidth() / 2 - dtsz.x / 2,
                         (float)(GetScreenHeight() / 2 - 24)},
               48, 2, PALETTE[12]);
    const char *sub = "PULSA ENTER PARA REINICIAR";
    Vector2 ssz = MeasureTextEx(game_font, sub, 24, 2);
    DrawTextEx(game_font, sub,
               (Vector2){GetScreenWidth() / 2 - ssz.x / 2,
                         (float)(GetScreenHeight() / 2 + 40)},
               24, 2, (Color){150, 150, 170, 200});
  }

  /* Level indicator */
  char level_text[32];
  snprintf(level_text, 32, "LEVEL %d / %d", game->current_level + 1,
           MAX_LEVELS);
  DrawTextEx(game_font, level_text,
             (Vector2){(float)(GetScreenWidth() - 220), 145}, 22, 2,
             (Color){120, 140, 180, 200});
}

void render_grid_lines(GameState *game) {
  for (int y = 0; y < game->map->rows; y++) {
    for (int x = 0; x < game->map->cols; x++) {
      if (game->map->data[y][x] == CELL_FLOOR) {
        Vector2 pos = {x * CELL_SIZE, y * CELL_SIZE};
        DrawRectangleLinesEx((Rectangle){pos.x, pos.y, CELL_SIZE, CELL_SIZE},
                             0.5f, PALETTE[16]);
      }
    }
  }
}

void render_exit_glow(GameState *game) {
  if (game->exit_position.x < 0)
    return;
  Vector2 pos = vec2_scale(ivec2_to_vec2(game->exit_position), CELL_SIZE);
  float t = (float)GetTime();
  float pulse = (sinf(t * 3.0f) + 1.0f) * 0.5f;
  float r = CELL_SIZE * (0.8f + pulse * 0.4f);
  DrawCircleV((Vector2){pos.x + CELL_SIZE * 0.5f, pos.y + CELL_SIZE * 0.5f}, r,
              (Color){0, 255, 120, (unsigned char)(30 + pulse * 40)});
  DrawCircleV((Vector2){pos.x + CELL_SIZE * 0.5f, pos.y + CELL_SIZE * 0.5f},
              r * 0.6f, (Color){0, 255, 120, (unsigned char)(50 + pulse * 50)});
}

void render_button_markers(GameState *game) {
  for (int i = 0; i < MAX_BUTTONS; i++) {
    PressureButton *b = &game->buttons[i];
    if (!b->is_active)
      continue;
    Vector2 pos = vec2_scale(ivec2_to_vec2(b->position), CELL_SIZE);
    Vector2 center = {pos.x + CELL_SIZE * 0.5f, pos.y + CELL_SIZE * 0.5f};
    Color col = (b->phase == PHASE_RED) ? PALETTE[17] : PALETTE[18];
    if (b->is_pressed) {
      DrawCircleV(center, CELL_SIZE * 0.35f, col);
    } else {
      DrawCircleLinesV(center, CELL_SIZE * 0.35f, col);
      float t = (float)GetTime();
      float pulse = (sinf(t * 4.0f) + 1.0f) * 0.5f;
      col.a = (unsigned char)(80 + pulse * 80);
      DrawCircleV(center, CELL_SIZE * 0.2f, col);
    }
  }
}

void render_vignette(void) {
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  int border = 150;
  DrawRectangleGradientV(0, 0, sw, border, (Color){0, 0, 0, 180},
                         (Color){0, 0, 0, 0});
  DrawRectangleGradientV(0, sh - border, sw, border, (Color){0, 0, 0, 0},
                         (Color){0, 0, 0, 180});
  DrawRectangleGradientH(0, 0, border, sh, (Color){0, 0, 0, 150},
                         (Color){0, 0, 0, 0});
  DrawRectangleGradientH(sw - border, 0, border, sh, (Color){0, 0, 0, 0},
                         (Color){0, 0, 0, 150});
}

void render_scanlines(void) {
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  for (int y = 0; y < sh; y += 4) {
    DrawRectangle(0, y, sw, 1, (Color){0, 0, 0, 15});
  }
}

void render_phase_tint(GameState *game) {
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  if (game->player.phase_system.state == PHASE_STATE_SUPERPOSITION) {
    DrawRectangle(0, 0, sw, sh, (Color){140, 80, 200, 15});
  } else if (game->player.phase_system.current_phase == PHASE_RED) {
    DrawRectangle(0, 0, sw, sh, (Color){255, 60, 40, 8});
  } else if (game->player.phase_system.current_phase == PHASE_BLUE) {
    DrawRectangle(0, 0, sw, sh, (Color){40, 120, 255, 8});
  } else if (game->player.phase_system.current_phase == PHASE_GREEN) {
    DrawRectangle(0, 0, sw, sh, (Color){50, 255, 50, 8});
  } else if (game->player.phase_system.current_phase == PHASE_YELLOW) {
    DrawRectangle(0, 0, sw, sh, (Color){255, 255, 0, 8});
  }
}

void render_dark_effects(GameState *game) {
  render_phase_tint(game);
  render_scanlines();
  render_vignette();
}

void render_dialog(GameState *game) {
  DialogSystem *d = &game->dialog;
  if (!d->active)
    return;

  int sw = GetScreenWidth();
  int sh = GetScreenHeight();

  DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 200});

  int box_w = 700;
  int box_h = 680;
  int box_x = (sw - box_w) / 2;
  int box_y = (sh - box_h) / 2;

  DrawRectangle(box_x - 4, box_y - 4, box_w + 8, box_h + 8,
                (Color){80, 200, 255, 80});
  DrawRectangle(box_x - 2, box_y - 2, box_w + 4, box_h + 4,
                (Color){40, 180, 255, 150});
  DrawRectangle(box_x, box_y, box_w, box_h, (Color){15, 18, 28, 245});

  int corner = 12;
  DrawRectangle(box_x, box_y, corner, 2, PALETTE[18]);
  DrawRectangle(box_x, box_y, 2, corner, PALETTE[18]);
  DrawRectangle(box_x + box_w - corner, box_y, corner, 2, PALETTE[18]);
  DrawRectangle(box_x + box_w - 2, box_y, 2, corner, PALETTE[18]);
  DrawRectangle(box_x, box_y + box_h - 2, corner, 2, PALETTE[18]);
  DrawRectangle(box_x, box_y + box_h - corner, 2, corner, PALETTE[18]);
  DrawRectangle(box_x + box_w - corner, box_y + box_h - 2, corner, 2,
                PALETTE[18]);
  DrawRectangle(box_x + box_w - 2, box_y + box_h - corner, 2, corner,
                PALETTE[18]);

  DialogPage *page = &d->pages[d->current_page];
  Vector2 tsz = MeasureTextEx(game_font, page->title, 32, 2);
  DrawTextEx(game_font, page->title,
             (Vector2){(sw - tsz.x) / 2, (float)(box_y + 18)}, 32, 2,
             PALETTE[18]);

  DrawRectangle(box_x + 30, box_y + 55, box_w - 60, 1,
                (Color){80, 200, 255, 100});

  char buf[MAX_DIALOG_TEXT];
  strncpy(buf, page->text, MAX_DIALOG_TEXT - 1);
  buf[MAX_DIALOG_TEXT - 1] = '\0';
  int text_y = box_y + 70;
  char *line = strtok(buf, "\n");
  while (line) {
    DrawTextEx(game_font, line, (Vector2){(float)(box_x + 30), (float)text_y},
               22, 2, PALETTE[7]);
    text_y += 28;
    line = strtok(NULL, "\n");
  }

  char page_text[32];
  snprintf(page_text, 32, "%d / %d", d->current_page + 1, d->page_count);
  Vector2 psz = MeasureTextEx(game_font, page_text, 20, 2);
  DrawTextEx(game_font, page_text,
             (Vector2){(sw - psz.x) / 2, (float)(box_y + box_h - 32)}, 20, 2,
             (Color){120, 140, 160, 200});

  float pulse = (sinf((float)GetTime() * 4.0f) + 1.0f) * 0.5f;
  unsigned char alpha = (unsigned char)(150 + pulse * 105);
  const char *prompt = (d->current_page < d->page_count - 1)
                           ? "ENTER to continue >>"
                           : "ENTER to begin >>";
  Vector2 prsz = MeasureTextEx(game_font, prompt, 22, 2);
  DrawTextEx(game_font, prompt,
             (Vector2){sw / 2 - prsz.x / 2, (float)(box_y + box_h + 15)}, 22, 2,
             (Color){80, 200, 255, alpha});
}

void render_win_screen(void) {
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 220});

  const char *title = "STABILIZATION COMPLETE";
  Vector2 wtsz = MeasureTextEx(game_font, title, 56, 3);
  DrawTextEx(game_font, title,
             (Vector2){(sw - wtsz.x) / 2, (float)(sh / 2 - 80)}, 56, 3,
             (Color){0, 255, 180, 255});

  const char *sub = "Subject 44's wave function has been stabilized.";
  Vector2 wssz = MeasureTextEx(game_font, sub, 26, 2);
  DrawTextEx(game_font, sub, (Vector2){(sw - wssz.x) / 2, (float)(sh / 2 - 20)},
             26, 2, (Color){180, 200, 220, 200});

  const char *credits = "PHASE SHIFT -- Kipta-Studios";
  Vector2 wcsz = MeasureTextEx(game_font, credits, 22, 2);
  DrawTextEx(game_font, credits,
             (Vector2){(sw - wcsz.x) / 2, (float)(sh / 2 + 40)}, 22, 2,
             (Color){100, 140, 180, 150});

  float pulse = (sinf((float)GetTime() * 3.0f) + 1.0f) * 0.5f;
  unsigned char a = (unsigned char)(120 + pulse * 135);
  const char *restart = "Press ENTER to restart";
  Vector2 wrsz = MeasureTextEx(game_font, restart, 24, 2);
  DrawTextEx(game_font, restart,
             (Vector2){(sw - wrsz.x) / 2, (float)(sh / 2 + 100)}, 24, 2,
             (Color){80, 200, 255, a});
}

void render_tunnels(GameState *game) {
  for (int i = 0; i < MAX_TUNNELS; i++) {
    QuantumTunnel *t = &game->tunnels[i];
    if (t->position.x == 0 && t->position.y == 0 && t->size.x == 0)
      continue;

    Vector2 pos = vec2_scale(ivec2_to_vec2(t->position), CELL_SIZE);
    Vector2 size = vec2_scale(ivec2_to_vec2(t->size), CELL_SIZE);

    /* More visible tunnel: Purple semi-transparent fill with pulsing border */
    DrawRectangleV(pos, size, (Color){180, 50, 220, 100});

    float pulse = (sinf((float)GetTime() * 5.0f) + 1.0f) * 0.5f;
    Color border = {200, 100, 255, (unsigned char)(150 + pulse * 105)};
    DrawRectangleLinesEx((Rectangle){pos.x, pos.y, size.x, size.y}, 3.0f,
                         border);

    DrawTextEx(game_font, "TUNNEL", (Vector2){pos.x + 5, pos.y + 5}, 14, 2,
               border);
    Vector2 center = {pos.x + size.x * 0.5f, pos.y + size.y * 0.5f};
    float time = (float)GetTime();
    float radius = fminf(size.x, size.y) * 0.4f;

    for (int j = 0; j < 8; j++) {
      float angle = time * 2.0f + j * (PI / 4.0f);
      float r = radius * (0.6f + 0.4f * sinf(time * 3.0f + j));
      Vector2 p = {center.x + cosf(angle) * r, center.y + sinf(angle) * r};
      DrawCircleV(p, 4.0f, (Color){200, 150, 255, 180});
    }
  }
}

void render_portals(GameState *game) {
  float time = (float)GetTime();

  for (int i = 0; i < MAX_PORTALS; i++) {
    QuantumPortal *p = &game->portals[i];
    if (!p->active)
      continue;

    Vector2 pos = vec2_scale(ivec2_to_vec2(p->position), CELL_SIZE);
    Vector2 center = {pos.x + CELL_SIZE * 0.5f, pos.y + CELL_SIZE * 0.5f};

    /* Phase color */
    Color phase_col;
    switch (p->phase) {
    case PHASE_RED:
      phase_col = (Color){255, 60, 40, 255};
      break;
    case PHASE_BLUE:
      phase_col = (Color){40, 180, 255, 255};
      break;
    case PHASE_GREEN:
      phase_col = (Color){50, 255, 50, 255};
      break;
    case PHASE_YELLOW:
      phase_col = (Color){255, 255, 0, 255};
      break;
    default:
      phase_col = WHITE;
      break;
    }

    /* Pulsing glow background */
    float pulse = (sinf(time * 3.0f + i * 2.0f) + 1.0f) * 0.5f;
    float glow_radius = CELL_SIZE * (0.5f + pulse * 0.15f);
    DrawCircleV(center, glow_radius, Fade(phase_col, 0.15f + pulse * 0.1f));

    /* Inner core */
    DrawCircleV(center, CELL_SIZE * 0.3f, Fade(phase_col, 0.4f + pulse * 0.3f));

    /* Spinning ring particles */
    for (int j = 0; j < 6; j++) {
      float angle = time * 2.5f + j * (PI / 3.0f) + i * 1.5f;
      float r = CELL_SIZE * 0.35f;
      Vector2 pt = {center.x + cosf(angle) * r, center.y + sinf(angle) * r};
      DrawCircleV(pt, 3.0f, Fade(phase_col, 0.7f + pulse * 0.3f));
    }

    /* Outer ring */
    DrawCircleLinesV(center, CELL_SIZE * 0.4f,
                     Fade(phase_col, 0.5f + pulse * 0.5f));

    /* Phase label */
    const char *label = "?";
    switch (p->phase) {
    case PHASE_RED:
      label = "R";
      break;
    case PHASE_BLUE:
      label = "A";
      break;
    case PHASE_GREEN:
      label = "V";
      break;
    case PHASE_YELLOW:
      label = "Am";
      break;
    default:
      break;
    }
    int tw = MeasureText(label, 12);
    DrawText(label, (int)(center.x - tw / 2), (int)(center.y - 6), 12, WHITE);
  }
}

void render_oracles(GameState *game) {
  for (int i = 0; i < MAX_ORACLES; i++) {
    GroverOracle *oracle = &game->oracles[i];
    if (!oracle->active)
      continue;

    Vector2 pos = vec2_scale(ivec2_to_vec2(oracle->position), CELL_SIZE);
    Color col = GRAY;
    if (oracle->query_count > 0) {
      if (oracle->marked_phase == PHASE_RED)
        col = RED;
      else if (oracle->marked_phase == PHASE_BLUE)
        col = BLUE;
      else if (oracle->marked_phase == PHASE_GREEN)
        col = GREEN;
      else if (oracle->marked_phase == PHASE_YELLOW)
        col = YELLOW;
    }

    DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE}, Fade(col, 0.5f));
    DrawRectangleLines(pos.x, pos.y, CELL_SIZE, CELL_SIZE, col);
    DrawText("?", pos.x + 15, pos.y + 10, 20, col);
  }
}

void render_level_transition(GameState *game) {
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();

  DrawRectangle(0, 0, sw, sh, PALETTE[0]);

  const char *title = "NIVEL COMPLETADO";
  const char *subtitle = "Preparando siguiente fase...";

  int title_width = MeasureText(title, 40);
  int sub_width = MeasureText(subtitle, 20);

  DrawText(title, sw / 2 - title_width / 2, sh / 3, 40, PALETTE[4]);
  DrawText(subtitle, sw / 2 - sub_width / 2, sh / 3 + 60, 20, PALETTE[6]);

  // STATS DISPLAY
  char stat_buf[64];
  int start_y = sh / 2;
  int line_height = 30;
  int label_x = sw / 2 - 180;
  int value_x = sw / 2 + 120;

  snprintf(stat_buf, 64, "TIEMPO:");
  DrawText(stat_buf, label_x, start_y, 20, PALETTE[5]);
  snprintf(stat_buf, 64, "%.2fs", game->player.level_time);
  DrawText(stat_buf, value_x, start_y, 20, WHITE);

  start_y += line_height;
  snprintf(stat_buf, 64, "PASOS:");
  DrawText(stat_buf, label_x, start_y, 20, PALETTE[5]);
  snprintf(stat_buf, 64, "%d", game->player.steps_taken);
  DrawText(stat_buf, value_x, start_y, 20, WHITE);

  start_y += line_height;
  snprintf(stat_buf, 64, "MUERTES:");
  DrawText(stat_buf, label_x, start_y, 20, PALETTE[5]);
  snprintf(stat_buf, 64, "%d", game->player.deaths);
  DrawText(stat_buf, value_x, start_y, 20, WHITE);

  start_y += line_height;
  snprintf(stat_buf, 64, "ENTRELAZAMIENTOS:");
  DrawText(stat_buf, label_x, start_y, 20, PALETTE[5]);
  snprintf(stat_buf, 64, "%d", game->player.entanglements_created);
  DrawText(stat_buf, value_x, start_y, 20, WHITE);

  start_y += line_height;
  snprintf(stat_buf, 64, "MEDICIONES:");
  DrawText(stat_buf, label_x, start_y, 20, PALETTE[5]);
  snprintf(stat_buf, 64, "%d", game->player.measurements_made);
  DrawText(stat_buf, value_x, start_y, 20, WHITE);

  start_y += line_height;
  snprintf(stat_buf, 64, "CAMBIOS DE FASE:");
  DrawText(stat_buf, label_x, start_y, 20, PALETTE[5]);
  snprintf(stat_buf, 64, "%d", game->player.phase_shifts);
  DrawText(stat_buf, value_x, start_y, 20, WHITE);

  DrawText("Presiona [ENTER] para continuar", sw / 2 - 150, sh - 100, 20,
           Fade(WHITE, 0.5f + sinf(GetTime() * 3.0f) * 0.5f));
}

void render_encyclopedia(GameState *game) {
  if (!game->encyclopedia_active)
    return;

  int sw = GetScreenWidth();
  int sh = GetScreenHeight();

  // Dim background
  DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 200});

  int w = 600;
  int h = 400;
  int x = (sw - w) / 2;
  int y = (sh - h) / 2;

  // Panel
  DrawRectangle(x, y, w, h, PALETTE[1]);
  DrawRectangleLines(x, y, w, h, PALETTE[7]);

  QuantumConcept *page = &game->encyclopedia[game->encyclopedia_page];

  // Header
  DrawText("ENCICLOPEDIA CUANTICA", x + 20, y + 20, 20, PALETTE[4]);
  char page_str[32];
  snprintf(page_str, 32, "%d / %d", game->encyclopedia_page + 1,
           game->encyclopedia_count);
  DrawText(page_str, x + w - 80, y + 20, 20, PALETTE[6]);

  // Content
  if (page->unlocked) {
    DrawText(page->concept_name, x + 40, y + 80, 30, PALETTE[12]);
    DrawText(page->explanation, x + 40, y + 140, 20, WHITE);
  } else {
    DrawText("???", x + 40, y + 80, 30, PALETTE[7]);
    DrawText("Bloqueado", x + 40, y + 140, 20, GRAY);
  }

  DrawText("Flechas para navegar | H para cerrar", x + 40, y + h - 40, 10,
           PALETTE[6]);
}

/* === PARTICLE SYSTEM === */

void spawn_particle(GameState *game, Vector2 pos, Vector2 vel, Color col,
                    float size, float life) {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!game->particles[i].active) {
      game->particles[i].active = true;
      game->particles[i].position = pos;
      game->particles[i].velocity = vel;
      game->particles[i].color = col;
      game->particles[i].size = size;
      game->particles[i].life = life;
      return;
    }
  }
}

void update_particles(GameState *game) {
  float dt = GetFrameTime();
  for (int i = 0; i < MAX_PARTICLES; i++) {
    Particle *p = &game->particles[i];
    if (p->active) {
      p->position.x += p->velocity.x * dt;
      p->position.y += p->velocity.y * dt;
      p->life -= dt;

      // Shrink over time
      p->size -= dt * 2.0f;

      if (p->life <= 0 || p->size <= 0) {
        p->active = false;
      }
    }
  }
}

void render_particles(GameState *game) {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    Particle *p = &game->particles[i];
    if (p->active) {
      Color c = p->color;
      c.a = (unsigned char)(255.0f * (p->life / 2.0f));
      if (c.a > 255)
        c.a = 255;
      DrawCircleV(p->position, p->size, c);
    }
  }
}

/* === MENU RENDERER === */

void render_main_menu(GameState *game) {
  // Render Atmosphere as background
  render_atmosphere_bg(game);

  // Title
  const char *title = "PHASE SHIFT";
  int title_w = MeasureText(title, 60);
  DrawText(title, (GetScreenWidth() - title_w) / 2, 200, 60, RAYWHITE);

  // Subtitle
  const char *sub = "Quantum Puzzle Game";
  int sub_w = MeasureText(sub, 30);
  DrawText(sub, (GetScreenWidth() - sub_w) / 2, 270, 30, SKYBLUE);

  // Menu Options
  int center_x = GetScreenWidth() / 2;
  int start_y = 450;

  const char *txt_start = (game->highest_level_unlocked > 0)
                              ? "ENTER - CONTINUE GAME"
                              : "ENTER - NEW GAME";
  int start_w = MeasureText(txt_start, 30);

  // Pulsing effect
  float alpha = (sinf((float)GetTime() * 3.0f) + 1.0f) * 0.5f; // 0 to 1
  Color col_start = WHITE;
  col_start.a = (unsigned char)(150 + alpha * 105);

  DrawText(txt_start, center_x - start_w / 2, start_y, 30, col_start);

  const char *txt_reset = "R - RESET PROGRESS";
  int reset_w = MeasureText(txt_reset, 20);
  DrawText(txt_reset, center_x - reset_w / 2, start_y + 100, 20, GRAY);

  // Icon
  if (title_icon.id > 0) {
    float scale = 2.0f;
    Vector2 icon_pos = {(float)(center_x - title_icon.width * scale / 2),
                        (float)(start_y + 140)};
    DrawTextureEx(title_icon, icon_pos, 0.0f, scale, WHITE);
  }

  DrawText("Kripta-Studios - 2026", 20, GetScreenHeight() - 40, 20, DARKGRAY);
}

void render_pause_menu(GameState *game) {
  // Semi-transparent overlay
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                (Color){0, 0, 0, 150});

  const char *title = "PAUSED";
  int title_w = MeasureText(title, 50);
  DrawText(title, (GetScreenWidth() - title_w) / 2, 300, 50, WHITE);

  const char *txt_resume = "ESC - RESUME";
  const char *txt_restart = "R - RESTART LEVEL";
  const char *txt_quit = "Q - QUIT TO MENU";

  DrawText(txt_resume, (GetScreenWidth() - MeasureText(txt_resume, 30)) / 2,
           400, 30, WHITE);
  DrawText(txt_restart, (GetScreenWidth() - MeasureText(txt_restart, 30)) / 2,
           450, 30, WHITE);
  DrawText(txt_quit, (GetScreenWidth() - MeasureText(txt_quit, 30)) / 2, 500,
           30, RED);
}
void spawn_floating_text(GameState *game, IVector2 pos, const char *text,
                         Color col) {
  for (int i = 0; i < 20; i++) {
    if (!game->floating_texts[i].active) {
      game->floating_texts[i].active = true;
      game->floating_texts[i].position = (Vector2){
          (float)pos.x * CELL_SIZE + CELL_SIZE / 2, (float)pos.y * CELL_SIZE};
      snprintf(game->floating_texts[i].text, 32, "%s", text);
      game->floating_texts[i].color = col;
      game->floating_texts[i].life = 1.5f;
      game->floating_texts[i].velocity_y = -20.0f;
      return;
    }
  }
}

void spawn_centered_text(GameState *game, const char *text, Color col) {
  for (int i = 0; i < 20; i++) {
    if (!game->floating_texts[i].active) {
      game->floating_texts[i].active = true;
      /* Position at center of screen */
      game->floating_texts[i].position =
          (Vector2){(float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2};
      snprintf(game->floating_texts[i].text, 32, "%s", text);
      game->floating_texts[i].color = col;
      game->floating_texts[i].life = 2.0f;         // Longer life
      game->floating_texts[i].velocity_y = -10.0f; // Slower rise
      return;
    }
  }
}

void update_and_render_floating_texts(GameState *game) {
  float dt = GetFrameTime();
  for (int i = 0; i < 20; i++) {
    if (game->floating_texts[i].active) {
      game->floating_texts[i].life -= dt;
      game->floating_texts[i].position.y +=
          game->floating_texts[i].velocity_y * dt;

      if (game->floating_texts[i].life <= 0) {
        game->floating_texts[i].active = false;
      } else {
        int width = MeasureText(game->floating_texts[i].text, 20);
        float alpha = 1.0f;
        if (game->floating_texts[i].life < 0.5f)
          alpha = game->floating_texts[i].life * 2.0f;
        Color col = game->floating_texts[i].color;
        col.a = (unsigned char)(255.0f * alpha);

        // Draw outline for better visibility
        DrawText(game->floating_texts[i].text,
                 (int)(game->floating_texts[i].position.x - width / 2) + 1,
                 (int)game->floating_texts[i].position.y + 1, 20, BLACK);

        DrawText(game->floating_texts[i].text,
                 (int)(game->floating_texts[i].position.x - width / 2),
                 (int)game->floating_texts[i].position.y, 20, col);
      }
    }
  }
}
