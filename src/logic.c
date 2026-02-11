#include "logic.h"
#include "levels.h"
#include <string.h>

/* ===== SPAWNING ===== */

void spawn_guard(GameState *game, IVector2 pos) {
  for (int i = 0; i < MAX_EEPERS; i++) {
    if (game->eepers[i].dead) {
      game->eepers[i].dead = false;
      game->eepers[i].kind = EEPER_GUARD;
      game->eepers[i].position = pos;
      game->eepers[i].prev_position = pos;
      game->eepers[i].size = ivec2(3, 3);
      game->eepers[i].eyes = EYES_CLOSED;
      game->eepers[i].prev_eyes = EYES_CLOSED;
      game->eepers[i].eyes_angle = M_PI * 0.5f;
      game->eepers[i].eyes_target = ivec2_add(pos, ivec2(1, 3));
      game->eepers[i].health = 1.0f;
      game->eepers[i].attack_cooldown = GUARD_ATTACK_COOLDOWN;
      return;
    }
  }
}

void spawn_gnome(GameState *game, IVector2 pos) {
  for (int i = 0; i < MAX_EEPERS; i++) {
    if (game->eepers[i].dead) {
      game->eepers[i].dead = false;
      game->eepers[i].kind = EEPER_GNOME;
      game->eepers[i].position = pos;
      game->eepers[i].prev_position = pos;
      game->eepers[i].size = ivec2(1, 1);
      game->eepers[i].eyes = EYES_CLOSED;
      game->eepers[i].prev_eyes = EYES_CLOSED;
      game->eepers[i].eyes_angle = M_PI * 0.5f;
      game->eepers[i].eyes_target = ivec2_add(pos, ivec2(0, 1));
      game->eepers[i].health = 1.0f;
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
      game->detectors[i].beam_alpha = 0.0f;
      return;
    }
  }
}

