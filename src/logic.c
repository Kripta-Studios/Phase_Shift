#include "logic.h"
#include "levels.h"
#include "qiskit.h"
#include "quantum.h"
#include "render.h"
#include <string.h>

/* ===== SPAWNING ===== */

static void spawn_spark_effect(GameState *game, IVector2 pos, Color col) {
  Vector2 center = {pos.x * CELL_SIZE + CELL_SIZE / 2.0f,
                    pos.y * CELL_SIZE + CELL_SIZE / 2.0f};
  for (int i = 0; i < 10; i++) {
    Vector2 vel = {(float)(rand() % 200 - 100), (float)(rand() % 200 - 100)};
    spawn_particle(game, center, vel, col, 4.0f, 0.5f);
  }
}

void spawn_guard(GameState *game, IVector2 pos) {
  for (int i = 0; i < MAX_COLAPSORES; i++) {
    if (game->colapsores[i].dead) {
      game->colapsores[i].dead = false;
      game->colapsores[i].kind = COLAPSOR_GUARD;
      game->colapsores[i].position = pos;
      game->colapsores[i].prev_position = pos;
      game->colapsores[i].size = ivec2(3, 3);
      game->colapsores[i].eyes = EYES_CLOSED;
      game->colapsores[i].prev_eyes = EYES_CLOSED;
      game->colapsores[i].eyes_angle = M_PI * 0.5f;
      game->colapsores[i].eyes_target = ivec2_add(pos, ivec2(1, 3));
      game->colapsores[i].health = 1.0f;
      game->colapsores[i].attack_cooldown = GUARD_ATTACK_COOLDOWN;
      return;
    }
  }
}

void spawn_gnome(GameState *game, IVector2 pos) {
  for (int i = 0; i < MAX_COLAPSORES; i++) {
    if (game->colapsores[i].dead) {
      game->colapsores[i].dead = false;
      game->colapsores[i].kind = COLAPSOR_GNOME;
      game->colapsores[i].position = pos;
      game->colapsores[i].prev_position = pos;
      game->colapsores[i].size = ivec2(1, 1);
      game->colapsores[i].eyes = EYES_CLOSED;
      game->colapsores[i].prev_eyes = EYES_CLOSED;
      game->colapsores[i].eyes_angle = M_PI * 0.5f;
      game->colapsores[i].eyes_target = ivec2_add(pos, ivec2(0, 1));
      game->colapsores[i].health = 1.0f;
      return;
    }
  }
}

void allocate_item(GameState *game, IVector2 pos, ItemKind kind) {
  for (int i = 0; i < MAX_ITEMS; i++) {
    if (game->items[i].kind == ITEM_NONE) {
      game->items[i].kind = kind;
      game->items[i].position = pos;
      game->items[i].cooldown = 0;
      return;
    }
  }
}

void spawn_detector(GameState *game, IVector2 pos, Direction dir,
                    PhaseKind phase) {
  for (int i = 0; i < MAX_DETECTORS; i++) {
    if (!game->detectors[i].is_active) {
      game->detectors[i].is_active = true;
      game->detectors[i].position = pos;
      game->detectors[i].direction = dir;
      game->detectors[i].detects_phase = phase;
      game->detectors[i].view_distance = 5;
      game->detectors[i].current_length = 0;
      game->detectors[i].beam_alpha = 0.0f;
      return;
    }
  }
}

void spawn_tunnel(GameState *game, IVector2 pos, IVector2 size,
                  IVector2 offset) {
  for (int i = 0; i < MAX_TUNNELS; i++) {
    /* valid slot if position is 0,0 (empty) */
    if (game->tunnels[i].position.x == 0 && game->tunnels[i].position.y == 0) {
      game->tunnels[i].position = pos;
      game->tunnels[i].size = size;
      game->tunnels[i].target_offset = offset;
      game->tunnels[i].success_probability = 0.5f;
      game->tunnels[i].last_failed = false;
      return;
    }
  }
}

void spawn_button(GameState *game, IVector2 pos, PhaseKind phase) {
  for (int i = 0; i < MAX_BUTTONS; i++) {
    if (!game->buttons[i].is_active) {
      game->buttons[i].is_active = true;
      game->buttons[i].position = pos;
      game->buttons[i].phase = phase;
      game->buttons[i].is_pressed = false;
      return;
    }
  }
}

void spawn_portal(GameState *game, IVector2 pos, int linked_idx,
                  PhaseKind phase) {
  for (int i = 0; i < MAX_PORTALS; i++) {
    if (!game->portals[i].active) {
      game->portals[i].active = true;
      game->portals[i].position = pos;
      game->portals[i].linked_portal_index = linked_idx;
      game->portals[i].phase = phase;
      game->portals[i].glow_intensity = 1.0f;
      game->portals[i].requires_entanglement = false;
      return;
    }
  }
}

void spawn_oracle(GameState *game, IVector2 pos, PhaseKind phase, bool marked) {
  for (int i = 0; i < MAX_ORACLES; i++) {
    if (!game->oracles[i].active) {
      game->oracles[i].active = true;
      game->oracles[i].position = pos;
      game->oracles[i].marked_phase = phase;
      game->oracles[i].is_marked_state = marked;
      game->oracles[i].query_count = 0;
      return;
    }
  }
}

