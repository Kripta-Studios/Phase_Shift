#include "utils.h"
#include "quantum.h"

// Global Definitions
Color PALETTE[25];
const IVector2 DIRECTION_VECTORS[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
AudioSound footstep_sounds[4];
AudioSound blast_sound;
AudioSound key_pickup_sound;
AudioSound bomb_pickup_sound;
AudioSound checkpoint_sound;
AudioSound phase_shift_sound;
AudioMusic ambient_music;
Font game_font;

// New sound globals
AudioSound teleport_sound;
AudioSound measurement_sound;
AudioSound entangle_sound;
AudioSound qubit_rotate_sound;
AudioSound oracle_sound;
AudioSound ice_slide_sound;
AudioSound mirror_reflect_sound;
AudioSound decoherence_sound;
AudioSound portal_activate_sound;
AudioSound level_complete_sound;
AudioSound guard_step_sound;
AudioSound open_door_sound;
AudioSound plant_bomb_sound;
Texture2D title_icon;

IVector2 ivec2(int x, int y) { return (IVector2){x, y}; }

int ivec2_dist_manhattan(IVector2 a, IVector2 b) {
  return abs(a.x - b.x) + abs(a.y - b.y);
}

IVector2 ivec2_add(IVector2 a, IVector2 b) {
  return ivec2(a.x + b.x, a.y + b.y);
}

IVector2 ivec2_sub(IVector2 a, IVector2 b) {
  return ivec2(a.x - b.x, a.y - b.y);
}

bool ivec2_eq(IVector2 a, IVector2 b) { return a.x == b.x && a.y == b.y; }

Vector2 ivec2_to_vec2(IVector2 iv) {
  return (Vector2){(float)iv.x, (float)iv.y};
}

Vector2 vec2_scale(Vector2 v, float s) { return (Vector2){v.x * s, v.y * s}; }

Vector2 vec2_add(Vector2 a, Vector2 b) {
  return (Vector2){a.x + b.x, a.y + b.y};
}

IVector2 ivec2_scale(IVector2 v, int s) { return (IVector2){v.x * s, v.y * s}; }

Map *map_create(int rows, int cols) {
  Map *map = malloc(sizeof(Map));
  map->rows = rows;
  map->cols = cols;
  map->data = malloc(rows * sizeof(Cell *));
  for (int i = 0; i < rows; i++) {
    map->data[i] = calloc(cols, sizeof(Cell));
  }
  return map;
}

void map_free(Map *map) {
  if (!map)
    return;
  for (int i = 0; i < map->rows; i++) {
    free(map->data[i]);
  }
  free(map->data);
  free(map);
}

int **path_create(int rows, int cols) {
  int **path = malloc(rows * sizeof(int *));
  for (int i = 0; i < rows; i++) {
    path[i] = malloc(cols * sizeof(int));
    for (int j = 0; j < cols; j++) {
      path[i][j] = -1;
    }
  }
  return path;
}

void path_free(int **path, int rows) {
  if (!path)
    return;
  for (int i = 0; i < rows; i++) {
    free(path[i]);
  }
  free(path);
}

void path_reset(int **path, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      path[i][j] = -1;
    }
  }
}

Vector2 Vector2LerpCustom(Vector2 v1, Vector2 v2, float amount) {
  Vector2 result = {0};
  result.x = v1.x + amount * (v2.x - v1.x);
  result.y = v1.y + amount * (v2.y - v1.y);
  return result;
}

bool within_map(GameState *game, IVector2 pos) {
  return pos.y >= 0 && pos.y < game->map->rows && pos.x >= 0 &&
         pos.x < game->map->cols;
}

bool inside_of_rect(IVector2 start, IVector2 size, IVector2 point) {
  return point.x >= start.x && point.x < start.x + size.x &&
         point.y >= start.y && point.y < start.y + size.y;
}

bool colapsor_can_stand_here(GameState *game, IVector2 start,
                             int colapsor_idx) {
  ColapsarState *colapsor = &game->colapsores[colapsor_idx];

  for (int dx = 0; dx < colapsor->size.x; dx++) {
    for (int dy = 0; dy < colapsor->size.y; dy++) {
      IVector2 pos = ivec2(start.x + dx, start.y + dy);

      if (!within_map(game, pos))
        return false;

      Cell cell = game->map->data[pos.y][pos.x];
      if (cell != CELL_FLOOR && cell != CELL_EXPLOSION) {
        return false;
      }

      for (int i = 0; i < MAX_COLAPSORES; i++) {
        if (i == colapsor_idx || game->colapsores[i].dead)
          continue;

        ColapsarState *other = &game->colapsores[i];
        if (inside_of_rect(other->position, other->size, pos)) {
          return false;
        }
      }
    }
  }
  return true;
}

