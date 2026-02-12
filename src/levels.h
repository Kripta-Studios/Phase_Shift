#ifndef LEVELS_H
#define LEVELS_H

#include "common.h"
#include "logic.h"

void load_level(GameState *game, int level_index);

void load_level_1(GameState *game);
void load_level_2(GameState *game);
void load_level_3(GameState *game);
void load_level_4(GameState *game);
void load_level_5(GameState *game);
void load_level_6(GameState *game);
void load_level_7(GameState *game);
void load_level_8(GameState *game);
void load_level_9(GameState *game);
void load_level_10(GameState *game);
void load_level_11(GameState *game);
void load_level_12(GameState *game);
void load_level_13(GameState *game);
void load_level_14(GameState *game);
void load_level_15(GameState *game);
void init_encyclopedia(GameState *game);

void init_intro_dialogs(GameState *game);
void show_level_dialog(GameState *game);
void check_level_events(GameState *game);

#endif