void make_room(GameState *game) {
  for (int y = 0; y < game->map->rows; y++) {
    for (int x = 0; x < game->map->cols; x++) {
      if (x == 0 || x == game->map->cols - 1 || y == 0 ||
          y == game->map->rows - 1) {
        game->map->data[y][x] = CELL_WALL;
      } else {
        game->map->data[y][x] = CELL_FLOOR;
      }
    }
  }
}

/* ===== QUEUE & HELPERS ===== */

typedef struct {
  IVector2 *items;
  int size;
  int capacity;
} Queue;

static void queue_init(Queue *q) {
  q->capacity = 256;
  q->size = 0;
  q->items = malloc(q->capacity * sizeof(IVector2));
}

static void queue_push(Queue *q, IVector2 item) {
  if (q->size >= q->capacity) {
    q->capacity *= 2;
    q->items = realloc(q->items, q->capacity * sizeof(IVector2));
  }
  q->items[q->size++] = item;
}

static IVector2 queue_pop(Queue *q) {
  IVector2 result = q->items[0];
  q->size--;
  for (int i = 0; i < q->size; i++) {
    q->items[i] = q->items[i + 1];
  }
  return result;
}

static void queue_free(Queue *q) { free(q->items); }

void recompute_path_for_colapsor(GameState *game, int colapsor_idx) {
  ColapsarState *colapsor = &game->colapsores[colapsor_idx];
  Queue q;
  queue_init(&q);

  path_reset(colapsor->path, colapsor->path_rows, colapsor->path_cols);

  for (int dy = 0; dy < colapsor->size.y; dy++) {
    for (int dx = 0; dx < colapsor->size.x; dx++) {
      IVector2 pos = ivec2_sub(game->player.position, ivec2(dx, dy));
      if (colapsor_can_stand_here(game, pos, colapsor_idx)) {
        colapsor->path[pos.y][pos.x] = 0;
        queue_push(&q, pos);
      }
    }
  }

  while (q.size > 0) {
    IVector2 pos = queue_pop(&q);

    if (ivec2_eq(pos, colapsor->position)) {
      break;
    }

    if (colapsor->path[pos.y][pos.x] >= 10) {
      break;
    }

    for (int dir = 0; dir < 4; dir++) {
      IVector2 new_pos = ivec2_add(pos, DIRECTION_VECTORS[dir]);

      for (int step = 1; step <= 100; step++) {
        if (!colapsor_can_stand_here(game, new_pos, colapsor_idx))
          break;
        if (colapsor->path[new_pos.y][new_pos.x] >= 0)
          break;

        colapsor->path[new_pos.y][new_pos.x] = colapsor->path[pos.y][pos.x] + 1;
        queue_push(&q, new_pos);

        new_pos = ivec2_add(new_pos, DIRECTION_VECTORS[dir]);
      }
    }
  }

  queue_free(&q);
}

void kill_player(GameState *game) {
  game->player.dead = true;
  game->player.death_time = GetTime();
  game->screen_shake = 2.0f;
  game->flash_intensity = 1.0f;
  PlayAudioSound(blast_sound);
}

void flood_fill(GameState *game, IVector2 start, Cell fill) {
  if (!within_map(game, start))
    return;

  Cell background = game->map->data[start.y][start.x];
  game->map->data[start.y][start.x] = fill;

  Queue q;
  queue_init(&q);
  queue_push(&q, start);

  while (q.size > 0) {
    IVector2 pos = queue_pop(&q);

    for (int dir = 0; dir < 4; dir++) {
      IVector2 new_pos = ivec2_add(pos, DIRECTION_VECTORS[dir]);

      if (within_map(game, new_pos) &&
          game->map->data[new_pos.y][new_pos.x] == background) {
        game->map->data[new_pos.y][new_pos.x] = fill;
        queue_push(&q, new_pos);
      }
    }
  }

  queue_free(&q);
}

void explode_line(GameState *game, IVector2 position, Direction dir) {
  IVector2 new_pos = position;

  for (int i = 0; i < EXPLOSION_LENGTH; i++) {
    if (!within_map(game, new_pos))
      return;

    Cell cell = game->map->data[new_pos.y][new_pos.x];

    if (cell == CELL_FLOOR || cell == CELL_EXPLOSION) {
      game->map->data[new_pos.y][new_pos.x] = CELL_EXPLOSION;

      if (ivec2_eq(new_pos, game->player.position)) {
        game->player.deaths++;
        kill_player(game);
      }

      for (int e = 0; e < MAX_COLAPSORES; e++) {
        if (!game->colapsores[e].dead &&
            inside_of_rect(game->colapsores[e].position,
                           game->colapsores[e].size, new_pos)) {
          game->colapsores[e].damaged = true;
        }
      }

      new_pos = ivec2_add(new_pos, DIRECTION_VECTORS[dir]);
    } else if (cell == CELL_BARRICADE) {
      flood_fill(game, new_pos, CELL_EXPLOSION);
      game->map->data[new_pos.y][new_pos.x] = CELL_EXPLOSION;
      return;
    } else {
      return;
    }
  }
}