Color get_cell_color(Cell cell, PhaseKind current_phase,
                     bool in_superposition) {
  switch (cell) {
  case CELL_NONE:
    return PALETTE[0];
  case CELL_FLOOR:
    return PALETTE[1];
  case CELL_WALL:
    return PALETTE[2];
  case CELL_BARRICADE:
    return PALETTE[3];
  case CELL_DOOR:
    return PALETTE[4];
  case CELL_EXPLOSION:
    return PALETTE[11];
  case CELL_EXIT:
    return PALETTE[15];
  case CELL_WALL_RED:
    /* Solid (blocks) when in RED phase, ghostly (passable) otherwise */
    if (current_phase == PHASE_RED && !in_superposition) {
      return PALETTE[17]; // Vivid Red
    } else {
      return (Color){PALETTE[17].r, PALETTE[17].g, PALETTE[17].b, 60};
    }
  case CELL_WALL_BLUE:
    if (current_phase == PHASE_BLUE && !in_superposition) {
      return PALETTE[18]; // Vivid Blue
    } else {
      return (Color){PALETTE[18].r, PALETTE[18].g, PALETTE[18].b, 60};
    }
  case CELL_WALL_GREEN:
    if (current_phase == PHASE_GREEN && !in_superposition) {
      return PALETTE[20];
    } else {
      return (Color){PALETTE[20].r, PALETTE[20].g, PALETTE[20].b, 60};
    }
  case CELL_WALL_YELLOW:
    if (current_phase == PHASE_YELLOW && !in_superposition) {
      return PALETTE[21];
    } else {
      return (Color){PALETTE[21].r, PALETTE[21].g, PALETTE[21].b, 60};
    }
  case CELL_PLATFORM_RED:
    if (current_phase == PHASE_RED && !in_superposition) {
      return (Color){255, 120, 80, 255};
    } else {
      return (Color){255, 120, 80, 60};
    }
  case CELL_PLATFORM_BLUE:
    if (current_phase == PHASE_BLUE && !in_superposition) {
      return (Color){80, 160, 255, 255};
    } else {
      return (Color){80, 160, 255, 60};
    }
  case CELL_PLATFORM_GREEN:
    if (current_phase == PHASE_GREEN && !in_superposition) {
      return PALETTE[22];
    } else {
      return (Color){PALETTE[22].r, PALETTE[22].g, PALETTE[22].b, 60};
    }
  case CELL_PLATFORM_YELLOW:
    if (current_phase == PHASE_YELLOW && !in_superposition) {
      return PALETTE[23];
    } else {
      return (Color){PALETTE[23].r, PALETTE[23].g, PALETTE[23].b, 60};
    }
  default:
    return (Color){40, 40, 50, 255};
  }
}

bool is_cell_solid_for_phase(Cell cell, PhaseKind phase,
                             bool in_superposition) {
  switch (cell) {
  case CELL_WALL:
  case CELL_BARRICADE:
  case CELL_DOOR:
    return true;
  case CELL_WALL_RED:
  case CELL_PLATFORM_RED:
    /* RED walls block RED phase, passable in other phases */
    return phase == PHASE_RED && !in_superposition;
  case CELL_WALL_BLUE:
  case CELL_PLATFORM_BLUE:
    return phase == PHASE_BLUE && !in_superposition;
  case CELL_WALL_GREEN:
  case CELL_PLATFORM_GREEN:
    return phase == PHASE_GREEN && !in_superposition;
  case CELL_WALL_YELLOW:
  case CELL_PLATFORM_YELLOW:
    return phase == PHASE_YELLOW && !in_superposition;
  case CELL_FLOOR:
  case CELL_EXPLOSION:
  case CELL_EXIT:
  case CELL_NONE:
    return false;
  default:
    return false;
  }
}

void init_palette(void) {
  PALETTE[0] = (Color){5, 5, 12, 255};      /* Background: abyss black */
  PALETTE[1] = (Color){22, 25, 35, 255};    /* Floor: deep slate */
  PALETTE[2] = (Color){45, 48, 62, 255};    /* Wall: dark gunmetal */
  PALETTE[3] = (Color){70, 30, 35, 255};    /* Barricade: dark crimson */
  PALETTE[4] = (Color){0, 255, 255, 255};   /* Cyan: keys/doors */
  PALETTE[5] = (Color){60, 210, 255, 255};  /* Player: electric cyan */
  PALETTE[6] = (Color){255, 160, 0, 255};   /* Bomb: amber glow */
  PALETTE[7] = (Color){210, 220, 240, 255}; /* White: text */
  PALETTE[8] = (Color){0, 220, 80, 255};    /* Guard: neon green */
  PALETTE[9] = (Color){255, 120, 0, 255};   /* Gnome: deep orange */
  PALETTE[10] = (Color){180, 0, 255, 255};  /* Checkpoint: vivid purple */
  PALETTE[11] = (Color){255, 60, 0, 255};   /* Explosion: hot orange */
  PALETTE[12] = (Color){255, 30, 50, 255};  /* Red/health */
  PALETTE[13] = (Color){255, 255, 60, 255}; /* Yellow: eyes */
  PALETTE[14] = (Color){80, 100, 255, 255}; /* Blue accent */
  PALETTE[15] = (Color){0, 255, 100, 255};  /* Exit: neon green */
  PALETTE[16] = (Color){15, 18, 28, 255};   /* Grid lines: near invisible */
  PALETTE[17] = (Color){255, 0, 50, 255};   /* Phase red neon (VIBRANT) */
  PALETTE[18] = (Color){0, 100, 255, 255};  /* Phase blue neon (VIBRANT) */
  PALETTE[19] = (Color){160, 60, 220, 255}; /* Superposition purple */

  // New definitions (VIBRANT)
  PALETTE[20] = (Color){0, 255, 0, 255};    /* Phase Green (Pure Lime) */
  PALETTE[21] = (Color){255, 255, 0, 255};  /* Phase Yellow (Pure Yellow) */
  PALETTE[22] = (Color){80, 255, 80, 255};  /* Platform Green */
  PALETTE[23] = (Color){255, 255, 80, 255}; /* Platform Yellow */
  PALETTE[24] = (Color){255, 0, 255, 255};  /* Logic Purple */
}

