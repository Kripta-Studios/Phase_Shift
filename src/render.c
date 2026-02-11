#include "render.h"
#include <stdio.h> // for sprintf

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

void render_eepers(GameState *game) {
  for (int i = 0; i < MAX_EEPERS; i++) {
    EeperState *eeper = &game->eepers[i];
    if (eeper->dead)
      continue;

    Vector2 pos;
    if (game->turn_animation > 0.0f) {
      pos = interpolate_positions(eeper->prev_position, eeper->position,
                                  game->turn_animation);
    } else {
      pos = vec2_scale(ivec2_to_vec2(eeper->position), CELL_SIZE);
    }

    Vector2 size = vec2_scale(ivec2_to_vec2(eeper->size), CELL_SIZE);

    switch (eeper->kind) {
    case EEPER_GUARD:
      DrawRectangleV(pos, size, PALETTE[8]);
      if (eeper->health < 1.0f) {
        Vector2 health_bar_pos = {pos.x, pos.y - 15.0f};
        DrawRectangleV(health_bar_pos, (Vector2){size.x * eeper->health, 10.0f},
                       PALETTE[12]);
      }
      draw_eyes(pos, size, eeper->eyes_angle, eeper->eyes);
      break;
    case EEPER_GNOME: {
      Vector2 gnome_size = {size.x * 0.7f, size.y * 0.7f};
      Vector2 gnome_pos = {pos.x + (size.x - gnome_size.x) * 0.5f,
                           pos.y + (size.y - gnome_size.y) * 0.5f};
      DrawRectangleV(gnome_pos, gnome_size, PALETTE[9]);
      draw_eyes(gnome_pos, gnome_size, eeper->eyes_angle, eeper->eyes);
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

  for (int i = 0; i < MAX_DETECTORS; i++) {
    QuantumDetector *det = &game->detectors[i];
    if (!det->is_active)
      continue;

    Vector2 start = vec2_scale(ivec2_to_vec2(det->position), CELL_SIZE);

    /* Draw Detector Base (Triangle) */
    Vector2 center = {start.x + CELL_SIZE * 0.5f, start.y + CELL_SIZE * 0.5f};
    DrawCircleV(center, 5.0f, (Color){100, 100, 100, 255});
    /* Draw small "eye" based on phase */
    Color eye_col = (det->detects_phase == PHASE_RED) ? RED : BLUE;
    DrawCircleV(center, 3.0f, eye_col);

    Vector2 end = start;
    for (int d = 0; d < det->view_distance; d++) {
      end.x += DIRECTION_VECTORS[det->direction].x * CELL_SIZE;
      end.y += DIRECTION_VECTORS[det->direction].y * CELL_SIZE;
    }

    /* ALWAYS draw faint beam so player sees the threat */
    float base_alpha = 0.2f + (sinf((float)GetTime() * 2.0f) + 1.0f) * 0.1f;
    if (det->beam_alpha > base_alpha)
      base_alpha = det->beam_alpha;

    Color beam_color = {255, 0, 0, (unsigned char)(base_alpha * 200)};
    /* If detected, beam is very bright red. If idle, it's faint red/pink */
    if (det->beam_alpha < 0.5f) {
      beam_color =
          (det->detects_phase == PHASE_RED)
              ? (Color){255, 100, 100, (unsigned char)(base_alpha * 100)}
              : (Color){100, 100, 255, (unsigned char)(base_alpha * 100)};
    }

    DrawLineEx(
        (Vector2){start.x + CELL_SIZE * 0.5f, start.y + CELL_SIZE * 0.5f},
        (Vector2){end.x + CELL_SIZE * 0.5f, end.y + CELL_SIZE * 0.5f}, 3.0f,
        beam_color);
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
  DrawText(coherence_text, (int)bar_x + 10, (int)bar_y + 5, 20, WHITE);

  const char *phase_text =
      (game->player.phase_system.current_phase == PHASE_RED) ? "PHASE: RED"
                                                             : "PHASE: BLUE";
  Color phase_color = (game->player.phase_system.current_phase == PHASE_RED)
                          ? (Color){255, 100, 100, 255}
                          : (Color){100, 100, 255, 255};
  DrawText(phase_text, GetScreenWidth() - 200, 50, 20, phase_color);

  if (game->player.phase_system.state == PHASE_STATE_SUPERPOSITION) {
    char super_text[50];
    sprintf(super_text, "GRABANDO ECO: %d",
            game->player.phase_system.superposition_turns_left);
    DrawText(super_text, GetScreenWidth() - 300, 80, 20, YELLOW);

    if (game->player.is_recording_echo) {
      /* Always show the prompt so user knows it exists */
      DrawText("PULSA [T] PARA ESPERAR", GetScreenWidth() - 300, 110, 18,
               (Color){200, 200, 200, 200});

      if (game->player.recording_frame > 0) {
        EchoAction *last =
            &game->player.current_recording[game->player.recording_frame - 1];
        if (last->action.kind == CMD_WAIT) {
          const char *wait_text = "ESPERANDO... (GRABANDO)";
          int w = MeasureText(wait_text, 24);
          DrawText(wait_text, (GetScreenWidth() - w) / 2,
                   GetScreenHeight() / 2 - 50, 24, (Color){255, 255, 0, 255});
        }
      }
    }
  }

  if (game->player.dead) {
    const char *death_text = "FUNCION DE ONDA COLAPSADA";
    int text_width = MeasureText(death_text, 40);
    DrawText(death_text, GetScreenWidth() / 2 - text_width / 2 + 2,
             GetScreenHeight() / 2 - 24 + 2, 40, (Color){0, 0, 0, 200});
    DrawText(death_text, GetScreenWidth() / 2 - text_width / 2,
             GetScreenHeight() / 2 - 24, 40, PALETTE[12]);
    const char *sub = "PULSA ENTER PARA REINICIAR";
    int sub_w = MeasureText(sub, 20);
    DrawText(sub, GetScreenWidth() / 2 - sub_w / 2, GetScreenHeight() / 2 + 35,
             20, (Color){150, 150, 170, 200});
  }

  /* Level indicator */
  char level_text[32];
  snprintf(level_text, 32, "LEVEL %d / %d", game->current_level + 1,
           MAX_LEVELS);
  DrawText(level_text, GetScreenWidth() - 200, 110, 18,
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
  } else {
    DrawRectangle(0, 0, sw, sh, (Color){40, 120, 255, 8});
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
  int box_h = 340;
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
  int title_w = MeasureText(page->title, 28);
  DrawText(page->title, (sw - title_w) / 2, box_y + 20, 28, PALETTE[18]);

  DrawRectangle(box_x + 30, box_y + 55, box_w - 60, 1,
                (Color){80, 200, 255, 100});

  char buf[MAX_DIALOG_TEXT];
  strncpy(buf, page->text, MAX_DIALOG_TEXT - 1);
  buf[MAX_DIALOG_TEXT - 1] = '\0';
  int text_y = box_y + 70;
  char *line = strtok(buf, "\n");
  while (line) {
    DrawText(line, box_x + 30, text_y, 18, PALETTE[7]);
    text_y += 24;
    line = strtok(NULL, "\n");
  }

  char page_text[32];
  snprintf(page_text, 32, "%d / %d", d->current_page + 1, d->page_count);
  int page_w = MeasureText(page_text, 16);
  DrawText(page_text, (sw - page_w) / 2, box_y + box_h - 30, 16,
           (Color){120, 140, 160, 200});

  float pulse = (sinf((float)GetTime() * 4.0f) + 1.0f) * 0.5f;
  unsigned char alpha = (unsigned char)(150 + pulse * 105);
  const char *prompt = (d->current_page < d->page_count - 1)
                           ? "ENTER to continue >>"
                           : "ENTER to begin >>";
  int prompt_w = MeasureText(prompt, 18);
  DrawText(prompt, sw / 2 - prompt_w / 2, box_y + box_h + 15, 18,
           (Color){80, 200, 255, alpha});
}

void render_level_transition(GameState *game) {
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  float alpha = game->level_transition_timer * 255.0f;
  if (alpha > 255)
    alpha = 255;
  DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, (unsigned char)alpha});

  if (game->level_transition_timer > 0.5f) {
    int tw = MeasureText(game->level_name, 36);
    unsigned char ta =
        (unsigned char)((game->level_transition_timer - 0.5f) * 2.0f * 255.0f);
    if (ta > 255)
      ta = 255;
    DrawText(game->level_name, (sw - tw) / 2, sh / 2 - 18, 36,
             (Color){80, 200, 255, ta});
  }
}

void render_win_screen(void) {
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 220});

  const char *title = "STABILIZATION COMPLETE";
  int tw = MeasureText(title, 48);
  DrawText(title, (sw - tw) / 2, sh / 2 - 80, 48, (Color){0, 255, 180, 255});

  const char *sub = "Subject 44's wave function has been stabilized.";
  int sw2 = MeasureText(sub, 22);
  DrawText(sub, (sw - sw2) / 2, sh / 2 - 20, 22, (Color){180, 200, 220, 200});

  const char *credits = "PHASE SHIFT -- Kripta Studios";
  int cw = MeasureText(credits, 18);
  DrawText(credits, (sw - cw) / 2, sh / 2 + 40, 18,
           (Color){100, 140, 180, 150});

  float pulse = (sinf((float)GetTime() * 3.0f) + 1.0f) * 0.5f;
  unsigned char a = (unsigned char)(120 + pulse * 135);
  const char *restart = "Press ENTER to restart";
  int rw = MeasureText(restart, 20);
  DrawText(restart, (sw - rw) / 2, sh / 2 + 100, 20, (Color){80, 200, 255, a});
}

void render_tunnels(GameState *game) {
  for (int i = 0; i < MAX_TUNNELS; i++) {
    QuantumTunnel *t = &game->tunnels[i];
    /* Only render if active/spawned? Logic: spawn_tunnel sets position.
       tunnels are always active unless game reset clears them.
       But we check by tunnel->last_failed or just initialized?
       init_game_state clears tunnels. spawn_tunnel fills them.
       We should check if position is within map or something?
       Or add 'active' flag to tunnel struct?
       Current logic: spawn_tunnel marks !last_failed.
       Let's just render all tunnels within map bounds.
    */
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

    /* Swirl effect is okay but maybe too subtle? */
    /* Add text label? "ZONE" */
    DrawText("TUNNEL", pos.x + 5, pos.y + 5, 10, border);
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