void explode(GameState *game, IVector2 position) {
  game->screen_shake = 0.5f;
  game->flash_intensity = 0.5f;
  for (int dir = 0; dir < 4; dir++) {
    explode_line(game, position, dir);
  }
}

/* ===== TURN LOGIC ===== */

void update_phase_system(GameState *game) {
  PlayerState *player = &game->player;
  QuantumPhaseSystem *phase = &player->phase_system;

  if (phase->phase_lock_turns > 0) {
    phase->phase_lock_turns--;
    return;
  }

  if (phase->state == PHASE_STATE_SUPERPOSITION) {
    phase->superposition_turns_left--;

    if (phase->superposition_turns_left <= 0) {
      phase->state = PHASE_STATE_STABLE;

      /* Create echo from recorded actions, keep current phase unchanged */
      for (int i = 0; i < MAX_ECHOS; i++) {
        if (!game->echos[i].active) {
          game->echos[i].active = true;
          game->echos[i].phase = phase->current_phase;
          memcpy(game->echos[i].recording, player->current_recording,
                 sizeof(EchoAction) * MAX_ECHO_FRAMES);
          game->echos[i].recording_index = player->recording_frame;
          game->echos[i].playback_index = 0;
          game->echos[i].position = player->superposition_start_pos;
          game->echos[i].prev_position = player->superposition_start_pos;
          game->echos[i].eyes = EYES_OPEN;
          game->echos[i].opacity = 0.5f;
          break;
        }
      }

      player->is_recording_echo = false;
    }
  }
}

void handle_phase_change(GameState *game) {
  PlayerState *player = &game->player;

  if (player->phase_system.phase_lock_turns > 0) {
    return;
  }

  /* Instant phase cycle to next unlocked phase */
  QuantumPhaseSystem *phase = &player->phase_system;
  PhaseKind next = phase->current_phase;
  do {
    next = (next + 1) % 4;
    bool valid = true;
    if (next == PHASE_GREEN && !phase->green_unlocked)
      valid = false;
    if (next == PHASE_YELLOW && !phase->yellow_unlocked)
      valid = false;
    if (valid)
      break;
  } while (next != phase->current_phase);

  phase->current_phase = next;
  game->player.phase_shifts++;
  PlayAudioSound(phase_shift_sound);
}

void handle_superposition(GameState *game) {
  PlayerState *player = &game->player;

  if (player->phase_system.phase_lock_turns > 0) {
    return;
  }

  if (player->phase_system.state == PHASE_STATE_STABLE) {
    player->phase_system.state = PHASE_STATE_SUPERPOSITION;
    player->phase_system.superposition_turns_left = SUPERPOSITION_DURATION;
    game->flash_intensity = 0.3f;

    player->is_recording_echo = true;
    player->recording_frame = 0;
    player->superposition_start_pos = player->position;

    if (player->recording_frame < MAX_ECHO_FRAMES) {
      player->current_recording[player->recording_frame].position =
          player->position;
      player->current_recording[player->recording_frame].action.kind =
          CMD_SUPERPOSITION;
      player->recording_frame++;
    }
  }
}

void update_coherence(GameState *game) {
  CoherenceSystem *coh = &game->player.coherence;
  Cell cell = game->map->data[game->player.position.y][game->player.position.x];

  // Base decay
  coh->decay_counter++;
  if (coh->decay_counter >= 5) {
    coh->current -= 1.0f;
    coh->decay_counter = 0;
  }

  game->player.level_time += GetFrameTime();

  // Decoherence Zone: Rapid decay
  if (cell == CELL_DECOHERENCE_ZONE) {
    coh->current -= 2.0f; // Extra penalty per turn
    if (IsAudioSoundValid(decoherence_sound) && ((int)coh->current % 10 == 0)) {
      PlayAudioSound(decoherence_sound);
    }
  }

  // Measurement Zone: Force Collapse
  if (cell == CELL_MEASUREMENT_ZONE) {
    if (game->player.phase_system.state == PHASE_STATE_SUPERPOSITION) {
      game->player.phase_system.state = PHASE_STATE_STABLE;
      game->player.phase_system.superposition_turns_left = 0;
      game->player.measurements_made++;
      PlayAudioSound(measurement_sound);
      // Collapse to... random? Or current?
      // Superposition usually means we are in both.
      // Let's say we collapse to the phase we initiated it from, or random.
      // For gameplay stability, let's keep current_phase but remove the
      // superposition state key.
    }
  }

  if (coh->current < 30.0f) {
    game->glitch_intensity = (30.0f - coh->current) / 30.0f;
  } else {
    game->glitch_intensity = 0.0f;
  }

  if (coh->current <= 0.0f) {
    kill_player(game);
  }
}

