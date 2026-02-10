#ifndef LOGIC_H
#define LOGIC_H

#include "common.h"
#include "utils.h"

// Spawning & Level Builders
void spawn_guard(GameState *game, IVector2 pos);
void spawn_gnome(GameState *game, IVector2 pos);
void allocate_item(GameState *game, IVector2 pos, ItemKind kind);
void spawn_detector(GameState *game, IVector2 pos, Direction dir,
                    PhaseKind phase);
void spawn_tunnel(GameState *game, IVector2 pos, IVector2 size);
void spawn_button(GameState *game, IVector2 pos, PhaseKind phase);
void make_room(GameState *game);

// Turn Execution
void execute_turn(GameState *game, Command cmd);

// State Updates
void update_phase_system(GameState *game);
void update_coherence(GameState *game);
void update_quantum_echos(GameState *game);
void update_quantum_detectors(GameState *game);
void update_pressure_buttons(GameState *game);

// Game Rules
bool check_level_complete(GameState *game);
void kill_player(GameState *game);

#endif
