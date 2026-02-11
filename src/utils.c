#include "utils.h"

// Global Definitions
Color PALETTE[20];
const IVector2 DIRECTION_VECTORS[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
Sound footstep_sounds[4];
Sound blast_sound;
Sound key_pickup_sound;
Sound bomb_pickup_sound;
Sound checkpoint_sound;
Sound phase_shift_sound;
Music ambient_music;
Font game_font;

IVector2 ivec2(int x, int y) { return (IVector2){x, y}; }

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

bool eeper_can_stand_here(GameState *game, IVector2 start, int eeper_idx) {
  EeperState *eeper = &game->eepers[eeper_idx];

  for (int dx = 0; dx < eeper->size.x; dx++) {
    for (int dy = 0; dy < eeper->size.y; dy++) {
      IVector2 pos = ivec2(start.x + dx, start.y + dy);

      if (!within_map(game, pos))
        return false;

      Cell cell = game->map->data[pos.y][pos.x];
      if (cell != CELL_FLOOR && cell != CELL_EXPLOSION) {
        return false;
      }

      for (int i = 0; i < MAX_EEPERS; i++) {
        if (i == eeper_idx || game->eepers[i].dead)
          continue;

        EeperState *other = &game->eepers[i];
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
    if (current_phase == PHASE_RED || in_superposition) {
      return (Color){255, 60, 40, 255};
    } else {
      return (Color){255, 60, 40, 60};
    }
  case CELL_WALL_BLUE:
    if (current_phase == PHASE_BLUE || in_superposition) {
      return (Color){40, 180, 255, 255};
    } else {
      return (Color){40, 180, 255, 60};
    }
  case CELL_PLATFORM_RED:
    if (current_phase == PHASE_RED || in_superposition) {
      return (Color){255, 120, 80, 255};
    } else {
      return (Color){255, 120, 80, 60};
    }
  case CELL_PLATFORM_BLUE:
    if (current_phase == PHASE_BLUE || in_superposition) {
      return (Color){80, 160, 255, 255};
    } else {
      return (Color){80, 160, 255, 60};
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
    return phase == PHASE_RED || in_superposition;
  case CELL_WALL_BLUE:
  case CELL_PLATFORM_BLUE:
    return phase == PHASE_BLUE || in_superposition;
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
  PALETTE[17] = (Color){255, 50, 50, 255};  /* Phase red neon */
  PALETTE[18] = (Color){30, 180, 255, 255}; /* Phase blue neon */
  PALETTE[19] = (Color){160, 60, 220, 255}; /* Superposition purple */
}

void init_game_state(GameState *game, int rows, int cols) {
  int saved_level = game->current_level;
  GameStateKind saved_state = game->state_kind;
  DialogSystem saved_dialog = game->dialog;

  if (game->map) {
    map_free(game->map);
    game->map = NULL;
  }
  for (int i = 0; i < MAX_EEPERS; i++) {
    if (game->eepers[i].path) {
      path_free(game->eepers[i].path, game->eepers[i].path_rows);
      game->eepers[i].path = NULL;
    }
  }

  memset(game, 0, sizeof(GameState));
  game->current_level = saved_level;
  game->state_kind = saved_state;
  game->dialog = saved_dialog;

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

  for (int i = 0; i < MAX_EEPERS; i++) {
    game->eepers[i].dead = true;
    game->eepers[i].path = path_create(rows, cols);
    game->eepers[i].path_rows = rows;
    game->eepers[i].path_cols = cols;
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

  game->exit_position = ivec2(-1, -1);
  game->has_checkpoint = false;

  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  if (sw <= 0)
    sw = SCREEN_WIDTH;
  if (sh <= 0)
    sh = SCREEN_HEIGHT;
  game->camera.offset = (Vector2){sw * 0.5f, sh * 0.5f};
  game->camera.target = (Vector2){CELL_SIZE, CELL_SIZE};
  game->camera.rotation = 0.0f;
  game->camera.zoom = 1.0f;
  game->turn_animation = 0.0f;
  game->game_over = false;
  game->turn_count = 0;
}