#include "common.h"
#include <stdio.h>

void save_game(GameState *game) {
  FILE *file = fopen("savegame.dat", "wb");
  if (file) {
    fwrite(&game->highest_level_unlocked, sizeof(int), 1, file);

    // Save Encyclopedia Unlocked Status
    for (int i = 0; i < game->encyclopedia_count; i++) {
      fwrite(&game->encyclopedia[i].unlocked, sizeof(bool), 1, file);
    }

    fclose(file);
    printf("Game Saved! Highest Level: %d\n", game->highest_level_unlocked);
  } else {
    printf("Failed to save game.\n");
  }
}

void load_game(GameState *game) {
  FILE *file = fopen("savegame.dat", "rb");
  if (file) {
    fread(&game->highest_level_unlocked, sizeof(int), 1, file);

    // Load Encyclopedia Unlocked Status
    for (int i = 0; i < game->encyclopedia_count; i++) {
      if (i < 10) { // Safety check against header MAX
        fread(&game->encyclopedia[i].unlocked, sizeof(bool), 1, file);
      }
    }

    fclose(file);
    printf("Game Loaded! Highest Level: %d\n", game->highest_level_unlocked);
  } else {
    printf("No save file found. Starting fresh.\n");
    game->highest_level_unlocked = 0;
  }
}