void init_game_state(GameState *game, int rows, int cols) {
  // Backup persistent state
  int saved_level = game->current_level;
  GameStateKind saved_state = game->state_kind;
  DialogSystem saved_dialog = game->dialog;
  int saved_highest_level = game->highest_level_unlocked;
  QuantumConcept saved_encyclopedia[10];
  memcpy(saved_encyclopedia, game->encyclopedia, sizeof(saved_encyclopedia));
  int saved_encyclopedia_count = game->encyclopedia_count;

  // Backup persistent player stats
  int saved_deaths = game->player.deaths;
  int saved_measurements = game->player.measurements_made;
  int saved_entanglements = game->player.entanglements_created;
  int saved_phase_shifts = game->player.phase_shifts;

  if (game->map) {
    map_free(game->map);
    game->map = NULL;
  }
  for (int i = 0; i < MAX_COLAPSORES; i++) {
    if (game->colapsores[i].path) {
      path_free(game->colapsores[i].path, game->colapsores[i].path_rows);
      game->colapsores[i].path = NULL;
    }
  }

  memset(game, 0, sizeof(GameState));

  // Restore persistent state
  game->current_level = saved_level;
  game->state_kind = saved_state;
  game->dialog = saved_dialog;
  game->highest_level_unlocked = saved_highest_level;
  memcpy(game->encyclopedia, saved_encyclopedia, sizeof(saved_encyclopedia));
  game->encyclopedia_count = saved_encyclopedia_count;

  // Restore persistent player stats
  game->player.deaths = saved_deaths;
  game->player.measurements_made = saved_measurements;
  game->player.entanglements_created = saved_entanglements;
  game->player.phase_shifts = saved_phase_shifts;

  game->map = map_create(rows, cols);
  game->player.position = ivec2(1, 1);
  game->player.prev_position = game->player.position;
  game->player.eyes = EYES_OPEN;
  game->player.prev_eyes = EYES_CLOSED;
  game->player.eyes_angle = M_PI * 0.5f;
  game->player.eyes_target = ivec2(1, 0);
  game->player.bombs = 0;
  game->player.bomb_slots = 1;
  game->player.keys = 0;
  game->player.dead = false;
  game->player.phase_system.current_phase = PHASE_RED;
  game->player.phase_system.state = PHASE_STATE_STABLE;
  game->player.phase_system.superposition_turns_left = 0;
  game->player.phase_system.phase_lock_turns = 0;
  game->player.coherence.current = 100.0f;
  game->player.coherence.decay_counter = 0;
  game->player.coherence.regen_counter = 0;
  game->player.is_recording_echo = false;
  game->player.recording_frame = 0;
  game->player.is_stuck = false;
  game->player.stuck_turns = 0;
  game->player.death_time = 0.0;

  game->shown_level_intro = false;

  for (int i = 0; i < MAX_COLAPSORES; i++) {
    game->colapsores[i].dead = true;
    game->colapsores[i].path = path_create(rows, cols);
    game->colapsores[i].path_rows = rows;
    game->colapsores[i].path_cols = cols;
  }

  for (int i = 0; i < MAX_ECHOS; i++) {
    game->echos[i].active = false;
  }

  for (int i = 0; i < MAX_DETECTORS; i++) {
    game->detectors[i].is_active = false;
  }

  for (int i = 0; i < MAX_BUTTONS; i++) {
    game->buttons[i].is_active = false;
  }

  init_portals(game);

  // Init player qubits
  game->player.qubit_count = 0;
  for (int i = 0; i < MAX_QUBITS; i++) {
    init_qubit(&game->player.qubits[i]);
    game->player.qubits[i].active = false;
  }

  game->exit_position = ivec2(-1, -1);
  game->has_checkpoint = false;

  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  if (sw <= 0)
    sw = SCREEN_WIDTH;
  if (sh <= 0)
    sh = SCREEN_HEIGHT;
  game->camera.offset = (Vector2){sw * 0.5f, sh * 0.5f};
  game->camera.target =
      (Vector2){cols * CELL_SIZE * 0.5f, rows * CELL_SIZE * 0.5f};
  game->camera.rotation = 0.0f;
  game->camera.zoom = 1.0f;
  game->turn_animation = 0.0f;
  game->game_over = false;
  game->turn_count = 0;
}