void update_quantum_echos(GameState *game) {
  for (int i = 0; i < MAX_ECHOS; i++) {
    QuantumEcho *echo = &game->echos[i];
    if (!echo->active)
      continue;

    echo->prev_position = echo->position;

    if (echo->playback_index < echo->recording_index) {
      EchoAction *action = &echo->recording[echo->playback_index];

      if (action->action.kind == CMD_STEP) {
        echo->position =
            ivec2_add(echo->position, DIRECTION_VECTORS[action->action.dir]);
      } else if (action->action.kind == CMD_PLANT) {
        for (int b = 0; b < MAX_BOMBS; b++) {
          if (game->bombs[b].countdown <= 0) {
            game->bombs[b].countdown = 3;
            game->bombs[b].position = echo->position;
            break;
          }
        }
      } else if (action->action.kind == CMD_WAIT) {
        /* Do nothing, just stay in place */
      }

      echo->playback_index++;
    } else {
      echo->active = false;
      echo->opacity = 0.0f;
    }

    if (echo->playback_index > echo->recording_index - 5) {
      echo->opacity =
          (float)(echo->recording_index - echo->playback_index) / 5.0f;
    }
  }
}

void update_quantum_detectors(GameState *game) {
  PlayerState *player = &game->player;

  for (int i = 0; i < MAX_DETECTORS; i++) {
    QuantumDetector *det = &game->detectors[i];
    if (!det->is_active)
      continue;

    IVector2 ray_pos = det->position;
    bool detected = false;
    Direction current_dir = det->direction;
    det->current_length = 0; // Reset length each frame

    for (int dist = 1; dist <= det->view_distance; dist++) {
      ray_pos = ivec2_add(ray_pos, DIRECTION_VECTORS[current_dir]);

      if (!within_map(game, ray_pos))
        break;

      // Update actual length traversed (visuals will likely use this)
      // Note: This logic assumes straight beams (no mirrors) for visual length
      // matching If mirrors bend it, current_length will represent total path
      // length, but renderer might draw it straight if not updated. But for
      // WALLS, this fixes "beam going through wall".
      det->current_length = dist;

      // Check Oracle Interaction
      for (int o = 0; o < MAX_ORACLES; o++) {
        GroverOracle *oracle = &game->oracles[o];
        if (ivec2_eq(oracle->position, ray_pos)) {
          if (oracle->marked_phase == det->detects_phase) {
            if (!oracle->active) {
              oracle->active = true;
              oracle->query_count++;
              if (IsAudioSoundValid(oracle_sound))
                PlayAudioSound(oracle_sound);
            }
          }
        }
      }

      if (ivec2_eq(ray_pos, player->position)) {
        if (player->phase_system.current_phase == det->detects_phase ||
            player->phase_system.state == PHASE_STATE_SUPERPOSITION) {
          detected = true;
          break;
        }
      }

      Cell cell = game->map->data[ray_pos.y][ray_pos.x];
      if (cell == CELL_MIRROR) {
        if (current_dir == DIR_RIGHT)
          current_dir = DIR_DOWN;
        else if (current_dir == DIR_DOWN)
          current_dir = DIR_LEFT;
        else if (current_dir == DIR_LEFT)
          current_dir = DIR_UP;
        else if (current_dir == DIR_UP)
          current_dir = DIR_RIGHT;

        if (IsAudioSoundValid(mirror_reflect_sound))
          PlayAudioSound(mirror_reflect_sound);
      }
      // User Request: Lasers penetrate walls.
      // Removed CELL_WALL checks.
      // else if (cell == CELL_WALL || ...) { break; }
    }

    if (detected) {
      player->phase_system.current_phase = det->detects_phase;
      player->phase_system.state = PHASE_STATE_STABLE;
      player->phase_system.phase_lock_turns = 5;
      player->coherence.current -= 40.0f;
      PlayAudioSound(blast_sound);

      det->beam_alpha = 1.0f;
    } else {
      det->beam_alpha *= 0.9f;
    }
  }
}

bool attempt_quantum_tunnel(GameState *game, int tunnel_idx) {
  QuantumTunnel *tunnel = &game->tunnels[tunnel_idx];
  PlayerState *player = &game->player;

  if (player->phase_system.state != PHASE_STATE_SUPERPOSITION) {
    return false;
  }

  if (!inside_of_rect(tunnel->position, tunnel->size, player->position)) {
    return false;
  }

  float success_chance = tunnel->success_probability;

  for (int i = 0; i < MAX_ITEMS; i++) {
    if (game->items[i].kind == ITEM_STABILIZER) {
      success_chance = 0.75f;
      break;
    }
  }

  float random_val = (float)(rand() % 100) / 100.0f;

  if (random_val < success_chance) {
    player->position = ivec2_add(tunnel->position, tunnel->target_offset);
    spawn_spark_effect(game, player->position, PURPLE);
    return true;
  } else {
    player->is_stuck = true;
    player->stuck_turns = 2;
    player->coherence.current -= 20.0f;
    tunnel->last_failed = true;
    spawn_spark_effect(game, player->position, RED);
    return false;
  }
}

