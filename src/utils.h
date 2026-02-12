#ifndef UTILS_H
#define UTILS_H

#include "common.h"

IVector2 ivec2(int x, int y);
IVector2 ivec2_add(IVector2 a, IVector2 b);
IVector2 ivec2_sub(IVector2 a, IVector2 b);
bool ivec2_eq(IVector2 a, IVector2 b);
int ivec2_dist_manhattan(IVector2 a, IVector2 b);
Vector2 ivec2_to_vec2(IVector2 iv);
Vector2 vec2_scale(Vector2 v, float s);
Vector2 vec2_add(Vector2 a, Vector2 b);
IVector2 ivec2_scale(IVector2 v, int s);

Map *map_create(int rows, int cols);
void map_free(Map *map);
int **path_create(int rows, int cols);
void path_free(int **path, int rows);
void path_reset(int **path, int rows, int cols);

Vector2 Vector2LerpCustom(Vector2 v1, Vector2 v2, float amount);

bool within_map(GameState *game, IVector2 pos);
bool inside_of_rect(IVector2 start, IVector2 size, IVector2 point);
bool colapsor_can_stand_here(GameState *game, IVector2 start, int colapsor_idx);

Color get_cell_color(Cell cell, PhaseKind current_phase, bool in_superposition);
bool is_cell_solid_for_phase(Cell cell, PhaseKind phase, bool in_superposition);

void init_palette(void);
void init_game_state(GameState *game, int rows, int cols);

#endif