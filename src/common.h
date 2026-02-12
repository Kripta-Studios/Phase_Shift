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
// #define DEBUG_MODE
#define CELL_SIZE 50.0f
#define MAX_COLAPSORES 30
#define MAX_ITEMS 100
#define MAX_BOMBS 10
#define MAX_ECHOS 3
#define MAX_ENTANGLED 10
#define MAX_DETECTORS 10
#define MAX_TUNNELS 5
#define MAX_PORTALS 10
#define MAX_QUBITS 5
#define MAX_ORACLES 5
#define MAX_ECHO_FRAMES 20
#define SUPERPOSITION_DURATION 12
#define BASE_TURN_DURATION 0.125f
#define GUARD_ATTACK_COOLDOWN 10
#define EXPLOSION_LENGTH 10
#define EYES_ANGULAR_VELOCITY 10.0f
#define MAX_LEVELS 15
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
  CELL_WALL_GREEN,
  CELL_WALL_YELLOW,
  CELL_PLATFORM_RED,
  CELL_PLATFORM_BLUE,
  CELL_PLATFORM_GREEN,
  CELL_PLATFORM_YELLOW,
  CELL_ICE,
  CELL_MIRROR,
  CELL_ONEWAY_UP,
  CELL_ONEWAY_DOWN,
  CELL_ONEWAY_LEFT,
  CELL_ONEWAY_RIGHT,
  CELL_DECOHERENCE_ZONE,
  CELL_MEASUREMENT_ZONE,
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

typedef enum { PHASE_RED, PHASE_BLUE, PHASE_GREEN, PHASE_YELLOW } PhaseKind;

typedef enum { PHASE_STATE_STABLE, PHASE_STATE_SUPERPOSITION } PhaseState;

typedef struct {
  PhaseKind current_phase;
  PhaseState state;
  int superposition_turns_left;
  int phase_lock_turns;
  bool green_unlocked;
  bool yellow_unlocked;
} QuantumPhaseSystem;

typedef struct {
  float current;
  float max_coherence;
  int decay_counter;
  int regen_counter;
} CoherenceSystem;

typedef enum {
  CMD_STEP,
  CMD_PLANT,
  CMD_PHASE_CHANGE,
  CMD_SUPERPOSITION,
  CMD_WAIT,
  CMD_ENTANGLE,
  CMD_INTERACT
} CommandKind;

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
  bool is_permanent;
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
  int linked_portal_index;
  PhaseKind phase;
  bool requires_entanglement;
  float glow_intensity;
  bool active;
} QuantumPortal;

typedef struct {
  float alpha_real, alpha_imag; // Amplitude |0>
  float beta_real, beta_imag;   // Amplitude |1>
  bool is_measured;
  int measured_value;
  bool active;
} Qubit;

typedef struct {
  IVector2 position;
  IVector2 size;
  PhaseKind marked_phase;
  bool is_marked_state;
  int query_count;
  bool inverts_phase;
  bool active;
} GroverOracle;

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
  Qubit qubits[MAX_QUBITS];
  int qubit_count;
  bool next_echo_permanent;

  // Statistics
  int steps_taken;
  int measurements_made;
  int entanglements_created;
  int phase_shifts;
  int deaths;
  double level_time;
} PlayerState;

typedef enum {
  COLAPSOR_GUARD,
  COLAPSOR_MOTHER,
  COLAPSOR_GNOME,
  COLAPSOR_FATHER
} ColapsarKind;

typedef struct {
  ColapsarKind kind;
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
  bool teleports;
  bool entangled_with_player;
} ColapsarState;

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
  ITEM_STABILIZER,
  ITEM_QUBIT,
  ITEM_HADAMARD_GATE,
  ITEM_PHASE_UNLOCKER,
  ITEM_TELEPORT_DEVICE,
  ITEM_ECHO_PERMANENT,
  ITEM_PHASE_LOCK,
  ITEM_COHERENCE_UPGRADE,
  ITEM_CNOT_GATE
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
  GAME_STATE_MAIN_MENU,
  GAME_STATE_DIALOG,
  GAME_STATE_PLAYING,
  GAME_STATE_LEVEL_TRANSITION,
  GAME_STATE_WIN,
  GAME_STATE_PAUSE
} GameStateKind;

typedef struct {
  char concept_name[64];
  char explanation[256];
  bool unlocked;
} QuantumConcept;

#define MAX_PARTICLES 256

typedef struct {
  Vector2 position;
  Vector2 velocity;
  Color color;
  float life; // Remaining time in seconds
  float size;
  bool active;
} Particle;

#define MAX_STARS 100
#define MAX_ATOMS 10

typedef struct {
  Vector2 position;
  float brightness;
  float twinkle_speed;
} Star;

typedef struct {
  Vector2 position;
  float radius;
  Color color;
  float electron_angle;
} Atom;

typedef struct {
  Map *map;
  PlayerState player;
  ColapsarState colapsores[MAX_COLAPSORES];
  float turn_animation;
  Item items[MAX_ITEMS];
  BombState bombs[MAX_BOMBS];
  Camera2D camera;
  QuantumEcho echos[MAX_ECHOS];
  EntangledObject entangled[MAX_ENTANGLED];
  QuantumDetector detectors[MAX_DETECTORS];
  QuantumTunnel tunnels[MAX_TUNNELS];
  QuantumPortal portals[MAX_PORTALS];
  GroverOracle oracles[MAX_ORACLES];
  PressureButton buttons[MAX_BUTTONS];
  float glitch_intensity;
  bool game_over;
  int turn_count;
  GameStateKind state_kind;
  int current_level;
  int highest_level_unlocked;
  IVector2 exit_position;
  DialogSystem dialog;
  float level_transition_timer;
  char level_name[64];
  IVector2 checkpoint_pos;
  bool has_checkpoint;
  bool shown_level_intro; /* Flag to prevent dialog loop */
  bool has_teleport_device;

  QuantumConcept encyclopedia[10];
  int encyclopedia_count;
  bool encyclopedia_active;

  int encyclopedia_page;

  Particle particles[MAX_PARTICLES];
  float screen_shake;
  float flash_intensity;

  // Atmosphere
  Star stars[MAX_STARS];
  Atom atoms[MAX_ATOMS];

  // Flashlight
  bool flashlight_active;
  float flashlight_angle;

  // Level transition: deferred next level loading
  int pending_next_level; // -1 = none, >=0 = level to load after transition
} GameState;

// Global Externs
extern Color PALETTE[25];
extern const IVector2 DIRECTION_VECTORS[];
extern Sound footstep_sounds[4];
extern Sound blast_sound;
extern Sound key_pickup_sound;
extern Sound bomb_pickup_sound;
extern Sound checkpoint_sound;
extern Sound phase_shift_sound;
extern Music ambient_music;
extern Font game_font;

// New sound externs
extern Sound teleport_sound;
extern Sound measurement_sound;
extern Sound entangle_sound;
extern Sound qubit_rotate_sound;
extern Sound oracle_sound;
extern Sound ice_slide_sound;
extern Sound mirror_reflect_sound;
extern Sound decoherence_sound;
extern Sound portal_activate_sound;
extern Sound level_complete_sound;
extern Sound guard_step_sound;
extern Sound open_door_sound;
extern Sound plant_bomb_sound;

#endif