void game_player_turn(GameState *game, Direction dir) {
  PlayerState *player = &game->player;

  player->prev_position = player->position;
  player->prev_eyes = player->eyes;

  /* One-way door check BEFORE moving */
  Cell current_cell = game->map->data[player->position.y][player->position.x];
  if (current_cell == CELL_ONEWAY_UP && dir != DIR_UP)
    return;
  if (current_cell == CELL_ONEWAY_DOWN && dir != DIR_DOWN)
    return;
  if (current_cell == CELL_ONEWAY_LEFT && dir != DIR_LEFT)
    return;
  if (current_cell == CELL_ONEWAY_RIGHT && dir != DIR_RIGHT)
    return;

  IVector2 new_pos = ivec2_add(player->position, DIRECTION_VECTORS[dir]);
  player->eyes_target = ivec2_add(new_pos, DIRECTION_VECTORS[dir]);

  if (!within_map(game, new_pos))
    return;

  Cell cell = game->map->data[new_pos.y][new_pos.x];

  /* One-way door Entry check */
  if (cell == CELL_ONEWAY_UP && dir != DIR_UP)
    return;
  if (cell == CELL_ONEWAY_UP && dir == DIR_DOWN)
    return;
  if (cell == CELL_ONEWAY_DOWN && dir == DIR_UP)
    return;
  if (cell == CELL_ONEWAY_LEFT && dir == DIR_RIGHT)
    return;
  if (cell == CELL_ONEWAY_RIGHT && dir == DIR_LEFT)
    return;

  bool in_superposition =
      player->phase_system.state == PHASE_STATE_SUPERPOSITION;

  if (!is_cell_solid_for_phase(cell, player->phase_system.current_phase,
                               in_superposition)) {
    player->position = new_pos;
    player->steps_taken++;
    PlayAudioSound(footstep_sounds[rand() % 4]);

    // ICE LOGIC: Slide until hit something solid or non-ice
    if (cell == CELL_ICE) {
      IVector2 slide_pos = new_pos;
      int slide_limit = 20; // Prevent infinite loops
      while (slide_limit-- > 0) {
        IVector2 next_slide = ivec2_add(slide_pos, DIRECTION_VECTORS[dir]);
        if (!within_map(game, next_slide))
          break;

        Cell next_cell = game->map->data[next_slide.y][next_slide.x];

        if (is_cell_solid_for_phase(next_cell,
                                    player->phase_system.current_phase,
                                    in_superposition)) {
          PlayAudioSound(ice_slide_sound);
          break; // Stop sliding
        }

        slide_pos = next_slide;
        if (next_cell != CELL_ICE) {
          PlayAudioSound(ice_slide_sound);
          break; // Slid onto floor/other
        }
      }
      player->position = slide_pos;
    }

    /* Check collision with guards immediately after player moves */
    for (int e = 0; e < MAX_COLAPSORES; e++) {
      if (game->colapsores[e].dead)
        continue;
      if (inside_of_rect(game->colapsores[e].position, game->colapsores[e].size,
                         player->position)) {
        kill_player(game);
        break;
      }
    }

    if (player->is_recording_echo &&
        player->recording_frame < MAX_ECHO_FRAMES) {
      player->current_recording[player->recording_frame].position =
          player->position;
      player->current_recording[player->recording_frame].action.kind = CMD_STEP;
      player->current_recording[player->recording_frame].action.dir = dir;
      player->recording_frame++;
    }

    printf("[AUDIO] Playing footstep sound\n");
    PlayAudioSound(footstep_sounds[rand() % 4]);
  } else if (cell == CELL_DOOR) {
    if (player->keys > 0) {
      player->keys--;
      flood_fill(game, new_pos, CELL_FLOOR);
      player->position = new_pos;
      PlayAudioSound(open_door_sound);
    }
  }
}

