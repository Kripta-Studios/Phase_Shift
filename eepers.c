#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900
#define CELL_SIZE 50.0f
#define MAX_EEPERS 30
#define MAX_ITEMS 100
#define MAX_BOMBS 10
#define MAX_ECHOS 3
#define MAX_ENTANGLED 10
#define MAX_DETECTORS 10
#define MAX_TUNNELS 5
#define MAX_ECHO_FRAMES 20
#define SUPERPOSITION_DURATION 3
#define BASE_TURN_DURATION 0.125f
#define GUARD_ATTACK_COOLDOWN 10
#define EXPLOSION_LENGTH 10
#define EYES_ANGULAR_VELOCITY 10.0f
#define MAX_LEVELS 8
#define MAX_BUTTONS 10
#define MAX_DIALOG_PAGES 12
#define MAX_DIALOG_TEXT 512

typedef struct {
  int x, y;
} IVector2;

typedef enum {
  CELL_NONE,
  CELL_FLOOR,
  CELL_WALL,
  CELL_BARRICADE,
  CELL_DOOR,
  CELL_EXPLOSION,
  CELL_WALL_RED,
  CELL_WALL_BLUE,
  CELL_PLATFORM_RED,
  CELL_PLATFORM_BLUE,
  CELL_EXIT
} Cell;

typedef enum {
  DIR_LEFT = 0,
  DIR_RIGHT = 1,
  DIR_UP = 2,
  DIR_DOWN = 3
} Direction;

