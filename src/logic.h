#ifndef LOGIC_H
#define LOGIC_H

#include "common.h"
#include "utils.h"

// Aparición y Constructores de Nivel
void spawn_guard(GameState *game, IVector2 pos);
void spawn_gnome(GameState *game, IVector2 pos);
void allocate_item(GameState *game, IVector2 pos, ItemKind kind);
void spawn_detector(GameState *game, IVector2 pos, Direction dir,
                    PhaseKind phase);
void spawn_tunnel(GameState *game, IVector2 pos, IVector2 size,
                  IVector2 offset);
void spawn_button(GameState *game, IVector2 pos, PhaseKind phase);
void make_room(GameState *game);

// Turn Execution
void execute_turn(GameState *game, Command cmd);
void handle_phase_change(GameState *game);
void handle_superposition(GameState *game);

// State Updates
void update_phase_system(GameState *game);
void update_coherence(GameState *game);
void update_quantum_echos(GameState *game);
void update_quantum_detectors(GameState *game);
void update_pressure_buttons(GameState *game);

// Spawning
void spawn_portal(GameState *game, IVector2 pos, int linked_idx,
                  PhaseKind phase);
void spawn_oracle(GameState *game, IVector2 pos, PhaseKind phase, bool marked);

// Game Rules
bool check_level_complete(GameState *game);
void check_level_events(GameState *game);
void kill_player(GameState *game);

// Atmosphere & Flashlight
void init_atmosphere(GameState *game);
void update_atmosphere(GameState *game);

// Menus
void update_main_menu(GameState *game);
void update_pause_menu(GameState *game);

#endif