void collect_item_at(GameState *game, IVector2 pos) {
  PlayerState *player = &game->player;
  for (int i = 0; i < MAX_ITEMS; i++) {
    Item *item = &game->items[i];
    if (item->kind == ITEM_NONE)
      continue;
    if (!ivec2_eq(item->position, pos))
      continue;

    switch (item->kind) {
    case ITEM_KEY:
      player->keys++;
      item->kind = ITEM_NONE;
      PlayAudioSound(key_pickup_sound);
      spawn_spark_effect(game, item->position, YELLOW);
      spawn_centered_text(game, "LLAVE OBTENIDA", BLUE);
      break;
    case ITEM_BOMB_REFILL:
      if (player->bombs < player->bomb_slots && item->cooldown <= 0) {
        player->bombs++;
        item->cooldown = 10;
        PlayAudioSound(bomb_pickup_sound);
        spawn_spark_effect(game, item->position, RED);
      }
      break;
    case ITEM_BOMB_SLOT:
      item->kind = ITEM_NONE;
      player->bomb_slots++;
      player->bombs = player->bomb_slots;
      PlayAudioSound(key_pickup_sound);
      spawn_spark_effect(game, item->position, ORANGE);
      spawn_centered_text(game, "AMPLIACION BOMBAS", ORANGE);
      break;
    case ITEM_CHECKPOINT:
      item->kind = ITEM_NONE;
      player->bombs = player->bomb_slots;
      player->coherence.current = 100.0f;
      game->has_checkpoint = true;
      game->checkpoint_pos = item->position;
      PlayAudioSound(checkpoint_sound);
      spawn_spark_effect(game, item->position, GREEN);
      spawn_centered_text(game, "PUNTO DE CONTROL", GREEN);
      break;
    case ITEM_COHERENCE_PICKUP:
      item->kind = ITEM_NONE;
      player->coherence.current =
          fminf(100.0f, player->coherence.current + 5.0f);
      PlayAudioSound(key_pickup_sound); // Added sound
      spawn_spark_effect(game, item->position, BLUE);
      spawn_centered_text(game, "+5% COHERENCE", YELLOW);
      break;
    case ITEM_STABILIZER:
      break;
    case ITEM_PHASE_UNLOCKER:
      item->kind = ITEM_NONE;
      if (!game->player.phase_system.green_unlocked) {
        game->player.phase_system.green_unlocked = true;
        spawn_centered_text(game, "FASE VERDE DESBLOQUEADA", GREEN);
      } else {
        game->player.phase_system.yellow_unlocked = true;
        spawn_centered_text(game, "FASE AMARILLA DESBLOQUEADA", YELLOW);
      }
      PlayAudioSound(key_pickup_sound);
      break;
    case ITEM_QUBIT:
      item->kind = ITEM_NONE;
      if (game->player.qubit_count < MAX_QUBITS) {
        init_qubit(&game->player.qubits[game->player.qubit_count]);
        game->player.qubit_count++;
        if (IsAudioSoundValid(qubit_rotate_sound))
          PlayAudioSound(qubit_rotate_sound);
        spawn_spark_effect(game, item->position, SKYBLUE);
      }
      break;
    case ITEM_HADAMARD_GATE:
      item->kind = ITEM_NONE;
      if (game->player.qubit_count > 0) {
        apply_hadamard_gate(&game->player.qubits[game->player.qubit_count - 1]);
      }
      break;
    case ITEM_TELEPORT_DEVICE:
      item->kind = ITEM_NONE;
      game->has_teleport_device = true;
      PlayAudioSound(key_pickup_sound);
      spawn_spark_effect(game, item->position, MAGENTA);
      break;
    case ITEM_PHASE_LOCK:
      item->kind = ITEM_NONE;
      game->player.phase_system.phase_lock_turns = 10;
      break;
    default:
      break;
    }
  }
}

void game_bombs_turn(GameState *game) {
  for (int e = 0; e < MAX_COLAPSORES; e++) {
    game->colapsores[e].damaged = false;
  }

  for (int i = 0; i < MAX_BOMBS; i++) {
    if (game->bombs[i].countdown > 0) {
      game->bombs[i].countdown--;
      if (game->bombs[i].countdown <= 0) {
        PlayAudioSound(blast_sound);
        explode(game, game->bombs[i].position);
      }
    }
  }

  for (int e = 0; e < MAX_COLAPSORES; e++) {
    ColapsarState *colapsor = &game->colapsores[e];
    if (!colapsor->dead && colapsor->damaged) {
      switch (colapsor->kind) {
      case COLAPSOR_GUARD:
        colapsor->eyes = EYES_CRINGE;
        colapsor->health -= 0.45f;
        if (colapsor->health <= 0.0f) {
          colapsor->dead = true;
        }
        break;
      case COLAPSOR_GNOME:
        colapsor->dead = true;
        allocate_item(game, colapsor->position, ITEM_KEY);
        break;
      default:
        break;
      }
    }
  }
}

void game_explosions_turn(GameState *game) {
  for (int y = 0; y < game->map->rows; y++) {
    for (int x = 0; x < game->map->cols; x++) {
      if (game->map->data[y][x] == CELL_EXPLOSION) {
        game->map->data[y][x] = CELL_FLOOR;
      }
    }
  }
}

void game_items_turn(GameState *game) {
  collect_item_at(game, game->player.position);
  for (int i = 0; i < MAX_ITEMS; i++) {
    if (game->items[i].kind == ITEM_BOMB_REFILL) {
      if (game->items[i].cooldown > 0) {
        game->items[i].cooldown--;
      }
    }
  }
}

