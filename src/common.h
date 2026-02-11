#ifndef COMMON_H
#define COMMON_H

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
#define SUPERPOSITION_DURATION 12
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

typedef enum { CMD_STEP, CMD_PLANT, CMD_PHASE_CHANGE, CMD_WAIT } CommandKind;

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
  IVector2 superposition_start_pos;
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

typedef struct {
  IVector2 position;
  PhaseKind phase;
  bool is_pressed;
  bool is_active;
} PressureButton;

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
  GameStateKind state_kind;
  int current_level;
  IVector2 exit_position;
  DialogSystem dialog;
  float level_transition_timer;
  char level_name[64];
  IVector2 checkpoint_pos;
  bool has_checkpoint;
  bool shown_level_intro; /* Flag to prevent dialog loop */
} GameState;

// Global Externs
extern Color PALETTE[20];
extern const IVector2 DIRECTION_VECTORS[];
extern Sound footstep_sounds[4];
extern Sound blast_sound;
extern Sound key_pickup_sound;
extern Sound bomb_pickup_sound;
extern Sound checkpoint_sound;
extern Sound phase_shift_sound;
extern Music ambient_music;
extern Font game_font;

#endif