#ifndef RENDER_H
#define RENDER_H

#include "common.h"
#include "utils.h"

// Core Rendering
void render_game_cells(GameState *game);
void render_grid_lines(GameState *game);
void render_items(GameState *game);
void render_button_markers(GameState *game);
void render_exit_glow(GameState *game);
void render_player(GameState *game);
void render_eepers(GameState *game);
void render_bombs(GameState *game);
void render_quantum_effects(GameState *game);
void render_dark_effects(GameState *game);

// UI & Overlay
void render_hud(GameState *game);
void render_dialog(GameState *game);
void render_win_screen(void);
void render_level_transition(GameState *game);

// Helpers
Vector2 interpolate_positions(IVector2 prev, IVector2 curr, float t);
void draw_eyes(Vector2 start, Vector2 size, float angle, EyesKind kind);

void render_tunnels(GameState *game);

#endif