void game_colapsores_turn(GameState *game) {
  for (int i = 0; i < MAX_COLAPSORES; i++) {
    ColapsarState *colapsor = &game->colapsores[i];
    if (colapsor->dead)
      continue;

    colapsor->prev_position = colapsor->position;
    colapsor->prev_eyes = colapsor->eyes;

    if (colapsor->entangled_with_player) {
      IVector2 delta =
          ivec2_sub(game->player.position, game->player.prev_position);
      if (delta.x != 0 || delta.y != 0) {
        IVector2 target = ivec2_add(colapsor->position, delta);
        if (within_map(game, target)) {
          /* Fix: Use proper collision logic so they don't clip walls */
          if (colapsor_can_stand_here(game, target, i)) {
            colapsor->position = target;
          }
        }
      }

      colapsor->entanglement_turns++;
      if (colapsor->entanglement_turns >= 6) {
        colapsor->entangled_with_player = false;
        colapsor->eyes = EYES_OPEN;
        spawn_spark_effect(game, colapsor->position, RED);
        spawn_floating_text(game, colapsor->position, "VINCULO ROTO", RED);
        if (IsAudioSoundValid(entangle_sound))
          PlayAudioSound(entangle_sound);
      } else {
        // Visual feedback
        colapsor->eyes = EYES_SURPRISED;
      }

      continue; // Skip AI behavior
    }

    switch (colapsor->kind) {
    case COLAPSOR_GUARD: {
      recompute_path_for_colapsor(game, i);
      IVector2 pos = colapsor->position;
      int dist = colapsor->path[pos.y][pos.x];

      if (dist == 0) {
        kill_player(game);
        colapsor->eyes = EYES_SURPRISED;
      } else if (dist > 0) {
        if (colapsor->attack_cooldown <= 0) {
          IVector2 best_moves[4];
          int count = 0;

          for (int dir = 0; dir < 4; dir++) {
            IVector2 test_pos = pos;
            while (colapsor_can_stand_here(game, test_pos, i)) {
              test_pos = ivec2_add(test_pos, DIRECTION_VECTORS[dir]);
              if (within_map(game, test_pos) &&
                  colapsor->path[test_pos.y][test_pos.x] == dist - 1) {
                best_moves[count++] = test_pos;
                break;
              }
            }
          }

          if (count > 0) {
            colapsor->position = best_moves[rand() % count];
            PlayAudioSound(guard_step_sound);
          }

          colapsor->attack_cooldown = GUARD_ATTACK_COOLDOWN;
        } else {
          colapsor->attack_cooldown--;
        }

        if (dist == 1) {
          colapsor->eyes = EYES_ANGRY;
        } else {
          colapsor->eyes = EYES_OPEN;
        }
        colapsor->eyes_target = game->player.position;

        if (inside_of_rect(colapsor->position, colapsor->size,
                           game->player.position)) {
          kill_player(game);
        }
      } else {
        /* NO PATH FOUND - WANDER RANDOMLY */
        colapsor->eyes = EYES_SURPRISED; // visual cue they are confused
        colapsor->eyes_target = ivec2_add(colapsor->position, ivec2(1, 3));
        colapsor->attack_cooldown = GUARD_ATTACK_COOLDOWN + 1;

        /* Increase wander chance to 50% to prevent being "frozen" */
        if (!colapsor->dead && (rand() % 100) < 50) {
          int dir = rand() % 4;
          IVector2 new_pos =
              ivec2_add(colapsor->position, DIRECTION_VECTORS[dir]);
          if (colapsor_can_stand_here(game, new_pos, i)) {
            colapsor->position = new_pos;
          }
        }
      }

      if (colapsor->health < 1.0f) {
        colapsor->health += 0.01f;
      }
      break;
    }
    case COLAPSOR_GNOME: {
      recompute_path_for_colapsor(game, i);
      IVector2 pos = colapsor->position;

      if (colapsor->path[pos.y][pos.x] >= 0) {
        IVector2 available[4];
        int count = 0;

        for (int dir = 0; dir < 4; dir++) {
          IVector2 new_pos = ivec2_add(pos, DIRECTION_VECTORS[dir]);
          if (within_map(game, new_pos) &&
              game->map->data[new_pos.y][new_pos.x] == CELL_FLOOR &&
              colapsor->path[new_pos.y][new_pos.x] >
                  colapsor->path[pos.y][pos.x]) {
            available[count++] = new_pos;
          }
        }

        if (count > 0) {
          colapsor->position = available[rand() % count];
        }
        colapsor->eyes = EYES_OPEN;
        colapsor->eyes_target = game->player.position;
      } else {
        colapsor->eyes = EYES_CLOSED;
        colapsor->eyes_target = ivec2_add(colapsor->position, ivec2(0, 1));
      }
      break;
    }
    default:
      break;
    }
  }
}

void handle_plant_bomb(GameState *game) {
  PlayerState *player = &game->player;

  if (player->bombs > 0) {
    for (int i = 0; i < MAX_BOMBS; i++) {
      if (game->bombs[i].countdown <= 0) {
        game->bombs[i].countdown = 3;
        game->bombs[i].position = player->position;
        break;
      }
    }
    player->bombs--;

    if (player->is_recording_echo &&
        player->recording_frame < MAX_ECHO_FRAMES) {
      player->current_recording[player->recording_frame].position =
          player->position;
      player->current_recording[player->recording_frame].action.kind =
          CMD_PLANT;
      player->recording_frame++;
    }
  }
}

void update_pressure_buttons(GameState *game) {
  PlayerState *player = &game->player;
  bool in_super = player->phase_system.state == PHASE_STATE_SUPERPOSITION;

  for (int i = 0; i < MAX_BUTTONS; i++) {
    PressureButton *b = &game->buttons[i];
    if (!b->is_active)
      continue;

    b->is_pressed = false;

    /* Player on button? */
    if (ivec2_eq(player->position, b->position)) {
      if (in_super || player->phase_system.current_phase == b->phase) {
        b->is_pressed = true;
      }
    }
    /* Echo on button? */
    for (int e = 0; e < MAX_ECHOS; e++) {
      if (game->echos[e].active &&
          ivec2_eq(game->echos[e].position, b->position)) {
        if (game->echos[e].phase == b->phase) {
          b->is_pressed = true;
        }
      }
    }

    /* Colapsor on button? */
    for (int c = 0; c < MAX_COLAPSORES; c++) {
      if (!game->colapsores[c].dead &&
          ivec2_eq(game->colapsores[c].position, b->position)) {
        b->is_pressed = true;
      }
    }
  }
}