const IVector2 DIRECTION_VECTORS[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

typedef enum {
  EYES_OPEN,
  EYES_CLOSED,
  EYES_ANGRY,
  EYES_CRINGE,
  EYES_SURPRISED
} EyesKind;

typedef enum { PHASE_RED, PHASE_BLUE } PhaseKind;

typedef enum { PHASE_STATE_STABLE, PHASE_STATE_SUPERPOSITION } PhaseState;

typedef struct {
  PhaseKind current_phase;
  PhaseState state;
  int superposition_turns_left;
  int phase_lock_turns;
} QuantumPhaseSystem;

typedef struct {
  float current;
  int decay_counter;
  int regen_counter;
} CoherenceSystem;

typedef enum { CMD_STEP, CMD_PLANT, CMD_PHASE_CHANGE } CommandKind;

typedef struct {
  CommandKind kind;
  Direction dir;
} Command;

typedef struct {
  IVector2 position;
  Command action;
} EchoAction;

typedef struct {
  bool active;
  PhaseKind phase;
  EchoAction recording[MAX_ECHO_FRAMES];
  int recording_index;
  int playback_index;
  IVector2 position;
  IVector2 prev_position;
  EyesKind eyes;
  float opacity;
} QuantumEcho;

typedef struct {
  IVector2 position;
  IVector2 size;
  PhaseKind phase;
  int partner_index;
  bool is_active;
} EntangledObject;

typedef struct {
  IVector2 position;
  Direction direction;
  PhaseKind detects_phase;
  int view_distance;
  bool is_active;
  float beam_alpha;
} QuantumDetector;

typedef struct {
  IVector2 position;
  IVector2 size;
  float success_probability;
  bool last_failed;
} QuantumTunnel;

typedef struct {
  IVector2 position;
  IVector2 prev_position;
  EyesKind eyes;
  EyesKind prev_eyes;
  float eyes_angle;
  IVector2 eyes_target;
  int keys;
  int bombs;
  int bomb_slots;
  bool dead;
  double death_time;
  QuantumPhaseSystem phase_system;
  CoherenceSystem coherence;
  bool is_recording_echo;
  EchoAction current_recording[MAX_ECHO_FRAMES];
  int recording_frame;
  bool is_stuck;
  int stuck_turns;
} PlayerState;

typedef enum { EEPER_GUARD, EEPER_MOTHER, EEPER_GNOME, EEPER_FATHER } EeperKind;

typedef struct {
  EeperKind kind;
  bool dead;
  IVector2 position;
  IVector2 prev_position;
  float eyes_angle;
  IVector2 eyes_target;
  EyesKind eyes;
  EyesKind prev_eyes;
  IVector2 size;
  int **path;
  int path_rows;
  int path_cols;
  bool damaged;
  float health;
  int attack_cooldown;
} EeperState;

typedef struct {
  IVector2 position;
  int countdown;
} BombState;

typedef enum {
  ITEM_NONE,
  ITEM_KEY,
  ITEM_BOMB_REFILL,
  ITEM_CHECKPOINT,
  ITEM_BOMB_SLOT,
  ITEM_COHERENCE_PICKUP,
  ITEM_STABILIZER
} ItemKind;

typedef struct {
  ItemKind kind;
  IVector2 position;
  int cooldown;
} Item;

typedef struct {
  Cell **data;
  int rows;
  int cols;
} Map;

/* === NEW: Dialog System === */
typedef struct {
  char title[64];
  char text[MAX_DIALOG_TEXT];
} DialogPage;

typedef struct {
  DialogPage pages[MAX_DIALOG_PAGES];
  int page_count;
  int current_page;
  bool active;
} DialogSystem;

/* === NEW: Pressure Buttons === */
typedef struct {
  IVector2 position;
  PhaseKind phase;
  bool is_pressed;
  bool is_active;
} PressureButton;

/* === NEW: Game State Kind === */
typedef enum {
  GAME_STATE_DIALOG,
  GAME_STATE_PLAYING,
  GAME_STATE_LEVEL_TRANSITION,
  GAME_STATE_WIN
} GameStateKind;

typedef struct {
  Map *map;
  PlayerState player;
  EeperState eepers[MAX_EEPERS];
  float turn_animation;
  Item items[MAX_ITEMS];
  BombState bombs[MAX_BOMBS];
  Camera2D camera;
  QuantumEcho echos[MAX_ECHOS];
  EntangledObject entangled[MAX_ENTANGLED];
  QuantumDetector detectors[MAX_DETECTORS];
  QuantumTunnel tunnels[MAX_TUNNELS];
  PressureButton buttons[MAX_BUTTONS];
  float glitch_intensity;
  bool game_over;
  int turn_count;
  /* NEW fields */
  GameStateKind state_kind;
  int current_level;
  IVector2 exit_position;
  DialogSystem dialog;
  float level_transition_timer;
  char level_name[64];
  IVector2 checkpoint_pos;
  bool has_checkpoint;
} GameState;

Color PALETTE[20];
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

/* Forward declarations */
void update_pressure_buttons(GameState *game);
bool check_level_complete(GameState *game);
void load_level(GameState *game, int level_index);
void init_intro_dialogs(GameState *game);
void show_level_dialog(GameState *game);

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

void init_palette(void) {
  PALETTE[0] = (Color){10, 10, 18, 255};     /* Background: deep midnight */
  PALETTE[1] = (Color){35, 38, 48, 255};     /* Floor: dark metallic */
  PALETTE[2] = (Color){55, 58, 68, 255};     /* Wall: polished steel */
  PALETTE[3] = (Color){80, 40, 40, 255};     /* Barricade: dark rust */
  PALETTE[4] = (Color){0, 255, 255, 255};    /* Cyan: keys/doors */
  PALETTE[5] = (Color){80, 200, 255, 255};   /* Player: bright cyan */
  PALETTE[6] = (Color){255, 180, 0, 255};    /* Bomb: amber glow */
  PALETTE[7] = (Color){220, 225, 240, 255};  /* White: text */
  PALETTE[8] = (Color){0, 200, 80, 255};     /* Guard: neon green */
  PALETTE[9] = (Color){255, 140, 0, 255};    /* Gnome: orange */
  PALETTE[10] = (Color){200, 0, 255, 255};   /* Checkpoint: purple */
  PALETTE[11] = (Color){255, 80, 0, 255};    /* Explosion */
  PALETTE[12] = (Color){255, 40, 40, 255};   /* Red/health */
  PALETTE[13] = (Color){255, 255, 80, 255};  /* Yellow: eyes */
  PALETTE[14] = (Color){100, 120, 255, 255}; /* Blue accent */
  PALETTE[15] = (Color){0, 255, 120, 255};   /* Exit: bright green */
  PALETTE[16] = (Color){25, 28, 38, 255};    /* Grid lines */
  PALETTE[17] = (Color){255, 60, 40, 255};   /* Phase red neon */
  PALETTE[18] = (Color){40, 180, 255, 255};  /* Phase blue neon */
  PALETTE[19] = (Color){140, 80, 200, 255};  /* Superposition purple */
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
    if (!game->tunnels[i].last_failed) {
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

/* ===== LEVEL 1: Tutorial - Phase switching ===== */
void load_level_1(GameState *game) {
  init_game_state(game, 15, 25);
  snprintf(game->level_name, 64, "LEVEL 1: AWAKENING");
  make_room(game);

  /* Vertical red wall blocking the path */
  for (int y = 3; y < 12; y++)
    game->map->data[y][10] = CELL_WALL_RED;

  /* Some wall decorations */
  for (int x = 3; x < 8; x++)
    game->map->data[3][x] = CELL_WALL;
  for (int x = 14; x < 20; x++)
    game->map->data[11][x] = CELL_WALL;

  game->player.position = ivec2(3, 7);
  game->player.prev_position = game->player.position;

  allocate_item(game, ivec2(7, 7), ITEM_COHERENCE_PICKUP);

  /* Exit on the right side - must switch to Blue phase to pass red wall */
  game->exit_position = ivec2(22, 7);
  game->map->data[7][22] = CELL_EXIT;
}

/* ===== LEVEL 2: Superposition - dual buttons ===== */
void load_level_2(GameState *game) {
  init_game_state(game, 16, 28);
  snprintf(game->level_name, 64, "LEVEL 2: SUPERPOSITION");
  make_room(game);

  /* Two-room structure */
  for (int y = 1; y < 15; y++)
    game->map->data[y][14] = CELL_WALL;
  game->map->data[7][14] = CELL_DOOR; /* Door between rooms */

  /* Red and blue buttons in room 1 (need to press both simultaneously) */
  spawn_button(game, ivec2(6, 5), PHASE_RED);
  spawn_button(game, ivec2(6, 9), PHASE_BLUE);

  /* Phase-colored platforms guiding the player */
  game->map->data[5][4] = CELL_PLATFORM_RED;
  game->map->data[9][4] = CELL_PLATFORM_BLUE;

  game->player.position = ivec2(3, 7);
  game->player.prev_position = game->player.position;
  game->player.keys = 0;

  allocate_item(game, ivec2(5, 7), ITEM_KEY);
  allocate_item(game, ivec2(10, 7), ITEM_COHERENCE_PICKUP);

  game->exit_position = ivec2(25, 8);
  game->map->data[8][25] = CELL_EXIT;
}

/* ===== LEVEL 3: Quantum Echo ===== */
void load_level_3(GameState *game) {
  init_game_state(game, 18, 30);
  snprintf(game->level_name, 64, "LEVEL 3: ECHOES");
  make_room(game);

  /* Vertical walls creating corridors */
  for (int y = 1; y < 10; y++)
    game->map->data[y][12] = CELL_WALL;
  for (int y = 8; y < 17; y++)
    game->map->data[y][20] = CELL_WALL;
  game->map->data[6][12] = CELL_DOOR;
  game->map->data[12][20] = CELL_DOOR;

  /* Button that needs sustained pressure (echo will hold it) */
  spawn_button(game, ivec2(5, 8), PHASE_RED);

  /* Checkpoint midway */
  allocate_item(game, ivec2(15, 9), ITEM_CHECKPOINT);
  allocate_item(game, ivec2(8, 4), ITEM_COHERENCE_PICKUP);

  game->player.position = ivec2(3, 8);
  game->player.prev_position = game->player.position;
  game->player.keys = 1;

  game->exit_position = ivec2(27, 9);
  game->map->data[9][27] = CELL_EXIT;
}

/* ===== LEVEL 4: Detectors + Entangled Objects ===== */
void load_level_4(GameState *game) {
  init_game_state(game, 18, 30);
  snprintf(game->level_name, 64, "LEVEL 4: ENTANGLEMENT");
  make_room(game);

  /* Detector corridor */
  for (int x = 8; x < 22; x++) {
    game->map->data[4][x] = CELL_WALL;
    game->map->data[13][x] = CELL_WALL;
  }

  spawn_detector(game, ivec2(10, 8), DIR_RIGHT, PHASE_RED);
  spawn_detector(game, ivec2(20, 8), DIR_LEFT, PHASE_BLUE);

  /* Entangled boxes */
  game->entangled[0].is_active = true;
  game->entangled[0].position = ivec2(8, 10);
  game->entangled[0].size = ivec2(1, 1);
  game->entangled[0].phase = PHASE_RED;
  game->entangled[0].partner_index = 1;
  game->entangled[1].is_active = true;
  game->entangled[1].position = ivec2(20, 10);
  game->entangled[1].size = ivec2(1, 1);
  game->entangled[1].phase = PHASE_BLUE;
  game->entangled[1].partner_index = 0;

  allocate_item(game, ivec2(14, 8), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(14, 2), ITEM_CHECKPOINT);
  allocate_item(game, ivec2(6, 15), ITEM_BOMB_REFILL);

  game->player.position = ivec2(3, 8);
  game->player.prev_position = game->player.position;

  game->exit_position = ivec2(27, 8);
  game->map->data[8][27] = CELL_EXIT;
}

/* ===== LEVEL 5: Quantum Tunnel ===== */
void load_level_5(GameState *game) {
  init_game_state(game, 20, 32);
  snprintf(game->level_name, 64, "LEVEL 5: TUNNELING");
  make_room(game);

  /* Long corridor with quantum tunnel shortcut */
  for (int y = 3; y < 17; y++)
    game->map->data[y][16] = CELL_WALL;

  /* Quantum tunnel in wall (shortcut) */
  spawn_tunnel(game, ivec2(16, 9), ivec2(1, 2));
  game->map->data[9][16] = CELL_BARRICADE;
  game->map->data[10][16] = CELL_BARRICADE;

  /* Long safe path around */
  game->map->data[3][16] = CELL_FLOOR;

  allocate_item(game, ivec2(5, 9), ITEM_STABILIZER);
  allocate_item(game, ivec2(20, 5), ITEM_CHECKPOINT);
  allocate_item(game, ivec2(8, 15), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(25, 12), ITEM_COHERENCE_PICKUP);

  spawn_gnome(game, ivec2(12, 5));

  game->player.position = ivec2(3, 9);
  game->player.prev_position = game->player.position;

  game->exit_position = ivec2(29, 10);
  game->map->data[10][29] = CELL_EXIT;
}

/* ===== LEVEL 6: Echo + Superposition Combo ===== */
void load_level_6(GameState *game) {
  init_game_state(game, 20, 32);
  snprintf(game->level_name, 64, "LEVEL 6: RESONANCE");
  make_room(game);

  /* Multi-room with phase walls */
  for (int y = 1; y < 19; y++) {
    game->map->data[y][10] = CELL_WALL;
    game->map->data[y][20] = CELL_WALL;
  }
  /* Passages */
  game->map->data[5][10] = CELL_WALL_RED;
  game->map->data[14][10] = CELL_WALL_BLUE;
  game->map->data[5][20] = CELL_WALL_BLUE;
  game->map->data[14][20] = CELL_WALL_RED;

  /* Buttons in different rooms and phases */
  spawn_button(game, ivec2(5, 10), PHASE_RED);
  spawn_button(game, ivec2(15, 5), PHASE_BLUE);
  spawn_button(game, ivec2(25, 10), PHASE_RED);

  /* Red/blue platforms */
  for (int x = 12; x < 18; x++) {
    game->map->data[8][x] = CELL_PLATFORM_RED;
    game->map->data[16][x] = CELL_PLATFORM_BLUE;
  }

  allocate_item(game, ivec2(5, 5), ITEM_CHECKPOINT);
  allocate_item(game, ivec2(15, 10), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(25, 15), ITEM_COHERENCE_PICKUP);

  game->player.position = ivec2(3, 10);
  game->player.prev_position = game->player.position;

  game->exit_position = ivec2(29, 10);
  game->map->data[10][29] = CELL_EXIT;
}

/* ===== LEVEL 7: Gauntlet ===== */
void load_level_7(GameState *game) {
  init_game_state(game, 22, 36);
  snprintf(game->level_name, 64, "LEVEL 7: GAUNTLET");
  make_room(game);

  /* Series of challenges */
  /* Room 1: Phase walls */
  for (int y = 1; y < 21; y++) {
    game->map->data[y][9] = CELL_WALL;
    game->map->data[y][18] = CELL_WALL;
    game->map->data[y][27] = CELL_WALL;
  }
  game->map->data[10][9] = CELL_WALL_RED;
  game->map->data[10][18] = CELL_WALL_BLUE;
  game->map->data[10][27] = CELL_WALL_RED;

  /* Detectors in room 2 */
  spawn_detector(game, ivec2(12, 5), DIR_DOWN, PHASE_RED);
  spawn_detector(game, ivec2(15, 15), DIR_UP, PHASE_BLUE);

  /* Buttons in room 3 */
  spawn_button(game, ivec2(22, 6), PHASE_RED);
  spawn_button(game, ivec2(22, 14), PHASE_BLUE);

  /* Guard in room 4 */
  spawn_guard(game, ivec2(30, 8));

  /* Items throughout */
  allocate_item(game, ivec2(5, 10), ITEM_CHECKPOINT);
  allocate_item(game, ivec2(14, 10), ITEM_CHECKPOINT);
  allocate_item(game, ivec2(23, 10), ITEM_CHECKPOINT);
  allocate_item(game, ivec2(8, 5), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(16, 8), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(25, 18), ITEM_BOMB_REFILL);
  game->player.bomb_slots = 2;

  game->player.position = ivec2(3, 10);
  game->player.prev_position = game->player.position;

  game->exit_position = ivec2(33, 10);
  game->map->data[10][33] = CELL_EXIT;
}

/* ===== LEVEL 8: Final Boss Puzzle ===== */
void load_level_8(GameState *game) {
  init_game_state(game, 24, 40);
  snprintf(game->level_name, 64, "LEVEL 8: COLLAPSE");
  make_room(game);

  /* Complex multi-room with all mechanics */
  for (int y = 1; y < 23; y++) {
    game->map->data[y][13] = CELL_WALL;
    game->map->data[y][26] = CELL_WALL;
  }

  /* Phase passages */
  game->map->data[8][13] = CELL_WALL_RED;
  game->map->data[16][13] = CELL_WALL_BLUE;
  game->map->data[8][26] = CELL_WALL_BLUE;
  game->map->data[16][26] = CELL_WALL_RED;

  /* Horizontal walls */
  for (int x = 14; x < 26; x++) {
    game->map->data[12][x] = CELL_WALL;
  }
  game->map->data[12][20] = CELL_DOOR;

  /* Detectors */
  spawn_detector(game, ivec2(5, 4), DIR_DOWN, PHASE_RED);
  spawn_detector(game, ivec2(20, 6), DIR_RIGHT, PHASE_BLUE);
  spawn_detector(game, ivec2(32, 12), DIR_LEFT, PHASE_RED);

  /* All three button types */
  spawn_button(game, ivec2(8, 18), PHASE_RED);
  spawn_button(game, ivec2(20, 18), PHASE_BLUE);
  spawn_button(game, ivec2(32, 18), PHASE_RED);

  /* Entangled pair */
  game->entangled[0].is_active = true;
  game->entangled[0].position = ivec2(18, 8);
  game->entangled[0].size = ivec2(1, 1);
  game->entangled[0].phase = PHASE_RED;
  game->entangled[0].partner_index = 1;
  game->entangled[1].is_active = true;
  game->entangled[1].position = ivec2(22, 8);
  game->entangled[1].size = ivec2(1, 1);
  game->entangled[1].phase = PHASE_BLUE;
  game->entangled[1].partner_index = 0;

  /* Guards */
  spawn_guard(game, ivec2(30, 4));
  spawn_gnome(game, ivec2(18, 20));
  spawn_gnome(game, ivec2(22, 20));

  /* Items */
  allocate_item(game, ivec2(6, 12), ITEM_CHECKPOINT);
  allocate_item(game, ivec2(20, 3), ITEM_CHECKPOINT);
  allocate_item(game, ivec2(3, 5), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(10, 18), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(32, 8), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(30, 20), ITEM_BOMB_REFILL);
  allocate_item(game, ivec2(16, 3), ITEM_KEY);
  game->player.bomb_slots = 2;

  game->player.position = ivec2(3, 12);
  game->player.prev_position = game->player.position;

  game->exit_position = ivec2(37, 12);
  game->map->data[12][37] = CELL_EXIT;
}

void load_level(GameState *game, int level_index) {
  game->current_level = level_index;
  switch (level_index) {
  case 0:
    load_level_1(game);
    break;
  case 1:
    load_level_2(game);
    break;
  case 2:
    load_level_3(game);
    break;
  case 3:
    load_level_4(game);
    break;
  case 4:
    load_level_5(game);
    break;
  case 5:
    load_level_6(game);
    break;
  case 6:
    load_level_7(game);
    break;
  case 7:
    load_level_8(game);
    break;
  default:
    load_level_1(game);
    break;
  }
  game->state_kind = GAME_STATE_PLAYING;
}

/* ===== DIALOG SYSTEM ===== */
void init_intro_dialogs(GameState *game) {
  DialogSystem *d = &game->dialog;
  d->current_page = 0;
  d->active = true;

  snprintf(d->pages[0].title, 64, "PHASE SHIFT");
  snprintf(d->pages[0].text, MAX_DIALOG_TEXT,
           "Year 2157. Quantum Research Facility 'PROMETHEUS'.\n"
           "You are Subject 44 -- a scientist trapped in a\n"
           "failed quantum experiment. Your consciousness is\n"
           "entangled with a particle accelerator...");

  snprintf(d->pages[1].title, 64, "QUANTUM ENTANGLEMENT");
  snprintf(d->pages[1].text, MAX_DIALOG_TEXT,
           "You exist in TWO phases of reality simultaneously:\n\n"
           "  [RED PHASE] - Solid matter, normal physics\n"
           "  [BLUE PHASE] - Ethereal energy, low gravity\n\n"
           "Press E to switch between phases.");

  snprintf(d->pages[2].title, 64, "CONTROLS");
  snprintf(d->pages[2].text, MAX_DIALOG_TEXT,
           "WASD / Arrows ...... Move (turn-based)\n"
           "E .................. Switch quantum phase\n"
           "SPACE .............. Plant bomb\n"
           "F11 ................ Toggle fullscreen\n\n"
           "Each movement is one TURN. Plan carefully!");

  snprintf(d->pages[3].title, 64, "SUPERPOSITION");
  snprintf(d->pages[3].text, MAX_DIALOG_TEXT,
           "When you switch phases, for 3 TURNS you exist\n"
           "in SUPERPOSITION -- both phases at once!\n\n"
           "During superposition you can interact with\n"
           "objects from BOTH phases simultaneously.\n"
           "Use this to press dual-phase buttons!");

  snprintf(d->pages[4].title, 64, "QUANTUM ECHO");
  snprintf(d->pages[4].text, MAX_DIALOG_TEXT,
           "After superposition ends, an ECHO appears in\n"
           "the previous phase. It replays your last moves!\n\n"
           "The echo has real physics -- it can hold buttons,\n"
           "trigger switches, and interact with the world.\n"
           "Max 3 echoes active at once.");

  snprintf(d->pages[5].title, 64, "COHERENCE");
  snprintf(d->pages[5].text, MAX_DIALOG_TEXT,
           "Your QUANTUM COHERENCE decays over time.\n"
           "When it reaches 0%%, your wave function collapses!\n\n"
           "  Decay:    -1%% every 5 turns\n"
           "  Tunneling fail: -20%%\n"
           "  Detection: -15%%\n"
           "  Pickup:   +5%%  |  Checkpoint: FULL restore");

  snprintf(d->pages[6].title, 64, "QISKIT INTEGRATION");
  snprintf(d->pages[6].text, MAX_DIALOG_TEXT,
           "Quantum tunneling uses probability-based mechanics\n"
           "inspired by IBM's Qiskit quantum computing framework.\n\n"
           "Tunnel success rate: 50%% (75%% with stabilizer).\n"
           "True quantum randomness, not pseudorandom!");

  snprintf(d->pages[7].title, 64, "MISSION");
  snprintf(d->pages[7].text, MAX_DIALOG_TEXT,
           "Complete 8 levels to reach the Stabilization\n"
           "Chamber before your wave function collapses.\n\n"
           "Each level introduces new quantum mechanics.\n"
           "Think strategically. Time is limited.\n\n"
           "         Good luck, Subject 44.");

  d->page_count = 8;
  game->state_kind = GAME_STATE_DIALOG;
}

void render_dialog(GameState *game) {
  DialogSystem *d = &game->dialog;
  if (!d->active)
    return;

  int sw = GetScreenWidth();
  int sh = GetScreenHeight();

  /* Dark overlay */
  DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 200});

  /* Dialog box dimensions */
  int box_w = 700;
  int box_h = 340;
  int box_x = (sw - box_w) / 2;
  int box_y = (sh - box_h) / 2;

  /* Metallic border (double border with glow) */
  DrawRectangle(box_x - 4, box_y - 4, box_w + 8, box_h + 8,
                (Color){80, 200, 255, 80});
  DrawRectangle(box_x - 2, box_y - 2, box_w + 4, box_h + 4,
                (Color){40, 180, 255, 150});
  DrawRectangle(box_x, box_y, box_w, box_h, (Color){15, 18, 28, 245});

  /* Corner accents */
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

  /* Title */
  DialogPage *page = &d->pages[d->current_page];
  int title_w = MeasureText(page->title, 28);
  DrawText(page->title, (sw - title_w) / 2, box_y + 20, 28, PALETTE[18]);

  /* Separator line */
  DrawRectangle(box_x + 30, box_y + 55, box_w - 60, 1,
                (Color){80, 200, 255, 100});

  /* Body text (line by line) */
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

  /* Page indicator */
  char page_text[32];
  snprintf(page_text, 32, "%d / %d", d->current_page + 1, d->page_count);
  int page_w = MeasureText(page_text, 16);
  DrawText(page_text, (sw - page_w) / 2, box_y + box_h - 30, 16,
           (Color){120, 140, 160, 200});

  /* Prompt with pulsing */
  float pulse = (sinf((float)GetTime() * 4.0f) + 1.0f) * 0.5f;
  unsigned char alpha = (unsigned char)(150 + pulse * 105);
  const char *prompt = (d->current_page < d->page_count - 1)
                           ? "ENTER to continue >>"
                           : "ENTER to begin >>";
  int prompt_w = MeasureText(prompt, 18);
  DrawText(prompt, sw / 2 - prompt_w / 2, box_y + box_h + 15, 18,
           (Color){80, 200, 255, alpha});
}

void show_level_dialog(GameState *game) {
  DialogSystem *d = &game->dialog;
  d->current_page = 0;
  d->active = true;
  d->page_count = 1;
  snprintf(d->pages[0].title, 64, "%s", game->level_name);

  const char *world = "";
  if (game->current_level < 2)
    world = "SURFACE LABORATORY";
  else if (game->current_level < 4)
    world = "EXPERIMENTATION CHAMBERS";
  else if (game->current_level < 6)
    world = "PARTICLE REACTOR";
  else
    world = "QUANTUM CORE";

  snprintf(d->pages[0].text, MAX_DIALOG_TEXT,
           "World: %s\n\n"
           "Reach the green EXIT to proceed.\n"
           "Your coherence is at %.0f%%.\n\n"
           "Good luck, Subject 44.",
           world, game->player.coherence.current);
  game->state_kind = GAME_STATE_DIALOG;
}

typedef struct {
  IVector2 *items;
  int size;
  int capacity;
} Queue;

void queue_init(Queue *q) {
  q->capacity = 256;
  q->size = 0;
  q->items = malloc(q->capacity * sizeof(IVector2));
}

void queue_push(Queue *q, IVector2 item) {
  if (q->size >= q->capacity) {
    q->capacity *= 2;
    q->items = realloc(q->items, q->capacity * sizeof(IVector2));
  }
  q->items[q->size++] = item;
}

IVector2 queue_pop(Queue *q) {
  IVector2 result = q->items[0];
  q->size--;
  for (int i = 0; i < q->size; i++) {
    q->items[i] = q->items[i + 1];
  }
  return result;
}

void queue_free(Queue *q) { free(q->items); }

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
          game->echos[i].position = player->position;
          game->echos[i].prev_position = player->position;
          game->echos[i].eyes = EYES_OPEN;
          game->echos[i].opacity = 0.5f;
          break;
        }
      }

      player->is_recording_echo = false;
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

    if (player->recording_frame < MAX_ECHO_FRAMES) {
      player->current_recording[player->recording_frame].position =
          player->position;
      player->current_recording[player->recording_frame].action.kind =
          CMD_PHASE_CHANGE;
      player->recording_frame++;
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
  }

  game_bombs_turn(game);
  game_eepers_turn(game);
  update_phase_system(game);
  update_coherence(game);
  update_quantum_echos(game);
  update_quantum_detectors(game);
  update_pressure_buttons(game);
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
    if (!det->is_active || det->beam_alpha < 0.01f)
      continue;

    Vector2 start = vec2_scale(ivec2_to_vec2(det->position), CELL_SIZE);
    Vector2 end = start;
    for (int d = 0; d < det->view_distance; d++) {
      end.x += DIRECTION_VECTORS[det->direction].x * CELL_SIZE;
      end.y += DIRECTION_VECTORS[det->direction].y * CELL_SIZE;
    }

    Color beam_color = {255, 0, 0, (unsigned char)(det->beam_alpha * 150)};
    DrawLineEx(
        (Vector2){start.x + CELL_SIZE * 0.5f, start.y + CELL_SIZE * 0.5f},
        (Vector2){end.x + CELL_SIZE * 0.5f, end.y + CELL_SIZE * 0.5f}, 5.0f,
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
    sprintf(super_text, "SUPERPOSITION: %d",
            game->player.phase_system.superposition_turns_left);
    DrawText(super_text, GetScreenWidth() - 300, 80, 20, YELLOW);
  }

  if (game->player.dead) {
    const char *death_text = "WAVE FUNCTION COLLAPSED";
    int text_width = MeasureText(death_text, 48);
    DrawText(death_text, GetScreenWidth() / 2 - text_width / 2 + 2,
             GetScreenHeight() / 2 - 24 + 2, 48, (Color){0, 0, 0, 200});
    DrawText(death_text, GetScreenWidth() / 2 - text_width / 2,
             GetScreenHeight() / 2 - 24, 48, PALETTE[12]);
    const char *sub = "Restarting...";
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

/* ===== DARK VISUAL EFFECTS ===== */
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
  /* Top */
  DrawRectangleGradientV(0, 0, sw, border, (Color){0, 0, 0, 180},
                         (Color){0, 0, 0, 0});
  /* Bottom */
  DrawRectangleGradientV(0, sh - border, sw, border, (Color){0, 0, 0, 0},
                         (Color){0, 0, 0, 180});
  /* Left */
  DrawRectangleGradientH(0, 0, border, sh, (Color){0, 0, 0, 150},
                         (Color){0, 0, 0, 0});
  /* Right */
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

/* ===== PRESSURE BUTTONS + LEVEL COMPLETION ===== */
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

bool check_level_complete(GameState *game) {
  if (game->exit_position.x < 0)
    return false;
  return ivec2_eq(game->player.position, game->exit_position);
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
  srand(time(NULL));

  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_FULLSCREEN_MODE |
                 FLAG_VSYNC_HINT);
  InitWindow(0, 0, "PHASE SHIFT");
  SetTargetFPS(144);

  InitAudioDevice();

  init_palette();

  /* Load assets */
  for (int i = 0; i < 4; i++) {
    footstep_sounds[i] = LoadSound("assets/sounds/footsteps.mp3");
    SetSoundPitch(footstep_sounds[i], 1.7f - i * 0.1f);
  }
  blast_sound = LoadSound("assets/sounds/blast.ogg");
  key_pickup_sound = LoadSound("assets/sounds/key-pickup.wav");
  bomb_pickup_sound = LoadSound("assets/sounds/bomb-pickup.ogg");
  checkpoint_sound = LoadSound("assets/sounds/checkpoint.ogg");
  phase_shift_sound =
      LoadSound("assets/sounds/checkpoint.ogg"); /* reusing for now */
  ambient_music = LoadMusicStream("assets/sounds/ambient.wav");
  SetMusicVolume(ambient_music, 0.5f);

  game_font = GetFontDefault();

  GameState game;
  /* Initial setup */
  memset(&game, 0, sizeof(GameState));
  load_level(&game, 0);
  init_intro_dialogs(&game); /* Start with intro dialogs */

  PlayMusicStream(ambient_music);

  while (!WindowShouldClose()) {
    UpdateMusicStream(ambient_music);

    if (IsKeyPressed(KEY_F11)) {
      ToggleFullscreen();
    }

    /* --- UPDATE LOOP --- */
    switch (game.state_kind) {
    case GAME_STATE_DIALOG: {
      DialogSystem *d = &game.dialog;
      if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        d->current_page++;
        if (d->current_page >= d->page_count) {
          bool was_intro = (d->page_count > 1);
          d->active = false;
          game.state_kind = GAME_STATE_PLAYING;
          /* If this was level 1 intro, show level dialog next */
          if (was_intro && game.current_level == 0 && game.turn_count == 0) {
            show_level_dialog(&game);
          }
        }
      }
      break;
    }

    case GAME_STATE_PLAYING: {
      /* P to pause/show title info? Maybe later. */

      if (game.player.dead) {
        if (GetTime() - game.player.death_time > 2.0) {
          /* Restart current level */
          load_level(&game, game.current_level);
        }
      } else {
        /* Check level completion */
        if (check_level_complete(&game)) {
          game.state_kind = GAME_STATE_LEVEL_TRANSITION;
          game.level_transition_timer = 1.0f;
        }

        /* Movement */
        if (!game.player.is_stuck) {
          if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
            execute_turn(&game, (Command){CMD_STEP, DIR_UP});
          } else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
            execute_turn(&game, (Command){CMD_STEP, DIR_DOWN});
          } else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
            execute_turn(&game, (Command){CMD_STEP, DIR_LEFT});
          } else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
            execute_turn(&game, (Command){CMD_STEP, DIR_RIGHT});
          } else if (IsKeyPressed(KEY_SPACE)) {
            execute_turn(&game, (Command){CMD_PLANT, 0});
          } else if (IsKeyPressed(KEY_E)) {
            execute_turn(&game, (Command){CMD_PHASE_CHANGE, 0});
            PlaySound(phase_shift_sound);
          }
        }
      }

      /* Animations */
      if (game.turn_animation > 0.0f) {
        game.turn_animation -= GetFrameTime() / BASE_TURN_DURATION;
        if (game.turn_animation < 0.0f) {
          game.turn_animation = 0.0f;
        }
      }

      /* Camera follow */
      Vector2 target =
          vec2_scale(ivec2_to_vec2(game.player.position), CELL_SIZE);
      /* Center camera on player with offset */
      target.x += CELL_SIZE * 0.5f;
      target.y += CELL_SIZE * 0.5f;

      game.camera.target =
          Vector2LerpCustom(game.camera.target, target, GetFrameTime() * 4.0f);
      game.camera.offset =
          (Vector2){GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
      break;
    }

    case GAME_STATE_LEVEL_TRANSITION: {
      game.level_transition_timer -= GetFrameTime() * 0.8f; /* fade speed */
      if (game.level_transition_timer <= 0.0f) {
        if (game.current_level < MAX_LEVELS - 1) {
          load_level(&game, game.current_level + 1);
          show_level_dialog(&game);
        } else {
          game.state_kind = GAME_STATE_WIN;
        }
      }
      break;
    }

    case GAME_STATE_WIN: {
      if (IsKeyPressed(KEY_ENTER)) {
        load_level(&game, 0);
        init_intro_dialogs(&game);
      }
      break;
    }
    }

    /* --- RENDER LOOP --- */
    BeginDrawing();
    ClearBackground(PALETTE[0]);

    BeginMode2D(game.camera);
    render_game_cells(&game);
    render_grid_lines(&game); /* New grid */
    render_exit_glow(&game);  /* New exit glow */
    render_items(&game);
    render_button_markers(&game); /* New buttons */
    render_quantum_effects(&game);
    render_player(&game);
    render_eepers(&game);
    render_bombs(&game);
    EndMode2D();

    render_dark_effects(&game); /* Overlay effects (vignette, scanlines) */

    if (game.state_kind == GAME_STATE_PLAYING ||
        game.state_kind == GAME_STATE_DIALOG) {
      render_hud(&game);
    }

    if (game.state_kind == GAME_STATE_DIALOG) {
      render_dialog(&game);
    } else if (game.state_kind == GAME_STATE_LEVEL_TRANSITION) {
      render_level_transition(&game);
    } else if (game.state_kind == GAME_STATE_WIN) {
      render_win_screen();
    }

    /* DrawFPS(10, 10); */
    EndDrawing();
  }

  cleanup_game(&game);
  CloseAudioDevice();
  CloseWindow();

  return 0;
}
