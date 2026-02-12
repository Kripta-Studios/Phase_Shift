#ifndef RENDER_H
#define RENDER_H

#include "common.h"
#include "utils.h"

// Post-processing shader system
extern Shader post_shader;
extern RenderTexture2D post_target;
extern bool post_shader_ready;

void init_post_shader(void);
void unload_post_shader(void);
void begin_post_processing(void);
void end_post_processing(GameState *game);

// Core Rendering
void render_game_cells(GameState *game);
void render_grid_lines(GameState *game);
void render_items(GameState *game);
void render_button_markers(GameState *game);
void render_exit_glow(GameState *game);
void render_player(GameState *game);
void render_colapsores(GameState *game);
void render_bombs(GameState *game);
void render_quantum_effects(GameState *game);
void render_dark_effects(GameState *game);

// UI & Overlay
void render_hud(GameState *game);
void render_dialog(GameState *game);
void render_win_screen(void);
void render_level_transition(GameState *game);
void render_encyclopedia(GameState *game);

// Helpers
Vector2 interpolate_positions(IVector2 prev, IVector2 curr, float t);
void draw_eyes(Vector2 start, Vector2 size, float angle, EyesKind kind);

void render_tunnels(GameState *game);
void render_portals(GameState *game);
void render_oracles(GameState *game);

void spawn_particle(GameState *game, Vector2 pos, Vector2 vel, Color col,
                    float size, float life);
void update_particles(GameState *game);
void render_particles(GameState *game);

void render_atmosphere_bg(GameState *game);
void render_flashlight_overlay(GameState *game);

void render_main_menu(GameState *game);
void render_pause_menu(GameState *game);

#endif