void handle_entangle_action(GameState *game) {
  // Toggle entanglement with entities adjacent to player
  PlayerState *player = &game->player;
  bool any_entangled = false;

  // Apply strict coherence cost regardless of outcome
  player->coherence.current -= 10.0f;
  spawn_floating_text(game, player->position, "-10 COHERENCIA", RED);

  for (int i = 0; i < MAX_COLAPSORES; i++) {
    ColapsarState *colapsor = &game->colapsores[i];
    if (colapsor->dead)
      continue;

    // Increased range to 2 (Chebyshev/King distance) for easier use
    int dx = abs(player->position.x - colapsor->position.x);
    int dy = abs(player->position.y - colapsor->position.y);

    // Check range (Chebyshev distance <= 3 for better usability)
    if (dx <= 3 && dy <= 3) {
      // Toggle state
      colapsor->entangled_with_player = !colapsor->entangled_with_player;
      if (colapsor->entangled_with_player) {
        colapsor->entanglement_turns = 0;
      }
      any_entangled = true;

      if (colapsor->entangled_with_player) {
        game->player.entanglements_created++;
        colapsor->eyes = EYES_SURPRISED;
        spawn_spark_effect(game, colapsor->position, GREEN);
        spawn_floating_text(game, colapsor->position, "ENTRELAZADO!", GREEN);
      } else {
        colapsor->eyes = EYES_OPEN;
        spawn_spark_effect(game, colapsor->position, WHITE);
        spawn_floating_text(game, colapsor->position, "DESVINCULADO", WHITE);
      }
    }
  }

  if (any_entangled) {
    if (IsAudioSoundValid(entangle_sound))
      PlayAudioSound(entangle_sound);
  } else {
    // Feedback if no valid target found
    spawn_floating_text(game, player->position, "NO HAY OBJETIVO", RED);
  }
}

void update_oracles(GameState *game) {
  for (int i = 0; i < MAX_ORACLES; i++) {
    GroverOracle *oracle = &game->oracles[i];
    if (!oracle->active) {
      // Maybe animate idle state?
    }
  }
}

void execute_turn(GameState *game, Command cmd) {
  if (game->player.is_stuck) {
    game->player.stuck_turns--;
    if (game->player.stuck_turns <= 0) {
      game->player.is_stuck = false;
    }
    return;
  }

  game->turn_animation = 1.0f;
  game->turn_count++;

  game_explosions_turn(game);
  game_items_turn(game);

  if (cmd.kind == CMD_STEP) {
    game_player_turn(game, cmd.dir);
  } else if (cmd.kind == CMD_PLANT) {
    handle_plant_bomb(game);
    PlayAudioSound(plant_bomb_sound);
    spawn_spark_effect(game, game->player.position, ORANGE);
  } else if (cmd.kind == CMD_PHASE_CHANGE) {
    handle_phase_change(game);
    spawn_spark_effect(game, game->player.position, SKYBLUE);
  } else if (cmd.kind == CMD_SUPERPOSITION) {
    handle_superposition(game);
    spawn_spark_effect(game, game->player.position, PURPLE);
  } else if (cmd.kind == CMD_INTERACT) {
    handle_portal_teleport(game);
    spawn_spark_effect(game, game->player.position, MAGENTA);
  } else if (cmd.kind == CMD_ENTANGLE) {
    handle_entangle_action(game);
    spawn_spark_effect(game, game->player.position, GREEN);
  } else if (cmd.kind == CMD_WAIT) {
    if (game->player.phase_system.state == PHASE_STATE_SUPERPOSITION) {
      SetAudioSoundPitch(phase_shift_sound, 0.5f);
      PlayAudioSound(phase_shift_sound);
    }
    if (game->player.is_recording_echo &&
        game->player.recording_frame < MAX_ECHO_FRAMES) {
      game->player.current_recording[game->player.recording_frame].position =
          game->player.position;
      game->player.current_recording[game->player.recording_frame].action.kind =
          CMD_WAIT;
      game->player.recording_frame++;
    }
  }

  game_bombs_turn(game);
  game_colapsores_turn(game);
  update_phase_system(game);
  update_coherence(game);
  update_quantum_echos(game);
  update_quantum_detectors(game);
  update_pressure_buttons(game);
  update_oracles(game);

  for (int i = 0; i < MAX_TUNNELS; i++) {
    if (attempt_quantum_tunnel(game, i)) {
      PlayAudioSound(teleport_sound);
      game_items_turn(game); // Force pickup check immediately
      break;
    }
  }

  /* Automatic Portal Check */
  handle_portal_teleport(game);

  /* Check for telefrag (landing on enemy after portal) */
  for (int i = 0; i < MAX_COLAPSORES; i++) {
    if (!game->colapsores[i].dead &&
        ivec2_eq(game->player.position, game->colapsores[i].position)) {
      kill_player(game);
      break;
    }
  }

  check_level_events(game);
}

bool check_level_complete(GameState *game) {
  if (game->exit_position.x < 0)
    return false;
  return ivec2_eq(game->player.position, game->exit_position);
}