void spawn_tunnel(GameState *game, IVector2 pos, IVector2 size) {
  for (int i = 0; i < MAX_TUNNELS; i++) {
    /* valid slot if position is 0,0 (empty) */
    if (game->tunnels[i].position.x == 0 && game->tunnels[i].position.y == 0) {
      game->tunnels[i].position = pos;
      game->tunnels[i].size = size;
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

void recompute_path_for_eeper(GameState *game, int eeper_idx) {
  EeperState *eeper = &game->eepers[eeper_idx];
  Queue q;
  queue_init(&q);

  path_reset(eeper->path, eeper->path_rows, eeper->path_cols);

  for (int dy = 0; dy < eeper->size.y; dy++) {
    for (int dx = 0; dx < eeper->size.x; dx++) {
      IVector2 pos = ivec2_sub(game->player.position, ivec2(dx, dy));
      if (eeper_can_stand_here(game, pos, eeper_idx)) {
        eeper->path[pos.y][pos.x] = 0;
        queue_push(&q, pos);
      }
    }
  }

  while (q.size > 0) {
    IVector2 pos = queue_pop(&q);

    if (ivec2_eq(pos, eeper->position)) {
      break;
    }

    if (eeper->path[pos.y][pos.x] >= 10) {
      break;
    }

    for (int dir = 0; dir < 4; dir++) {
      IVector2 new_pos = ivec2_add(pos, DIRECTION_VECTORS[dir]);

      for (int step = 1; step <= 100; step++) {
        if (!eeper_can_stand_here(game, new_pos, eeper_idx))
          break;
        if (eeper->path[new_pos.y][new_pos.x] >= 0)
          break;

        eeper->path[new_pos.y][new_pos.x] = eeper->path[pos.y][pos.x] + 1;
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
        kill_player(game);
      }

      for (int e = 0; e < MAX_EEPERS; e++) {
        if (!game->eepers[e].dead &&
            inside_of_rect(game->eepers[e].position, game->eepers[e].size,
                           new_pos)) {
          game->eepers[e].damaged = true;
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
      phase->current_phase =
          (phase->current_phase == PHASE_RED) ? PHASE_BLUE : PHASE_RED;

      for (int i = 0; i < MAX_ECHOS; i++) {
        if (!game->echos[i].active) {
          game->echos[i].active = true;
          game->echos[i].phase =
              (phase->current_phase == PHASE_RED) ? PHASE_BLUE : PHASE_RED;
          memcpy(game->echos[i].recording, player->current_recording,
                 sizeof(EchoAction) * MAX_ECHO_FRAMES);
          game->echos[i].recording_index = player->recording_frame;
          game->echos[i].playback_index = 0;
          game->echos[i].position =
              player->superposition_start_pos; /* Use recorded start pos */
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

  if (player->phase_system.state == PHASE_STATE_STABLE) {
    player->phase_system.state = PHASE_STATE_SUPERPOSITION;
    player->phase_system.superposition_turns_left = SUPERPOSITION_DURATION;
    player->is_recording_echo = true;
    player->recording_frame = 0;
    player->superposition_start_pos = player->position; /* RECORD START POS */

    if (player->recording_frame < MAX_ECHO_FRAMES) {
      player->current_recording[player->recording_frame].position =
          player->position;
      player->current_recording[player->recording_frame].action.kind =
          CMD_PHASE_CHANGE;
      player->recording_frame++;
    }
  }
}

void update_coherence(GameState *game) {
  CoherenceSystem *coh = &game->player.coherence;

  coh->decay_counter++;
  if (coh->decay_counter >= 5) {
    coh->current -= 1.0f;
    coh->decay_counter = 0;
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

    for (int dist = 1; dist <= det->view_distance; dist++) {
      ray_pos = ivec2_add(ray_pos, DIRECTION_VECTORS[det->direction]);

      if (!within_map(game, ray_pos))
        break;

      if (ivec2_eq(ray_pos, player->position)) {
        if (player->phase_system.current_phase == det->detects_phase ||
            player->phase_system.state == PHASE_STATE_SUPERPOSITION) {
          detected = true;
          break;
        }
      }

      if (game->map->data[ray_pos.y][ray_pos.x] == CELL_WALL) {
        break;
      }
    }

    if (detected) {
      player->phase_system.current_phase = det->detects_phase;
      player->phase_system.state = PHASE_STATE_STABLE;
      player->phase_system.phase_lock_turns = 5;
      player->coherence.current -= 15.0f;

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
    player->position = ivec2_add(tunnel->position, tunnel->size);
    return true;
  } else {
    player->is_stuck = true;
    player->stuck_turns = 2;
    player->coherence.current -= 20.0f;
    tunnel->last_failed = true;
    return false;
  }
}

void game_player_turn(GameState *game, Direction dir) {
  PlayerState *player = &game->player;

  player->prev_position = player->position;
  player->prev_eyes = player->eyes;

  IVector2 new_pos = ivec2_add(player->position, DIRECTION_VECTORS[dir]);
  player->eyes_target = ivec2_add(new_pos, DIRECTION_VECTORS[dir]);

  if (!within_map(game, new_pos))
    return;

  Cell cell = game->map->data[new_pos.y][new_pos.x];
  bool in_superposition =
      player->phase_system.state == PHASE_STATE_SUPERPOSITION;

  if (!is_cell_solid_for_phase(cell, player->phase_system.current_phase,
                               in_superposition)) {
    player->position = new_pos;

    for (int i = 0; i < MAX_ITEMS; i++) {
      Item *item = &game->items[i];
      if (item->kind == ITEM_NONE)
        continue;
      if (!ivec2_eq(item->position, new_pos))
        continue;

      switch (item->kind) {
      case ITEM_KEY:
        player->keys++;
        item->kind = ITEM_NONE;
        PlaySound(key_pickup_sound);
        break;
      case ITEM_BOMB_REFILL:
        if (player->bombs < player->bomb_slots && item->cooldown <= 0) {
          player->bombs++;
          item->cooldown = 10;
          PlaySound(bomb_pickup_sound);
        }
        break;
      case ITEM_BOMB_SLOT:
        item->kind = ITEM_NONE;
        player->bomb_slots++;
        player->bombs = player->bomb_slots;
        break;
      case ITEM_CHECKPOINT:
        item->kind = ITEM_NONE;
        player->bombs = player->bomb_slots;
        player->coherence.current = 100.0f;
        PlaySound(checkpoint_sound);
        break;
      case ITEM_COHERENCE_PICKUP:
        item->kind = ITEM_NONE;
        player->coherence.current =
            fminf(100.0f, player->coherence.current + 5.0f);
        break;
      case ITEM_STABILIZER:
        break;
      default:
        break;
      }
    }
  } else if (cell == CELL_DOOR) {
    if (player->keys > 0) {
      player->keys--;
      flood_fill(game, new_pos, CELL_FLOOR);
      player->position = new_pos;
    }
  }

  if (player->is_recording_echo && player->recording_frame < MAX_ECHO_FRAMES) {
    player->current_recording[player->recording_frame].position =
        player->position;
    player->current_recording[player->recording_frame].action.kind = CMD_STEP;
    player->current_recording[player->recording_frame].action.dir = dir;
    player->recording_frame++;
  }

  PlaySound(footstep_sounds[rand() % 4]);
}

void game_bombs_turn(GameState *game) {
  for (int e = 0; e < MAX_EEPERS; e++) {
    game->eepers[e].damaged = false;
  }

  for (int i = 0; i < MAX_BOMBS; i++) {
    if (game->bombs[i].countdown > 0) {
      game->bombs[i].countdown--;
      if (game->bombs[i].countdown <= 0) {
        PlaySound(blast_sound);
        explode(game, game->bombs[i].position);
      }
    }
  }

  for (int e = 0; e < MAX_EEPERS; e++) {
    EeperState *eeper = &game->eepers[e];
    if (!eeper->dead && eeper->damaged) {
      switch (eeper->kind) {
      case EEPER_GUARD:
        eeper->eyes = EYES_CRINGE;
        eeper->health -= 0.45f;
        if (eeper->health <= 0.0f) {
          eeper->dead = true;
        }
        break;
      case EEPER_GNOME:
        eeper->dead = true;
        allocate_item(game, eeper->position, ITEM_KEY);
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
  for (int i = 0; i < MAX_ITEMS; i++) {
    if (game->items[i].kind == ITEM_BOMB_REFILL) {
      if (game->items[i].cooldown > 0) {
        game->items[i].cooldown--;
      }
    }
  }
}

void game_eepers_turn(GameState *game) {
  for (int i = 0; i < MAX_EEPERS; i++) {
    EeperState *eeper = &game->eepers[i];
    if (eeper->dead)
      continue;

    eeper->prev_position = eeper->position;
    eeper->prev_eyes = eeper->eyes;

    switch (eeper->kind) {
    case EEPER_GUARD: {
      recompute_path_for_eeper(game, i);
      IVector2 pos = eeper->position;
      int dist = eeper->path[pos.y][pos.x];

      if (dist == 0) {
        kill_player(game);
        eeper->eyes = EYES_SURPRISED;
      } else if (dist > 0) {
        if (eeper->attack_cooldown <= 0) {
          IVector2 best_moves[4];
          int count = 0;

          for (int dir = 0; dir < 4; dir++) {
            IVector2 test_pos = pos;
            while (eeper_can_stand_here(game, test_pos, i)) {
              test_pos = ivec2_add(test_pos, DIRECTION_VECTORS[dir]);
              if (within_map(game, test_pos) &&
                  eeper->path[test_pos.y][test_pos.x] == dist - 1) {
                best_moves[count++] = test_pos;
                break;
              }
            }
          }

          if (count > 0) {
            eeper->position = best_moves[rand() % count];
          }

          eeper->attack_cooldown = GUARD_ATTACK_COOLDOWN;
        } else {
          eeper->attack_cooldown--;
        }

        if (dist == 1) {
          eeper->eyes = EYES_ANGRY;
        } else {
          eeper->eyes = EYES_OPEN;
        }
        eeper->eyes_target = game->player.position;

        if (inside_of_rect(eeper->position, eeper->size,
                           game->player.position)) {
          kill_player(game);
        }
      } else {
        eeper->eyes = EYES_CLOSED;
        eeper->eyes_target = ivec2_add(eeper->position, ivec2(1, 3));
        eeper->attack_cooldown = GUARD_ATTACK_COOLDOWN + 1;
      }

      if (eeper->health < 1.0f) {
        eeper->health += 0.01f;
      }
      break;
    }
    case EEPER_GNOME: {
      recompute_path_for_eeper(game, i);
      IVector2 pos = eeper->position;

      if (eeper->path[pos.y][pos.x] >= 0) {
        IVector2 available[4];
        int count = 0;

        for (int dir = 0; dir < 4; dir++) {
          IVector2 new_pos = ivec2_add(pos, DIRECTION_VECTORS[dir]);
          if (within_map(game, new_pos) &&
              game->map->data[new_pos.y][new_pos.x] == CELL_FLOOR &&
              eeper->path[new_pos.y][new_pos.x] > eeper->path[pos.y][pos.x]) {
            available[count++] = new_pos;
          }
        }

        if (count > 0) {
          eeper->position = available[rand() % count];
        }
        eeper->eyes = EYES_OPEN;
        eeper->eyes_target = game->player.position;
      } else {
        eeper->eyes = EYES_CLOSED;
        eeper->eyes_target = ivec2_add(eeper->position, ivec2(0, 1));
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
  } else if (cmd.kind == CMD_PHASE_CHANGE) {
    handle_phase_change(game);
  } else if (cmd.kind == CMD_WAIT) {
    /* Play wait sound/effect */
    if (game->player.phase_system.state == PHASE_STATE_SUPERPOSITION) {
      /* Subtly tint player or show icon? handled in render. */
      /* Just play a ticking sound or low pitch pop */
      SetSoundPitch(phase_shift_sound, 0.5f);
      PlaySound(phase_shift_sound);
    }

    /* If recording echo, record wait */
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
  game_eepers_turn(game);
  update_phase_system(game);
  update_coherence(game);
  update_quantum_echos(game);
  update_quantum_detectors(game);
  update_pressure_buttons(game);
  update_pressure_buttons(game);

  /* Check Tunnels */
  for (int i = 0; i < MAX_TUNNELS; i++) {
    if (attempt_quantum_tunnel(game, i)) {
      /* Tunnel success! Play sound? */
      PlaySound(phase_shift_sound);
      break; /* Only one tunnel per turn */
    }
  }

  check_level_events(game);
}

bool check_level_complete(GameState *game) {
  if (game->exit_position.x < 0)
    return false;
  return ivec2_eq(game->player.position, game->exit_position);
}