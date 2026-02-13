#include "common.h"
#include <stdio.h>

void save_game(GameState *game) {
  FILE *file = fopen("savegame.dat", "wb");
  if (file) {
    fwrite(&game->highest_level_unlocked, sizeof(int), 1, file);

    // Save Encyclopedia Unlocked Status
    for (int i = 0; i < game->encyclopedia_count; i++) {
      // Safety check: ensure we don't write out of bounds if count corrupted
      if (i < 10)
        fwrite(&game->encyclopedia[i].unlocked, sizeof(bool), 1, file);
    }

    // Save Persistent Player Stats
    fwrite(&game->player.deaths, sizeof(int), 1, file);
    fwrite(&game->player.measurements_made, sizeof(int), 1, file);
    fwrite(&game->player.entanglements_created, sizeof(int), 1, file);
    fwrite(&game->player.phase_shifts, sizeof(int), 1, file);

    fclose(file);
    printf("Game Saved! Highest Level: %d, Deaths: %d\n",
           game->highest_level_unlocked, game->player.deaths);
  } else {
    printf("Failed to save game.\n");
  }
}

void load_game(GameState *game) {
  FILE *file = fopen("savegame.dat", "rb");
  if (file) {
    fread(&game->highest_level_unlocked, sizeof(int), 1, file);

    // Cargar Estado Desbloqueado de Enciclopedia
    for (int i = 0; i < game->encyclopedia_count; i++) {
      if (i < 10) { // Control de seguridad
        fread(&game->encyclopedia[i].unlocked, sizeof(bool), 1, file);
      }
    }

    // Cargar Estadísticas Persistentes de Jugador
    // Comprobar si archivo tiene suficientes datos (retrocompatibilidad)
    long pos = ftell(file);
    fseek(file, 0, SEEK_END);
    long end = ftell(file);
    fseek(file, pos, SEEK_SET);

    if (end - pos >= sizeof(int) * 4) {
      fread(&game->player.deaths, sizeof(int), 1, file);
      fread(&game->player.measurements_made, sizeof(int), 1, file);
      fread(&game->player.entanglements_created, sizeof(int), 1, file);
      fread(&game->player.phase_shifts, sizeof(int), 1, file);
    } else {
      printf("Save file from older version. Stats reset.\n");
      game->player.deaths = 0;
      game->player.measurements_made = 0;
      game->player.entanglements_created = 0;
      game->player.phase_shifts = 0;
    }

    fclose(file);
    printf("Game Loaded! Highest Level: %d, Deaths: %d\n",
           game->highest_level_unlocked, game->player.deaths);
  } else {
    printf("No save file found. Starting fresh.\n");
    game->highest_level_unlocked = 0;
  }
}
