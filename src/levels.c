#include "levels.h"
#include "logic.h"

#include <stdio.h>
#include <string.h>

void init_encyclopedia(GameState *game) {
  game->encyclopedia_count = 8;
  game->encyclopedia_page = 0;
  game->encyclopedia_active = false;

  strncpy(game->encyclopedia[0].concept_name, "Superposicion", 64);
  strncpy(game->encyclopedia[0].explanation,
          "Un sistema cuantico puede existir\nen multiples estados "
          "simultaneamente\nhasta ser observado.",
          256);
  game->encyclopedia[0].unlocked = true;

  strncpy(game->encyclopedia[1].concept_name, "Entrelazamiento", 64);
  strncpy(game->encyclopedia[1].explanation,
          "Dos particulas entrelazadas comparten\nestado "
          "instantaneamente,\nsin importar la distancia.",
          256);
  game->encyclopedia[1].unlocked = false;

  strncpy(game->encyclopedia[2].concept_name, "Decoherencia", 64);
  strncpy(game->encyclopedia[2].explanation,
          "La interaccion con el entorno destruye\nlos estados "
          "cuanticos,\ncolapsando la superposicion.",
          256);
  game->encyclopedia[2].unlocked = false;

  strncpy(game->encyclopedia[3].concept_name, "Compuerta Hadamard", 64);
  strncpy(game->encyclopedia[3].explanation,
          "Transforma un qubit de un estado\ndefinido a una superposicion\nde "
          "ambos estados.",
          256);
  game->encyclopedia[3].unlocked = false;

  strncpy(game->encyclopedia[4].concept_name, "Algoritmo de Grover", 64);
  strncpy(game->encyclopedia[4].explanation,
          "Busqueda cuantica que encuentra\nun elemento en sqrt(N) pasos\nen "
          "lugar de N.",
          256);
  game->encyclopedia[4].unlocked = false;

  strncpy(game->encyclopedia[5].concept_name, "Tunel Cuantico", 64);
  strncpy(game->encyclopedia[5].explanation,
          "Una particula puede atravesar\nbarreras de potencial "
          "que\nclasicamente serian imposibles.",
          256);
  game->encyclopedia[5].unlocked = false;

  strncpy(game->encyclopedia[6].concept_name, "Medicion Cuantica", 64);
  strncpy(game->encyclopedia[6].explanation,
          "Observar un sistema cuantico\ncolapsa su funcion de onda\na un "
          "estado definido.",
          256);
  game->encyclopedia[6].unlocked = false;

  strncpy(game->encyclopedia[7].concept_name, "Teletransportacion", 64);
  strncpy(game->encyclopedia[7].explanation,
          "Transferencia de informacion cuantica\nusando entrelazamiento "
          "y\ncomunicacion clasica.",
          256);
  game->encyclopedia[7].unlocked = false;
}

/* ========== NIVEL 1: INTERFERENCIA ========== */
void load_level_1(GameState *game) {
  int rows = 14;
  int cols = 20;
  init_game_state(game, rows, cols);
  game->current_level = 0;
  snprintf(game->level_name, 64, "NIVEL 1: INTERFERENCIA");

  make_room(game);

  /* Patrón de interferencia cuántica: muros alternados rojo/azul */
  for (int x = 6; x <= 14; x++) {
    for (int y = 2; y < rows - 2; y++) {
      if ((x + y) % 2 == 0) {
        game->map->data[y][x] = CELL_WALL_RED;
      } else {
        game->map->data[y][x] = CELL_WALL_BLUE;
      }
    }
  }

  /* Crear "caminos" específicos para cada fase */
  for (int y = 3; y < rows - 3; y += 3) {
    game->map->data[y][8] = CELL_FLOOR;
    game->map->data[y][12] = CELL_FLOOR;
  }

  /* Llave en el centro del patrón */
  allocate_item(game, ivec2(10, rows / 2), ITEM_KEY);

  /* Puerta y salida */
  game->map->data[rows / 2][17] = CELL_DOOR;
  game->exit_position = ivec2(18, rows / 2);
  game->map->data[rows / 2][18] = CELL_EXIT;

  /* Muros alrededor de la salida para forzar paso por la puerta */
  for (int y = 1; y < rows - 1; y++) {
    if (y != rows / 2) {
      game->map->data[y][16] = CELL_WALL;
      game->map->data[y][17] = CELL_WALL;
      game->map->data[y][18] = CELL_WALL;
    }
  }

  game->player.position = ivec2(2, rows / 2);
}

/* ========== NIVEL 2: ZIGZAG DE FASE ========== */
void load_level_2(GameState *game) {
  int rows = 14;
  int cols = 22;
  init_game_state(game, rows, cols);
  game->current_level = 1;
  snprintf(game->level_name, 64, "NIVEL 2: ZIGZAG DE FASE");

  make_room(game);

  /* Corredor zigzag que FUERZA cambio de fase */
  /* Sección 1: bloqueada por muros ROJOS (necesitas fase AZUL para pasar) */
  for (int y = 1; y < rows - 1; y++) {
    game->map->data[y][5] = CELL_WALL_RED;
  }
  /* Apertura solo abajo */
  game->map->data[rows - 3][5] = CELL_FLOOR;

  /* Sección 2: bloqueada por muros AZULES (necesitas fase ROJA) */
  for (int y = 1; y < rows - 1; y++) {
    game->map->data[y][10] = CELL_WALL_BLUE;
  }
  /* Apertura solo arriba */
  game->map->data[2][10] = CELL_FLOOR;

  /* Sección 3: bloqueada por muros ROJOS de nuevo */
  for (int y = 1; y < rows - 1; y++) {
    game->map->data[y][15] = CELL_WALL_RED;
  }
  /* Apertura solo abajo */
  game->map->data[rows - 3][15] = CELL_FLOOR;

  /* Muros horizontales para forzar el zigzag */
  for (int x = 1; x < 5; x++) {
    game->map->data[rows / 2][x] = CELL_WALL;
  }
  for (int x = 6; x < 10; x++) {
    game->map->data[rows / 2 - 2][x] = CELL_WALL;
  }
  for (int x = 11; x < 15; x++) {
    game->map->data[rows / 2][x] = CELL_WALL;
  }

  /* Llave detrás de la segunda barrera de fase */
  allocate_item(game, ivec2(12, 3), ITEM_KEY);

  /* Muros azules alrededor de la salida para forzar paso por la puerta */
  for (int y = 1; y < rows - 1; y++) {
    if (y != rows / 2) {
      game->map->data[y][19] = CELL_WALL_BLUE;
      game->map->data[y][20] = CELL_WALL_BLUE;
    }
  }

  /* Puerta y salida */
  game->map->data[rows / 2][19] = CELL_DOOR;
  game->exit_position = ivec2(20, rows / 2);
  game->map->data[rows / 2][20] = CELL_EXIT;

  game->player.position = ivec2(2, 2);
}

/* ========== NIVEL 3: PARADOJA TEMPORAL ========== */
void load_level_3(GameState *game) {
  int rows = 14;
  int cols = 20;
  init_game_state(game, rows, cols);
  game->current_level = 2;
  snprintf(game->level_name, 64, "NIVEL 3: PARADOJA TEMPORAL");

  make_room(game);

  /* Dos botones ROJOS cercanos (distancia 6 tiles) */
  /* Botón A arriba */
  spawn_button(game, ivec2(6, 3), PHASE_RED);
  /* Botón B abajo */
  spawn_button(game, ivec2(6, 9), PHASE_RED);
  /* Distancia: |3 - 9| = 6 movimientos — cabe en el eco de 12 turnos */

  /* Barricada vertical antes de la salida */
  for (int y = 1; y < rows - 1; y++) {
    game->map->data[y][12] = CELL_BARRICADE;
  }

  /* Muros decorativos para guiar al jugador */
  for (int x = 3; x <= 9; x++) {
    game->map->data[6][x] = CELL_WALL;
  }
  game->map->data[6][6] = CELL_FLOOR; /* Paso central entre botones */

  /* Recarga de bomba como alternativa */
  allocate_item(game, ivec2(3, rows - 3), ITEM_BOMB_REFILL);
  game->player.bombs = 1;
  game->player.bomb_slots = 2;

  /* Salida detrás de la barricada */
  game->exit_position = ivec2(17, rows / 2);
  game->map->data[rows / 2][17] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
}

/* ========== NIVEL 4: COMPUERTA HADAMARD ========== */
void load_level_4(GameState *game) {
  int rows = 14;
  int cols = 20;
  init_game_state(game, rows, cols);
  game->current_level = 3;
  snprintf(game->level_name, 64, "NIVEL 4: COMPUERTA HADAMARD");

  make_room(game);

  /* Forma de "H" — la puerta Hadamard crea superposición */
  /* Pilar izquierdo de la H (rojo) */
  for (int y = 2; y < rows - 2; y++) {
    game->map->data[y][6] = CELL_WALL_RED;
  }
  /* Pilar derecho de la H (azul) */
  for (int y = 2; y < rows - 2; y++) {
    game->map->data[y][13] = CELL_WALL_BLUE;
  }
  /* Puente horizontal de la H */
  for (int x = 6; x <= 13; x++) {
    game->map->data[rows / 2][x] = CELL_WALL;
  }
  /* Aperturas fase en el puente */
  game->map->data[rows / 2][8] = CELL_WALL_RED;
  game->map->data[rows / 2][11] = CELL_WALL_BLUE;

  /*
   * REDESIGN:
   * 1. Buttons moved to Left (accessible).
   * 2. Barricade blocks path to Right.
   * 3. Key is past Barricade.
   * 4. Exit is surrounded by Doors (requires Key).
   */
  spawn_button(game, ivec2(4, 4), PHASE_RED);         /* Botón A (Izq Arriba) */
  spawn_button(game, ivec2(4, rows - 5), PHASE_BLUE); /* Botón B (Izq Abajo) */

  /* Barrera (Muro Rojo) que bloquea el paso, se abre con botones */
  for (int y = 0; y < rows; y++) {
    game->map->data[y][14] = CELL_BARRICADE;
  }
  /* Apertura central en barricada (opcional, si es muro completo, botones lo
   * borran) */
  /* Barricades are distinct from Walls. Buttons deactivate ALL barricades in
   * level. */

  /* Llave (Alejada de la salida: Esquina Sup. Izq) */
  allocate_item(game, ivec2(1, 1), ITEM_KEY);

  /* Salida rodeada de puertas */
  game->exit_position = ivec2(18, rows / 2);
  game->map->data[rows / 2][18] = CELL_EXIT;

  // Surround Exit
  game->map->data[rows / 2][17] = CELL_DOOR;     // Left
  game->map->data[rows / 2 - 1][18] = CELL_DOOR; // Top
  game->map->data[rows / 2 + 1][18] = CELL_DOOR; // Bottom
  if (19 < cols)
    game->map->data[rows / 2][19] = CELL_DOOR; // Right

  /* Coherencia */
  allocate_item(game, ivec2(3, 3), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(10, rows - 3), ITEM_COHERENCE_PICKUP);

  game->player.position = ivec2(2, rows / 2);
}

/* ========== NIVEL 5: ALGORITMO DE GROVER ========== */
/* ========== NIVEL 5: ALGORITMO DE GROVER (EXTREMO) ========== */
void load_level_5(GameState *game) {
  int rows = 16;
  int cols = 24;
  init_game_state(game, rows, cols);
  game->current_level = 4;
  snprintf(game->level_name, 64, "NIVEL 5: GROVER (EXTREMO)");

  make_room(game);

  /* Laberinto denso y claustrofóbico */
  /* Muros verticales */
  for (int y = 2; y < rows - 2; y++) {
    game->map->data[y][5] = CELL_WALL;
    game->map->data[y][10] = CELL_WALL;
    game->map->data[y][15] = CELL_WALL;
    game->map->data[y][19] = CELL_WALL;
  }
  /* Muros horizontales para cortar visión */
  for (int x = 2; x < 5; x++)
    game->map->data[5][x] = CELL_WALL;
  for (int x = 11; x < 15; x++)
    game->map->data[rows - 5][x] = CELL_WALL;

  /* Aperturas estrechas y trampas de fase */
  game->map->data[4][5] = CELL_WALL_RED;    /* Solo cruza en ROJO */
  game->map->data[12][10] = CELL_WALL_BLUE; /* Solo cruza en AZUL */
  game->map->data[6][15] = CELL_WALL_RED;
  game->map->data[10][19] = CELL_WALL_BLUE;

  /* Grid de detectores (Oráculos) */
  spawn_detector(game, ivec2(3, 3), DIR_DOWN, PHASE_RED);
  spawn_detector(game, ivec2(7, rows - 4), DIR_UP, PHASE_BLUE);
  spawn_detector(game, ivec2(12, 5), DIR_RIGHT, PHASE_RED);
  spawn_detector(game, ivec2(17, 10), DIR_LEFT, PHASE_BLUE);
  spawn_detector(game, ivec2(21, 5), DIR_DOWN, PHASE_RED);

  /* DOS Guardias patrullando en contrafase */
  spawn_guard(game, ivec2(8, 8));  /* Patrulla zona media */
  spawn_guard(game, ivec2(17, 8)); /* Patrulla zona final */

  /* Coherencia MUY escasa */
  allocate_item(game, ivec2(12, 8), ITEM_COHERENCE_PICKUP); /* Solo una! */

  /* Bomba única */
  allocate_item(game, ivec2(2, rows - 2), ITEM_BOMB_REFILL);
  game->player.bombs = 1;

  /* Llave escondida en el rincón más peligroso */
  allocate_item(game, ivec2(22, 2), ITEM_KEY);

  /* Salida */
  game->exit_position = ivec2(22, rows / 2);
  game->map->data[rows / 2][22] = CELL_EXIT;

  // Surround with Doors
  game->map->data[rows / 2][21] = CELL_DOOR;     // Left
  game->map->data[rows / 2 - 1][22] = CELL_DOOR; // Top
  game->map->data[rows / 2 + 1][22] = CELL_DOOR; // Bottom
  if (23 < cols)
    game->map->data[rows / 2][23] = CELL_DOOR; // Right

  game->player.position = ivec2(2, rows / 2);
}

/* ========== NIVEL 6: TELETRANSPORTE CUÁNTICO ========== */
/* ========== NIVEL 6: TELETRANSPORTE (EXTREMO) ========== */
void load_level_6(GameState *game) {
  int rows = 16;
  int cols = 28;
  init_game_state(game, rows, cols);
  game->current_level = 5;
  snprintf(game->level_name, 64, "NIVEL 6: TELETRANSPORTE (EXTREMO)");

  make_room(game);

  /* Tres zonas aisladas */
  for (int y = 1; y < rows - 1; y++) {
    game->map->data[y][9] = CELL_WALL;
    game->map->data[y][10] = CELL_WALL;
    game->map->data[y][18] = CELL_WALL;
    game->map->data[y][19] = CELL_WALL;
  }

  /* Túnel 1: (7,4) -> (11,7) [Zona 2] - EMBOSCADA */
  spawn_tunnel(game, ivec2(7, 4), ivec2(1, 1), ivec2(4, 3));

  /* GUARDIA ESPERANDO EN ZONA 2 (el "Comité de Bienvenida") */
  spawn_guard(game, ivec2(14, 7));

  /* Túnel 2: (16,6) -> (20,9) [Zona 3] */
  spawn_tunnel(game, ivec2(16, 6), ivec2(1, 1), ivec2(4, 3));

  /* DETECTOR EN ZONA 3 (Salida) */
  spawn_detector(game, ivec2(22, rows / 2), DIR_UP, PHASE_RED);

  /* Botón alternativo protegido por muros de fase */
  spawn_button(game, ivec2(14, 2), PHASE_RED);
  game->map->data[3][14] = CELL_WALL_BLUE; /* Bloquea acceso directo */

  /* Barricadas */
  for (int y = rows / 2 - 1; y <= rows / 2 + 1; y++) {
    game->map->data[y][18] = CELL_BARRICADE;
    game->map->data[y][19] = CELL_BARRICADE;
  }

  /* Items escasos */
  allocate_item(game, ivec2(14, rows - 2), ITEM_KEY);
  allocate_item(game, ivec2(3, 3),
                ITEM_COHERENCE_PICKUP); /* Solo uno en inicio */
  allocate_item(game, ivec2(25, 3), ITEM_COHERENCE_PICKUP); /* Uno en salida */

  game->exit_position = ivec2(26, rows / 2);
  game->map->data[rows / 2][26] = CELL_EXIT;
  // Surround with Doors
  game->map->data[rows / 2][25] = CELL_DOOR;     // Left
  game->map->data[rows / 2 - 1][26] = CELL_DOOR; // Top
  game->map->data[rows / 2 + 1][26] = CELL_DOOR; // Bottom
  if (27 < cols)
    game->map->data[rows / 2][27] = CELL_DOOR; // Right

  game->player.position = ivec2(2, rows / 2);
}

/* ========== NIVEL 7: CORRECCIÓN DE ERRORES ========== */
/* ========== NIVEL 7: CORRECCIÓN DE ERRORES (EXTREMO) ========== */
void load_level_7(GameState *game) {
  int rows = 18;
  int cols = 26;
  init_game_state(game, rows, cols);
  game->current_level = 6;
  snprintf(game->level_name, 64, "NIVEL 7: CORRECCION (EXTREMO)");

  make_room(game);

  /* Tres corredores */
  for (int x = 4; x < cols - 4; x++) {
    game->map->data[5][x] = CELL_WALL;
    game->map->data[12][x] = CELL_WALL;
  }
  /* Aperturas */
  game->map->data[5][4] = CELL_FLOOR;
  game->map->data[12][4] = CELL_FLOOR;
  game->map->data[5][cols - 5] = CELL_FLOOR;
  game->map->data[12][cols - 5] = CELL_FLOOR;

  /* Corredor SUPERIOR: Muros de fase + GUARDIA */
  for (int x = 7; x < cols - 5; x += 3)
    game->map->data[3][x] = CELL_WALL_RED;

  // FIX: Remove wall at 16 to give guard space
  game->map->data[3][16] = CELL_FLOOR;

  spawn_guard(game, ivec2(16, 3)); /* Guardia con espacio (15-17 libre) */

  /* Corredor MEDIO: GUARDIA + Detector */
  spawn_guard(game, ivec2(12, 8));
  spawn_detector(game, ivec2(18, 8), DIR_LEFT, PHASE_BLUE);

  /* Corredor INFERIOR: DETECTORES CRUZADOS (Bullet Hell) */
  spawn_detector(game, ivec2(6, 14), DIR_RIGHT, PHASE_RED);
  spawn_detector(game, ivec2(19, 14), DIR_LEFT, PHASE_BLUE);
  spawn_detector(game, ivec2(12, 16), DIR_UP, PHASE_RED);

  /* Botón (necesario) */
  spawn_button(game, ivec2(cols - 6, 3), PHASE_RED);

  /* Barrera final */
  for (int y = 0; y < rows; y++)
    game->map->data[y][cols - 3] = CELL_BARRICADE;

  /* Coherencia mínima */
  allocate_item(game, ivec2(6, 8), ITEM_COHERENCE_PICKUP);   /* Uno al inicio */
  allocate_item(game, ivec2(18, 14), ITEM_COHERENCE_PICKUP); /* Uno peligroso */

  /* Bombas MUY escasas (1 pack) */
  allocate_item(game, ivec2(4, 9), ITEM_BOMB_REFILL);
  game->player.bombs = 1;

  game->exit_position = ivec2(cols - 2, rows / 2);
  game->map->data[rows / 2][cols - 2] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
}

/* ========== NIVEL 8: SUPREMACÍA CUÁNTICA ========== */
/* ========== NIVEL 8: SUPREMACÍA (BOSS RUSH) ========== */
void load_level_8(GameState *game) {
  int rows = 20;
  int cols = 30;
  init_game_state(game, rows, cols);
  game->current_level = 7;
  snprintf(game->level_name, 64, "NIVEL 8: BOSS RUSH");

  make_room(game);

  /* ZONA 1: The Gauntlet */
  /* Laberinto de fase + Detector + Guardia */
  for (int y = 3; y < 9; y++) {
    for (int x = 4; x < 10; x++) {
      if ((x + y) % 3 == 0)
        game->map->data[y][x] = CELL_WALL_RED;
      else if ((x + y) % 3 == 1)
        game->map->data[y][x] = CELL_WALL_BLUE;
    }
  }
  /* Pasillo libre mínimo */
  game->map->data[4][6] = CELL_FLOOR;
  game->map->data[7][6] = CELL_FLOOR;

  /* Guardia patrullando Zona 1 */
  spawn_guard(game, ivec2(8, 8));

  /* Muro Zona 1->2 */
  for (int y = 1; y < rows - 1; y++)
    game->map->data[y][11] = CELL_WALL;
  game->map->data[6][11] = CELL_FLOOR; /* Apertura vigilada */

  /* ZONA 2: The Kill Box */
  /* 4 DETECTORES cubriendo el área central */
  spawn_detector(game, ivec2(12, 2), DIR_DOWN, PHASE_RED);
  spawn_detector(game, ivec2(12, rows - 3), DIR_UP, PHASE_BLUE);
  spawn_detector(game, ivec2(20, 2), DIR_DOWN, PHASE_BLUE);
  spawn_detector(game, ivec2(20, rows - 3), DIR_UP, PHASE_RED);

  /* GUARDIA patrullando botones */
  spawn_guard(game, ivec2(16, rows / 2));

  /* Botones (Eco requerido) */
  spawn_button(game, ivec2(16, rows / 2 - 4), PHASE_RED);
  spawn_button(game, ivec2(16, rows / 2 + 4), PHASE_BLUE);

  /* Muro Zona 2->3 (Barricada) */
  for (int y = 1; y < rows - 1; y++)
    game->map->data[y][21] = CELL_BARRICADE;

  /* ZONA 3: The Escape */
  /* Tunel para atravesar muro final */
  spawn_tunnel(game, ivec2(24, 6), ivec2(1, 1),
               ivec2(4, 3)); /* Teleporta a 28 (safe) */

  /* Muro final */
  for (int y = 5; y < 10; y++)
    game->map->data[y][27] = CELL_WALL;

  /* Llave y Salida */
  allocate_item(game, ivec2(28, 7), ITEM_KEY);

  /* ULTIMO GUARDIA custodiando la salida */
  spawn_guard(game, ivec2(25, rows - 4));

  /* Coherencia CRITICA (solo 2 para todo el nivel) */
  allocate_item(game, ivec2(2, 2), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(16, rows / 2), ITEM_COHERENCE_PICKUP);

  /* Bombas estándar (necesarias) */
  allocate_item(game, ivec2(12, rows / 2), ITEM_BOMB_REFILL);
  game->player.bombs = 2;

  game->exit_position = ivec2(28, rows / 2);
  game->map->data[rows / 2][28] = CELL_EXIT;

  // Surround with Doors
  game->map->data[rows / 2][27] = CELL_DOOR;     // Left
  game->map->data[rows / 2 - 1][28] = CELL_DOOR; // Top
  game->map->data[rows / 2 + 1][28] = CELL_DOOR; // Bottom
  if (29 < cols)
    game->map->data[rows / 2][29] = CELL_DOOR; // Right

  game->player.position = ivec2(2, rows / 2);
}

/* ========== NIVEL 9: COMPUERTA TOFFOLI ========== */
void load_level_9(GameState *game) {
  int rows = 16;
  int cols = 24;
  init_game_state(game, rows, cols);
  game->current_level = 8;
  snprintf(game->level_name, 64, "NIVEL 9: COMPUERTA TOFFOLI");

  make_room(game);

  // Phase mechanics: Start with Green LOCKED
  game->player.phase_system.green_unlocked = false;

  // Zone 1: Red/Blue Puzzle to get Unlocker
  for (int x = 8; x < 16; x++) {
    game->map->data[5][x] = CELL_WALL_RED;
    game->map->data[10][x] = CELL_WALL_BLUE;
  }

  // Item to unlock Green
  allocate_item(game, ivec2(12, 8), ITEM_PHASE_UNLOCKER);

  // Exit Zone protected by Green Walls
  for (int y = 0; y < rows; y++) {
    game->map->data[y][18] = CELL_WALL_GREEN;
  }
  game->map->data[rows / 2][18] =
      CELL_WALL_GREEN; // Make sure it's blocked by green

  // Use a button mechanism?
  // Maybe Control Qubits logic simulated with buttons
  spawn_button(game, ivec2(4, 4), PHASE_RED);
  spawn_button(game, ivec2(4, rows - 4), PHASE_BLUE);

  // Gate that requires Green Phase to pass?
  // Actually simplest is just wall_green.
  // If player gets unlocker, they can switch to Green and pass.

  game->exit_position = ivec2(22, rows / 2);
  game->map->data[rows / 2][22] = CELL_EXIT;
  game->player.position = ivec2(2, rows / 2);
}

/* ========== NIVEL 10: TELEPORTACION ========== */
void load_level_10(GameState *game) {
  int rows = 14;
  int cols = 20;
  init_game_state(game, rows, cols);
  game->current_level = 9;
  snprintf(game->level_name, 64, "NIVEL 10: TELEPORTACION");

  make_room(game);

  /* Dos islas separadas por un muro */
  for (int y = 0; y < rows; y++)
    game->map->data[y][10] = CELL_WALL;

  /* Portal ROJO en isla 1 (izquierda) -> lleva a isla 2 */
  spawn_portal(game, ivec2(7, rows / 2), 1, PHASE_RED);

  /* Portal AZUL en isla 2 (derecha) -> devuelve a isla 1 */
  spawn_portal(game, ivec2(13, rows / 2), 0, PHASE_BLUE);

  /* Llave en isla 2 */
  allocate_item(game, ivec2(15, 3), ITEM_KEY);

  /* REDISEÑO:
   * 1. Botones a la Izquierda (accesibles).
   * 2. Barricada bloquea camino a Derecha.
   * 3. Llave pasada la Barricada.
   * 4. Salida rodeada de Puertas (requiere Llave).
   */
  /* Puerta y salida en isla 1 */
  game->map->data[rows / 2][18] = CELL_DOOR;
  game->exit_position = ivec2(18, rows / 2);
  game->map->data[rows / 2][18] = CELL_DOOR;
  game->exit_position = ivec2(19, rows / 2);
  game->map->data[rows / 2][19] = CELL_EXIT;

  /* Jugador empieza en isla 1 */
  game->player.position = ivec2(2, rows / 2);

  /* Desbloquear fases necesarias */
  game->player.phase_system.green_unlocked = false;
}

/* ========== FUNCIONES DEL SISTEMA ========== */

void load_level(GameState *game, int level_index) {
  if (level_index == 0) {
    init_encyclopedia(game);
  }

  // Reiniciar Estadísticas de Nivel
  // Reiniciar Estadísticas de Nivel SOLO si es un inicio fresco de código de
  // nivel? Usuario quiere que estadísticas SE ACUMULEN incluso si mueren y
  // reinician. Así que NO deberíamos reiniciar muertes/mediciones/etc aquí.
  // Pero espera, si inician un NUEVO juego, estadísticas deben ser 0.
  // initialized pone a cero todo el GameState, así que estadísticas empiezan en
  // 0. load_level es llamado durante el juego. Si comentamos esto, estadísticas
  // persistirán entre niveles Y reinicios. Solo necesitamos reiniciar
  // steps_taken y level_time para la ejecución actual ¿funcionalidad?
  // Reiniciemos progreso transitorio de nivel, pero mantenemos "vida"
  // estadísticas.

  game->player.steps_taken = 0;
  game->player.level_time = 0.0;
  // game->player.measurements_made // MANTENER
  // game->player.entanglements_created // MANTENER
  // game->player.phase_shifts // MANTENER
  // game->player.deaths // MANTENER
  game->player.qubit_count = 0; // Items reiniciados
  game->player.keys = 0;        // Items reiniciados
  game->player.bombs = 0;       // Items reiniciados

  switch (level_index) {
  case 0:
    load_level_1(game);
    break;
  case 1:
    load_level_2(game);
    break;
  case 2:
    load_level_3(game);
    break;
  case 3:
    load_level_4(game);
    break;
  case 4:
    load_level_5(game);
    break;
  case 5:
    load_level_6(game);
    break;
  case 6:
    load_level_7(game);
    break;
  case 7:
    load_level_8(game);
    break;
  case 8:
    load_level_9(game);
    break;
  case 9:
    load_level_10(game);
    break;
  case 10:
    load_level_11(game);
    break;
  case 11:
    load_level_12(game);
    break;
  case 12:
    load_level_13(game);
    break;
  case 13:
    load_level_14(game);
    break;
  case 14:
    load_level_15(game);
    break;
  case 15:
    load_level_16(game);
    break;
  case 16:
    load_level_17(game);
    break;
  case 17:
    load_level_18(game);
    break;
  case 18:
    load_level_19(game);
    break;
  case 19:
    load_level_20(game);
    break;
  default:
    load_level_1(game);
    break;
  }
}

void init_intro_dialogs(GameState *game) {
  DialogSystem *d = &game->dialog;
  d->page_count = 5;
  d->current_page = 0;
  d->active = true;

  strncpy(d->pages[0].title, "INICIALIZANDO...", 64);
  strncpy(d->pages[0].text,
          "Sujeto 44 inicializado.\nCoherencia cuantica: 100%.\n\n"
          "Objetivo: Atravesar las instalaciones.\n"
          "Estado: Varianza de fase detectada.",
          MAX_DIALOG_TEXT);

  strncpy(d->pages[1].title, "MECANICAS DE FASE", 64);
  strncpy(d->pages[1].text,
          "Existes en un estado de fase dual.\n\n"
          "FASE ROJA atraviesa muros AZULES.\n"
          "FASE AZUL atraviesa muros ROJOS.\n\n"
          "Pulsa [Z] para cambiar de fase.",
          MAX_DIALOG_TEXT);

  strncpy(d->pages[2].title, "SUPERPOSICION", 64);
  strncpy(d->pages[2].text,
          "Pulsa [ESPACIO] para entrar en SUPERPOSICION.\n\n"
          "Esto divide tu linea temporal, creando un\n"
          "ECO temporal que repite tus acciones.\n\n"
          "Las probabilidades cuanticas se calculan\n"
          "en un servidor ejecutando IBM Qiskit.",
          MAX_DIALOG_TEXT);

  strncpy(d->pages[3].title, "ENTIDADES", 64);
  strncpy(d->pages[3].text,
          "Evita el contacto directo con los colapsores.\n\n"
          "Son anomalias cuanticas inestables.\n"
          "La observacion colapsa su funcion de onda.",
          MAX_DIALOG_TEXT);

  strncpy(d->pages[4].title, "ADVERTENCIA", 64);
  strncpy(d->pages[4].text,
          "Vigila tu medidor de COHERENCIA.\n"
          "Si llega al 0%, te disolveras.\n\n"
          "CONEXION QISKIT: ESTABLE\n"
          "Los tuneles y la superposicion usan circuitos\n"
          "cuanticos reales (IBM Qiskit) para calcular\n"
          "las probabilidades de colapso.\n\n"
          "Buena suerte, Sujeto 44.\n\n"
          "\n[WASD] Movimiento\n"
          "[Z] Cambia de fase\n"
          "[ESPACIO] Inicia superposicion\n"
          "[E] Empieza entrelazamiento\n"
          "[X] Pone una bomba\n"
          "[ESC] Cierra el juego",
          MAX_DIALOG_TEXT);
}

void show_level_dialog(GameState *game) {
  DialogSystem *d = &game->dialog;
  d->active = true;
  d->current_page = 0;
  d->page_count = 1;

  snprintf(d->pages[0].title, 64, "%s", game->level_name);

  switch (game->current_level) {
  case 0:
    strncpy(d->pages[0].text,
            "PATRON DE INTERFERENCIA CUANTICA\n\n"
            "Los muros forman un patron de interferencia.\n"
            "ROJO bloquea ROJO. AZUL bloquea AZUL.\n\n"
            "Cambia con [Z] para navegar el patron.",
            MAX_DIALOG_TEXT);
    break;
  case 1:
    strncpy(d->pages[0].text,
            "ZIGZAG DE FASE\n\n"
            "Tres barreras verticales de diferente fase\n"
            "bloquean el camino.\n\n"
            "ROJA - AZUL - ROJA\n\n"
            "Cambia de fase con [Z] para atravesarlas.\n"
            "Recuerda: cada fase solo pasa muros del\n"
            "color OPUESTO.",
            MAX_DIALOG_TEXT);
    break;
  case 2:
    strncpy(d->pages[0].text,
            "LINEAS TEMPORALES PARALELAS\n\n"
            "Dos botones ROJOS. Una barricada.\n"
            "Ambos deben pulsarse A LA VEZ.\n\n"
            "ESTRATEGIA:\n"
            "1. Ve al boton de ARRIBA (en fase ROJA)\n"
            "2. Pulsa [ESPACIO] para Superposicion\n"
            "3. Usa [T] para ESPERAR 3-4 turnos\n"
            "4. Corre al boton de ABAJO\n"
            "5. Tu eco repite tus pasos y pulsa el de arriba",
            MAX_DIALOG_TEXT);
    break;
  case 3:
    strncpy(d->pages[0].text,
            "COMPUERTA HADAMARD\n\n"
            "Dos botones activan el muro rojo.\n"
            "La LLAVE esta en la esquina sup-izq.\n\n"
            "1. Ve a hacia los botones\n"
            "2. Activa SUPERPOSICION [ESPACIO] sobre un boton\n"
            "3. Espera [T] y corre al otro boton\n"
            "4. El muro rojo se abrira\n"
            "5. Recoge la llave y sal por la derecha.",
            MAX_DIALOG_TEXT);
    break;
  case 4:
    strncpy(d->pages[0].text,
            "ALGORITMO DE GROVER\n\n"
            "Laberinto denso.\n"
            "La salida esta confinada por puertas azules.\n\n"
            "- Cambia fase con [Z] para cruzar aperturas de color\n"
            "- Detector ROJO solo ve fase ROJA\n"
            "- Detector AZUL solo ve fase AZUL\n"
            "- Cambia a fase OPUESTA del detector\n"
            "- Guardia: esquiva o elimina con bombas [X]\n\n",
            MAX_DIALOG_TEXT);
    break;
  case 5:
    strncpy(d->pages[0].text,
            "TELETRANSPORTE CUANTICO\n\n"
            "Tres zonas aisladas. Solo TUNELES CUANTICOS\n"
            "(zonas purpuras) conectan las zonas.\n\n"
            "COMO TUNELIZAR (Probabilidad Qiskit):\n"
            "1. Entra en la zona PURPURA\n"
            "2. Activa SUPERPOSICION [ESPACIO]\n"
            "3. 50% de exito calculado por circuito cuantico\n\n"
            "OJO: Un guardia espera en la zona 2.\n"
            "La salida esta rodeada de puertas azules.",
            MAX_DIALOG_TEXT);
    break;
  case 6:
    strncpy(d->pages[0].text,
            "CORRECCION DE ERRORES CUANTICOS\n\n"
            "La redundancia protege la informacion cuantica.\n"
            "SUPERIOR: Muros de fase (cambia con [Z])\n"
            "MEDIO: Guardia patrullando (usa bombas [X])\n"
            "INFERIOR: Detectores (evita su fase)\n\n"
            "El boton arriba abre la barrera final.\n"
            "Recoge coherencia en todos los corredores.",
            MAX_DIALOG_TEXT);
    break;
  case 7:
    strncpy(d->pages[0].text,
            "SUPREMACIA CUANTICA\n\n"
            "Todas las mecanicas combinadas.\n"
            "La salida esta fuertemente protegida.\n\n"
            "ZONA 1: Laberinto de fase (cambia fase)\n"
            "ZONA 2: Detectores + 2 botones (usa eco)\n"
            "ZONA 3: Tunel cuantico + llave\n\n"
            "TIP: Usa la BOMBA [X] para romper el muro\n"
            "que encierra al enemigo si se queda atascado.\n"
            "- Cruza el laberinto alternando fases\n"
            "- Usa un ECO para un boton\n"
            "- Tuneliza para alcanzar la llave final\n\n"
            "Buena suerte, Sujeto 44.",
            MAX_DIALOG_TEXT);
    break;
  case 8:
    strncpy(d->pages[0].text,
            "COMPUERTA TOFFOLI (CCNOT)\n\n"
            "Dos botones controlan la salida.\n"
            "Ambos deben estar PULSADOS a la vez\n"
            "para abrir los muros VERDES.\n\n"
            "ESTRATEGIA:\n"
            "1. Recoge el DESBLOQUEO DE FASE VERDE\n"
            "2. Ve al primer boton\n"
            "3. Activa SUPERPOSICION [ESPACIO]\n"
            "4. Espera [T] turnos, corre al otro boton\n"
            "5. Tu eco mantiene el primero pulsado",
            MAX_DIALOG_TEXT);
    break;
  case 9:
    strncpy(d->pages[0].text,
            "TELEPORTACION CUANTICA\n\n"
            "Dos islas separadas por un muro.\n"
            "Los PORTALES son la unica conexion.\n\n"
            "CLAVE: Cada portal tiene un COLOR.\n"
            "Para usarlo, tu FASE debe coincidir:\n"
            "- Portal ROJO: cambia a fase ROJA [Z]\n"
            "- Portal AZUL: cambia a fase AZUL [Z]\n\n"
            "1. Pisa el portal ROJO -> isla 2\n"
            "2. Recoge la LLAVE\n"
            "3. Cambia a AZUL, pisa portal AZUL\n"
            "4. Abre la PUERTA con la llave y sal.",
            MAX_DIALOG_TEXT);
    break;
  case 10:
    strncpy(d->pages[0].text,
            "LABERINTO DE FASE\n\n"
            "Muros ROJOS y AZULES bloquean el paso.\n"
            "Cambia de FASE con [Z] para\n"
            "atravesar los muros de tu mismo color.\n\n"
            "ESTRATEGIA:\n"
            "1. Busca las aperturas en cada muro\n"
            "2. Evita al guardia del centro\n"
            "3. Recoge la LLAVE y abre la PUERTA\n\n"
            "Recoge COHERENCIA si la necesitas.",
            MAX_DIALOG_TEXT);
    break;
  case 11:
    strncpy(d->pages[0].text,
            "ECO Y GUARDIA\n\n"
            "Un muro de barricadas bloquea el paso.\n"
            "El BOTON abre la barricada.\n\n"
            "ESTRATEGIA:\n"
            "1. Pisa el BOTON para abrir el muro\n"
            "2. Un GUARDIA patrulla al otro lado\n"
            "3. Usa una BOMBA [X] para eliminarlo\n"
            "4. Recoge la LLAVE y abre la PUERTA\n\n"
            "Tienes bombas disponibles.\n"
            "Presionando [E] puedes ENTRELAZAR con un\n"
            "GUARDIA, copia 6 movimientos",
            MAX_DIALOG_TEXT);
    break;
  case 12:
    strncpy(d->pages[0].text,
            "DOS BOTONES\n\n"
            "Dos botones deben estar pulsados\n"
            "A LA VEZ para abrir la barricada.\n\n"
            "ESTRATEGIA:\n"
            "1. Ve al primer boton\n"
            "2. Activa SUPERPOSICION [ESPACIO]\n"
            "3. Graba tu camino al segundo boton\n"
            "4. Tu ECO mantiene el primero pulsado\n\n",
            MAX_DIALOG_TEXT);
    break;
  case 13:
    strncpy(d->pages[0].text,
            "PORTALES Y FASE\n\n"
            "Tres secciones conectadas por PORTALES.\n"
            "Necesitas la LLAVE de la seccion 2.\n\n"
            "ESTRATEGIA:\n"
            "1. Usa portal ROJO -> seccion 2\n"
            "2. Atraviesa muro AZUL con fase AZUL\n"
            "3. Recoge LLAVE, esquiva al guardia\n"
            "4. Usa portal AZUL -> seccion 3\n"
            "5. Abre PUERTA y sal.\n\n"
            "Cambia de fase con [Z].",
            MAX_DIALOG_TEXT);
    break;
  case 14:
    strncpy(d->pages[0].text,
            "PRUEBA FINAL\n\n"
            "Combina todo lo aprendido:\n"
            "Muros de fase, botones y guardias.\n\n"
            "ESTRATEGIA:\n"
            "1. Cruza los muros ROJO y AZUL [Z]\n"
            "2. Pisa boton 1, activa SUPERPOSICION\n"
            "3. Corre al boton 2 (5 casillas)\n"
            "4. La barricada se abrira\n"
            "5. Elimina al guardia con BOMBA [X]\n"
            "6. Recoge LLAVE y abre PUERTA.\n\n"
            "Usa todo lo que has aprendido!",
            MAX_DIALOG_TEXT);
    break;
  case 15:
    strncpy(d->pages[0].text,
            "GROVER II (SUPERPOSICION)\n\n"
            "Muros de fase y detectores bloquean el camino.\n"
            "Algunos muros solo son pasables en una fase.\n\n"
            "ESTRATEGIA:\n"
            "1. Observa el patron de los detectores\n"
            "2. Usa SUPERPOSICION para probar caminos seguros\n"
            "3. Cambia de fase [Z] segun el muro que cruces\n"
            "4. La salida requiere una llave.",
            MAX_DIALOG_TEXT);
    break;
  case 16:
    strncpy(d->pages[0].text,
            "ENTRELAZAMIENTO CUANTICO\n\n"
            "El guardia esta encerrado pero tiene el BOTON.\n"
            "Tu estas fuera pero tienes la SALIDA.\n\n"
            "ESTRATEGIA:\n"
            "1. Acercate al guardia y pulsa [E] para ENTRELAZAR\n"
            "2. Muevete: el guardia copiara tus movimientos\n"
            "3. Guialo hacia el boton rojo\n"
            "4. EL PORTAL SE ACTIVARA (Brillara)\n"
            "5. Mueve al guardia fuera del portal\n"
            "6. Entra en la jaula (Usa fase [Z]) y pisa el portal.\n\n"
            "Usa COHERENCIA para mantener el vinculo.",
            MAX_DIALOG_TEXT);
    break;
  case 17:
    strncpy(d->pages[0].text,
            "BUSQUEDA DE ORACULO\n\n"
            "Cuatro camaras. Una llave.\n"
            "Detectores vigilan el cruce central.\n\n"
            "ESTRATEGIA:\n"
            "1. Identifica la camara con la llave\n"
            "2. Usa SUPERPOSICION para explorar sin riesgo\n"
            "3. Cuidado con el guardia custodiando la llave\n"
            "4. Vuelve al centro y sal por abajo.",
            MAX_DIALOG_TEXT);
    break;
  case 18:
    strncpy(d->pages[0].text,
            "Dos botones dispersos.\n"
            "Muros de fase oscilantes.\n"
            "Guardias en patrulla.\n\n"
            "MISION:\n"
            "Activa los DOS botones para abrir la barricada final.\n"
            "Necesitas la LLAVE AZUL para la puerta de salida.\n\n"
            "Usa todo: Eco, Entrelazamiento, Bombas.",
            MAX_DIALOG_TEXT);
    break;
  case 19:
    strncpy(d->pages[0].text,
            "Bienvenido al nucleo del procesador.\n"
            "INICIO DE SISTEMA: FASE AZUL FORZADA.\n\n"
            "OBJETIVO: Recolectar 3 LLAVES de seguridad.\n"
            "AMENAZAS:\n"
            "- Lasers de Alta Energia (-40 Coherencia)\n"
            "- Red de Fase Compleja\n"
            "- Multiples Guardias de Seguridad\n\n"
            "Usa TUNELES y PORTALES para navegar las islas.\n"
            "Esta es la prueba final de tu estado cuantico.\n",
            MAX_DIALOG_TEXT);
    break;
  default:
    strncpy(d->pages[0].text, "Proceda con precaucion.", MAX_DIALOG_TEXT);
    break;
  }
}

void check_level_events(GameState *game) {
  /* NIVEL 3: Ambos botones deben estar pulsados para abrir barricada col 12 */
  if (game->current_level == 2) {
    bool all_pressed = true;
    int active_count = 0;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active) {
        active_count++;
        if (!game->buttons[i].is_pressed)
          all_pressed = false;
      }
    }

    if (active_count > 0 && all_pressed) {
      int gate_col = 12;
      bool was_closed = false;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][gate_col] == CELL_BARRICADE) {
          game->map->data[y][gate_col] = CELL_FLOOR;
          was_closed = true;
        }
      }
      if (was_closed) {
        PlayAudioSound(phase_shift_sound);
      }
    }
  }

  /* NIVEL 4 (HADAMARD): Ambos botones abren barrera col 21 */
  if (game->current_level == 3) {
    bool all_pressed = true;
    int active_count = 0;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active) {
        active_count++;
        if (!game->buttons[i].is_pressed)
          all_pressed = false;
      }
    }

    if (active_count > 0 && all_pressed) {
      int gate_col = 14; /* FIXED: Was 17, now bar is at 14 */
      bool was_closed = false;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][gate_col] == CELL_BARRICADE) {
          game->map->data[y][gate_col] = CELL_FLOOR;
          was_closed = true;
        }
      }
      if (was_closed) {
        PlayAudioSound(phase_shift_sound);
      }
    }
  }

  /* NIVEL 6 (TELETRANSPORTE): Botón abre barricadas en muros zona 2→3 */
  if (game->current_level == 5) {
    bool button_pressed = false;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active && game->buttons[i].is_pressed) {
        button_pressed = true;
        break;
      }
    }

    if (button_pressed) {
      for (int y = game->map->rows / 2 - 1; y <= game->map->rows / 2 + 1; y++) {
        if (game->map->data[y][18] == CELL_BARRICADE) {
          game->map->data[y][18] = CELL_FLOOR;
        }
        if (game->map->data[y][19] == CELL_BARRICADE) {
          game->map->data[y][19] = CELL_FLOOR;
        }
      }
      PlayAudioSound(phase_shift_sound);
    }
  }

  /* NIVEL 7 (CORRECCIÓN ERRORES): Botón abre barrera col (cols-3) */
  if (game->current_level == 6) {
    bool button_pressed = false;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active && game->buttons[i].is_pressed) {
        button_pressed = true;
        break;
      }
    }

    if (button_pressed) {
      int gate_col = game->map->cols - 3;
      bool was_closed = false;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][gate_col] == CELL_BARRICADE) {
          game->map->data[y][gate_col] = CELL_FLOOR;
          was_closed = true;
        }
      }
      if (was_closed) {
        PlayAudioSound(phase_shift_sound);
      }
    }
  }

  /* NIVEL 9 (TOFFOLI): Ambos botones abren muro verde col 18 */
  if (game->current_level == 8) {
    bool all_pressed = true;
    int active_count = 0;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active) {
        active_count++;
        if (!game->buttons[i].is_pressed)
          all_pressed = false;
      }
    }

    if (active_count > 0 && all_pressed) {
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][18] == CELL_WALL_GREEN) {
          game->map->data[y][18] = CELL_FLOOR;
        }
      }
      PlayAudioSound(phase_shift_sound);
    }
  }

  /* NIVEL 8 (SUPREMACÍA): Ambos botones abren barrera col 21 */
  if (game->current_level == 7) {
    bool all_pressed = true;
    int active_count = 0;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active) {
        active_count++;
        if (!game->buttons[i].is_pressed)
          all_pressed = false;
      }
    }

    if (active_count > 0 && all_pressed) {
      int gate_col = 21;
      bool was_closed = false;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][gate_col] == CELL_BARRICADE) {
          game->map->data[y][gate_col] = CELL_FLOOR;
          was_closed = true;
        }
      }
      if (was_closed) {
        PlayAudioSound(phase_shift_sound);
      }
    }
  }

  /* NIVEL 11 (LABERINTO DE FASE): No button events - key+door only */

  /* NIVEL 12 (ECO Y GUARDIA): Button opens barricade wall col 9 */
  if (game->current_level == 11) {
    bool button_pressed = false;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active && game->buttons[i].is_pressed) {
        button_pressed = true;
        break;
      }
    }
    if (button_pressed) {
      bool was_closed = false;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][9] == CELL_BARRICADE) {
          game->map->data[y][9] = CELL_FLOOR;
          was_closed = true;
        }
      }
      if (was_closed)
        PlayAudioSound(phase_shift_sound);
    }
  }

  /* NIVEL 13 (DOS BOTONES): Both buttons open barricade col 14 */
  if (game->current_level == 12) {
    bool all_pressed = true;
    int active_count = 0;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active) {
        active_count++;
        if (!game->buttons[i].is_pressed)
          all_pressed = false;
      }
    }
    if (active_count > 0 && all_pressed) {
      bool was_closed = false;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][14] == CELL_BARRICADE) {
          game->map->data[y][14] = CELL_FLOOR;
          was_closed = true;
        }
      }
      if (was_closed)
        PlayAudioSound(phase_shift_sound);
    }
  }

  /* NIVEL 14 (PORTALES Y FASE): No button events - portals+key only */

  /* NIVEL 15 (PRUEBA FINAL): Both buttons open barricade col 17 */
  if (game->current_level == 14) {
    bool all_pressed = true;
    int active_count = 0;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active) {
        active_count++;
        if (!game->buttons[i].is_pressed)
          all_pressed = false;
      }
    }
    if (active_count > 0 && all_pressed) {
      bool was_closed = false;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][17] == CELL_BARRICADE) {
          game->map->data[y][17] = CELL_FLOOR;
          was_closed = true;
        }
      }
      if (was_closed)
        PlayAudioSound(phase_shift_sound);
    }
  }

  /* NIVEL 15 (PRUEBA FINAL): Dos botones abren la salida */
  if (game->current_level == 14) {
    bool all_pressed = true;
    int active_count = 0;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active) {
        active_count++;
        if (!game->buttons[i].is_pressed)
          all_pressed = false;
      }
    }
    if (active_count > 0 && all_pressed) {
      int gate_col = game->map->cols - 2;
      bool was_closed = false;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][gate_col] == CELL_BARRICADE) {
          game->map->data[y][gate_col] = CELL_FLOOR;
          was_closed = true;
        }
      }
      if (was_closed)
        PlayAudioSound(phase_shift_sound);
    }
  }

  /* NIVEL 17 (ENTRELAZAMIENTO): Boton abre salida */
  if (game->current_level == 16) {
    bool any_pressed = false;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active && game->buttons[i].is_pressed) {
        any_pressed = true;
        break;
      }
    }
    if (any_pressed) {
      // Activate Portal in Guard Cage
      for (int i = 0; i < MAX_PORTALS; i++) {
        if (game->portals[i].position.x == 17 &&
            game->portals[i].position.y == 7) {
          game->portals[i].active = true;
          game->portals[i].glow_intensity = 2.0f;
        }
      }
      PlayAudioSound(phase_shift_sound);
    }
  }

  /* NIVEL 19 (EJECUCION FINAL): 3 botones abren la compuerta final */
  if (game->current_level == 18) {
    bool all_pressed = true;
    int active_count = 0;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active) {
        active_count++;
        if (!game->buttons[i].is_pressed)
          all_pressed = false;
      }
    }
    if (active_count > 0 && all_pressed) {
      int gate_col = game->map->cols - 4;
      bool was_closed = false;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][gate_col] == CELL_BARRICADE) {
          game->map->data[y][gate_col] = CELL_FLOOR;
          was_closed = true;
        }
      }
      if (was_closed)
        PlayAudioSound(phase_shift_sound);
    }
  }
}

void load_level_11(GameState *game) {
  int rows = 14;
  int cols = 20;
  init_game_state(game, rows, cols);
  snprintf(game->level_name, 64, "NIVEL 11: LABERINTO DE FASE");
  game->current_level = 10;
  make_room(game);

  /* Laberinto con muros alternados rojo/azul */
  for (int y = 2; y < rows - 2; y++) {
    game->map->data[y][5] = CELL_WALL_RED;
    game->map->data[y][10] = CELL_WALL_BLUE;
    game->map->data[y][15] = CELL_WALL_RED;
  }
  /* Aperturas en posiciones alternas */
  game->map->data[3][5] = CELL_FLOOR;
  game->map->data[rows - 4][10] = CELL_FLOOR;
  game->map->data[5][15] = CELL_FLOOR;

  /* Guardia patrullando la zona central */
  spawn_guard(game, ivec2(8, rows / 2));

  /* Llave escondida */
  allocate_item(game, ivec2(12, 3), ITEM_KEY);

  /* Coherencia */
  allocate_item(game, ivec2(7, rows - 3), ITEM_COHERENCE_PICKUP);

  /* Puerta y salida */
  game->map->data[rows / 2][18] = CELL_DOOR;
  game->exit_position = ivec2(18, rows / 2);
  game->map->data[rows / 2][18] = CELL_DOOR;
  game->exit_position = ivec2(19, rows / 2);
  game->map->data[rows / 2][19] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
}

void load_level_12(GameState *game) {
  int rows = 14;
  int cols = 18;
  init_game_state(game, rows, cols);
  snprintf(game->level_name, 64, "NIVEL 12: ECO Y GUARDIA");
  game->current_level = 11;
  make_room(game);

  /* Muro central con barricada */
  for (int y = 0; y < rows; y++)
    game->map->data[y][9] = CELL_BARRICADE;

  /* Boton que abre la barricada (ROJO) */
  spawn_button(game, ivec2(4, rows / 2), PHASE_RED);

  /* Guardia al otro lado */
  spawn_guard(game, ivec2(13, rows / 2));

  /* Bomba para eliminar guardia */
  allocate_item(game, ivec2(3, 3), ITEM_BOMB_REFILL);
  game->player.bombs = 1;
  game->player.bomb_slots = 2;

  /* Llave detras del guardia */
  allocate_item(game, ivec2(15, 3), ITEM_KEY);

  /* Puerta y salida */
  game->map->data[rows / 2][16] = CELL_DOOR;
  game->exit_position = ivec2(17, rows / 2);
  game->map->data[rows / 2][17] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
}

void load_level_13(GameState *game) {
  int rows = 14;
  int cols = 18;
  init_game_state(game, rows, cols);
  snprintf(game->level_name, 64, "NIVEL 13: DOS BOTONES");
  game->current_level = 12;
  make_room(game);

  /* Dos botones CERCANOS (distancia 6) */
  spawn_button(game, ivec2(10, 4), PHASE_RED);
  spawn_button(game, ivec2(10, rows - 5), PHASE_BLUE);

  /* Barricada que bloquea salida */
  for (int y = 0; y < rows; y++)
    game->map->data[y][14] = CELL_BARRICADE;

  /* Muros de fase para complicar acceso */
  for (int y = 2; y < rows - 2; y++)
    game->map->data[y][7] = CELL_WALL_RED;
  game->map->data[rows / 2][7] = CELL_FLOOR;

  /* Llave */
  allocate_item(game, ivec2(12, rows / 2), ITEM_KEY);

  /* Coherencia */
  allocate_item(game, ivec2(4, 3), ITEM_COHERENCE_PICKUP);

  /* Puerta y salida */
  game->map->data[rows / 2][16] = CELL_DOOR;
  game->exit_position = ivec2(17, rows / 2);
  game->map->data[rows / 2][17] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
}

void load_level_14(GameState *game) {
  int rows = 14;
  int cols = 22;
  init_game_state(game, rows, cols);
  snprintf(game->level_name, 64, "NIVEL 14: PORTALES Y FASE");
  game->current_level = 13;
  make_room(game);

  /* Tres secciones separadas por muros */
  for (int y = 0; y < rows; y++) {
    game->map->data[y][7] = CELL_WALL;
    game->map->data[y][14] = CELL_WALL;
  }

  /* Portal ROJO en seccion 1 -> seccion 2 */
  spawn_portal(game, ivec2(5, rows / 2), 1, PHASE_RED);
  /* Portal AZUL en seccion 2 -> seccion 3 */
  spawn_portal(game, ivec2(11, rows / 2), 2, PHASE_BLUE);
  /* Portal ROJO en seccion 3 -> seccion 1 (retorno) */
  spawn_portal(game, ivec2(18, rows / 2), 0, PHASE_RED);

  /* Muros de fase en seccion 2 */
  for (int y = 3; y < rows - 3; y++)
    game->map->data[y][10] = CELL_WALL_BLUE;
  game->map->data[rows / 2 + 1][10] = CELL_FLOOR;

  /* Guardia en seccion 2 */
  spawn_guard(game, ivec2(11, 4));

  /* Llave en seccion 2 */
  allocate_item(game, ivec2(10, 3), ITEM_KEY);

  /* Coherencia en seccion 2 */
  allocate_item(game, ivec2(10, rows - 3), ITEM_COHERENCE_PICKUP);

  /* Puerta y salida en seccion 3 */
  game->map->data[rows / 2][20] = CELL_DOOR;
  game->exit_position = ivec2(21, rows / 2);
  game->map->data[rows / 2][21] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
}

void load_level_15(GameState *game) {
  int rows = 16;
  int cols = 22;
  init_game_state(game, rows, cols);
  snprintf(game->level_name, 64, "NIVEL 15: PRUEBA FINAL");
  game->current_level = 14;
  make_room(game);

  /* ZONA 1: Laberinto de fase (izquierda) */
  for (int y = 2; y < rows - 2; y++) {
    game->map->data[y][5] = CELL_WALL_RED;
    game->map->data[y][8] = CELL_WALL_BLUE;
  }
  game->map->data[4][5] = CELL_FLOOR;
  game->map->data[rows - 4][8] = CELL_FLOOR;

  /* ZONA 2: Dos botones cercanos (distancia 5) */
  spawn_button(game, ivec2(12, rows / 2 - 2), PHASE_RED);
  spawn_button(game, ivec2(12, rows / 2 + 3), PHASE_BLUE);

  /* Barricade blocking exit, opened by button */
  /* FULLY BLOCK the exit area */
  /* Barricade blocking exit, opened by button */
  /* FULLY BLOCK the exit area */
  for (int y = 0; y < rows; y++) {
    game->map->data[y][cols - 2] = CELL_WALL; // Solid wall at x=20
  }
  game->map->data[6][cols - 2] = CELL_BARRICADE; // The only way through
  game->map->data[5][cols - 2] = CELL_BARRICADE; // Wider opening
  game->map->data[7][cols - 2] = CELL_BARRICADE;

  game->exit_position = ivec2(cols - 1, 6);
  game->map->data[6][cols - 1] = CELL_EXIT;

  game->player.position = ivec2(1, rows / 2);
}

void load_level_16(GameState *game) {
  int rows = 16;
  int cols = 24;
  init_game_state(game, rows, cols);
  snprintf(game->level_name, 64, "NIVEL 16: TUNEL CUANTICO (GROVER II)");
  game->current_level = 15;
  make_room(game);

  /* Central Chamber (Accessible only via Tunnel) */
  for (int x = 10; x <= 14; x++) {
    for (int y = 6; y <= 10; y++) {
      game->map->data[y][x] = CELL_WALL;
    }
  }
  game->map->data[8][12] = CELL_FLOOR; // Inside chamber

  // Tunnel 1: Outside (4,4) -> Inside (12,8)
  spawn_tunnel(game, ivec2(4, 4), ivec2(1, 1), ivec2(8, 4));

  // Tunnel 2: Inside (12,8) -> Outside (4,4) (Return ticket!)
  spawn_tunnel(game, ivec2(12, 8), ivec2(1, 1), ivec2(-8, -4));

  allocate_item(game, ivec2(12, 8), ITEM_KEY); // Central Key

  /* Maze layout */
  for (int x = 6; x < 20; x += 4) {
    game->map->data[4][x] = CELL_WALL_RED;
    game->map->data[rows - 4][x] = CELL_WALL_BLUE;
  }

  /* Detectors */
  spawn_detector(game, ivec2(8, 2), DIR_DOWN, PHASE_RED);
  spawn_detector(game, ivec2(16, rows - 2), DIR_UP, PHASE_BLUE);

  /* Outer Key */
  allocate_item(game, ivec2(22, 2), ITEM_KEY);

  /* Exit - Double Door */
  game->map->data[rows / 2][cols - 3] = CELL_DOOR;
  game->map->data[rows / 2][cols - 2] = CELL_DOOR;
  game->exit_position = ivec2(cols - 1, rows / 2);
  game->map->data[rows / 2][cols - 1] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
}

void load_level_17(GameState *game) {
  int rows = 16;
  int cols = 24;
  init_game_state(game, rows, cols);
  snprintf(game->level_name, 64, "NIVEL 17: ENTRELAZAMIENTO FINAL");
  game->current_level = 16;
  make_room(game);

  /* Exit ISOLATED by Barricades */
  for (int y = 0; y < rows; y++)
    game->map->data[y][cols - 4] = CELL_WALL;
  game->map->data[rows / 2][cols - 4] = CELL_BARRICADE;

  /* Portal: Guard Cage -> Exit (Inactive initially) */
  spawn_portal(game, ivec2(17, 7), 1, PHASE_RED);
  game->portals[MAX_PORTALS - 1].active = false; // Manually deactivate

  /* Portal Destination: Between Barricade and Exit */
  spawn_portal(game, ivec2(cols - 3, rows / 2), 0, PHASE_RED);

  game->exit_position = ivec2(cols - 2, rows / 2);
  game->map->data[rows / 2][cols - 2] = CELL_EXIT;

  /* Guard Cage (Expanded for maneuvering) */
  for (int x = 15; x <= 21; x++) {
    game->map->data[3][x] = CELL_WALL;  // Top
    game->map->data[11][x] = CELL_WALL; // Bottom
  }
  for (int y = 3; y <= 11; y++) {
    game->map->data[y][15] = CELL_WALL; // Left
    game->map->data[y][21] = CELL_WALL; // Right
  }
  game->map->data[7][15] = CELL_WALL_RED; // Viewport (Left side)

  /* Button inside cage */
  spawn_button(game, ivec2(18, 7), PHASE_RED);

  /* Guard inside cage */
  spawn_guard(game, ivec2(17, 7));

  /* Player Start */
  game->player.position = ivec2(2, rows / 2);

  /* Coherence & Advice */
  allocate_item(game, ivec2(8, 4), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(8, 10), ITEM_COHERENCE_PICKUP);
}

void load_level_18(GameState *game) {
  // Keep existing Level 18 (Oracle)
  int rows = 18;
  int cols = 26;
  init_game_state(game, rows, cols);
  snprintf(game->level_name, 64, "NIVEL 18: GROVER IV (ORACULO)");
  game->current_level = 17;
  make_room(game);

  /* Oracle Search: 4 Chambers */
  for (int x = 2; x < 12; x++)
    game->map->data[8][x] = CELL_WALL;
  for (int y = 2; y < 8; y++)
    game->map->data[y][12] = CELL_WALL;

  for (int x = 14; x < 24; x++)
    game->map->data[8][x] = CELL_WALL;
  for (int y = 2; y < 8; y++)
    game->map->data[y][14] = CELL_WALL;

  for (int y = 10; y < 16; y++)
    game->map->data[y][12] = CELL_WALL;
  for (int y = 10; y < 16; y++)
    game->map->data[y][14] = CELL_WALL;

  game->map->data[8][12] = CELL_FLOOR;
  game->map->data[8][14] = CELL_FLOOR;

  spawn_detector(game, ivec2(13, 2), DIR_DOWN, PHASE_RED);
  spawn_detector(game, ivec2(13, 15), DIR_UP, PHASE_BLUE);

  allocate_item(game, ivec2(20, 4), ITEM_KEY);
  allocate_item(game, ivec2(6, 4), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(6, 12), ITEM_BOMB_REFILL);
  allocate_item(game, ivec2(20, 12), ITEM_COHERENCE_PICKUP);

  spawn_guard(game, ivec2(18, 5));

  game->exit_position = ivec2(13, 17);
  game->map->data[17][13] = CELL_EXIT;
  game->map->data[16][13] = CELL_DOOR;
  game->player.position = ivec2(13, 9);
}

void load_level_19(GameState *game) {
  int rows = 20;
  int cols = 30;
  init_game_state(game, rows, cols);
  snprintf(game->level_name, 64, "NIVEL 19: EJECUCION FINAL");
  game->current_level = 18;
  make_room(game);

  /* Complex architecture */
  for (int x = 0; x < cols; x++) {
    if (x % 4 == 0)
      game->map->data[6][x] = CELL_WALL_RED;
    if (x % 4 == 2)
      game->map->data[12][x] = CELL_WALL_BLUE;
  }

  /* RE-REDESIGN: 2 Buttons (Player + Echo) but HARDER */
  /* Remove one Red Button. Add Lasers. Add Blue Key. Add Yellow Spheres. */

  /* Button 1 (Red) - Top Left - CLOSER */
  spawn_button(game, ivec2(14, 8), PHASE_RED);

  /* Button 2 (Blue) - Bottom Right - CLOSER */
  spawn_button(game, ivec2(14, 12), PHASE_BLUE);

  /* Lasers (Detectors) guarding paths - REMOVED Green/Yellow */
  // spawn_detector(game, ivec2(15, 2), DIR_DOWN, PHASE_GREEN);
  // spawn_detector(game, ivec2(15, 18), DIR_UP, PHASE_YELLOW);

  /* Blue Key required for exit */
  allocate_item(game, ivec2(15, 10), ITEM_KEY);

  /* Yellow Spheres (Coherence) */
  allocate_item(game, ivec2(4, 16), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(26, 4), ITEM_COHERENCE_PICKUP);

  /* Guards */
  spawn_guard(game, ivec2(15, 6));
  spawn_guard(game, ivec2(15, 14));

  /* Final gate: Button Barricade -> Key Door -> Exit */
  game->map->data[rows / 2][cols - 2] = CELL_DOOR; // Needs Blue Key
  for (int y = 0; y < rows; y++) {
    if (y != rows / 2)
      game->map->data[y][cols - 4] = CELL_BARRICADE; // Outer ring
  }

  game->map->data[rows / 2][cols - 4] = CELL_BARRICADE;

  game->exit_position = ivec2(cols - 1, rows / 2);
  game->map->data[rows / 2][cols - 1] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
  game->player.bombs = 2;
  game->player.phase_system.current_phase = PHASE_BLUE;
}

void load_level_20(GameState *game) {
  int rows = 24;
  int cols = 32;
  init_game_state(game, rows, cols);
  snprintf(game->level_name, 64, "NIVEL 20: FINAL CUANTICO");
  game->current_level = 19;
  make_room(game);

  /* SECTION 1: The Mesh (Phase Wall Maze) */
  for (int x = 4; x < 12; x++) {
    for (int y = 2; y < rows - 2; y += 2) {
      if ((x + y) % 2 == 0)
        game->map->data[y][x] = CELL_WALL_RED;
      else
        game->map->data[y][x] = CELL_WALL_BLUE;
    }
  }

  /* SECTION 2: The Void (Islands & Tunnels) */
  /* Wall separating Sec 1 & 2 */
  for (int y = 0; y < rows; y++)
    game->map->data[y][12] = CELL_WALL;

  game->map->data[rows / 2][12] = CELL_DOOR; // Door 1 (Mandatory to enter Void)

  /* Island 2 (Central) - STRICTLY ENCLOSED */
  for (int x = 16; x < 24; x++) {
    for (int y = 4; y < rows - 4; y++) {
      if (x == 16 || x == 23 || y == 4 || y == rows - 5)
        game->map->data[y][x] = CELL_WALL; // Solid walls around Island 2
      else
        game->map->data[y][x] = CELL_FLOOR;
    }
  }

  /* Tunnel (Area 2x2) to Island 2 */
  // Tunnel Entrance at (10, 12). Target -> (18, 12) inside Island 2.
  spawn_tunnel(game, ivec2(10, rows / 2), ivec2(2, 2), ivec2(8, 0));

  /* SECTION 3: The Collapse (Final Gauntlet) */
  /* Wall separating Sec 2 & 3 - SOLID (Forces Portal) */
  for (int y = 0; y < rows; y++)
    game->map->data[y][24] = CELL_WALL;

  /* Walls inside Sec 3 to force pathing (Top vs Bottom) */
  for (int x = 24; x < cols; x++) {
    game->map->data[12][x] = CELL_WALL; // Split Sec 3 into Top/Bottom
  }

  // Door connecting Top Sec 3 to Bottom Sec 3 (Requires Key 2)
  game->map->data[12][26] = CELL_DOOR;

  // Final Door blocking Exit (Requires Key 3)
  // game->map->data[12][30] = CELL_DOOR; // OLD
  game->map->data[12][30] = CELL_WALL; // Blocked

  // Make Exit accessible behind Final Door
  // game->map->data[12][31] = CELL_EXIT; // OLD
  game->map->data[12][31] = CELL_WALL; // Blocked
  // game->exit_position = ivec2(31, 12); // OLD

  // NEW EXIT: Bottom Row Vertical (29, 23)
  // Approach from Top
  game->map->data[21][29] = CELL_FLOOR;
  game->map->data[22][29] = CELL_DOOR; // Final Door
  game->map->data[23][29] = CELL_EXIT; // Exit (Overwrites bottom wall)
  game->exit_position = ivec2(29, 23);

  // Clean up previous attempt pos
  game->map->data[22][30] = CELL_FLOOR; // Just floor next to it

  /* Portals: Island 2 -> Sec 3 */
  // Portal 0 (Source) links to 1.
  // Source: Inside Island 2 (20, 8)
  spawn_portal(game, ivec2(20, 8), 1, PHASE_BLUE);

  // Dest: Inside Sec 3 Top (26, 4)
  spawn_portal(game, ivec2(26, 4), 0, PHASE_BLUE);

  /* HAZARDS */
  /* Lasers (Drain 40 Coherence) */
  spawn_detector(game, ivec2(14, 2), DIR_DOWN, PHASE_RED);
  spawn_detector(game, ivec2(22, 20), DIR_UP, PHASE_BLUE); // Below Island 2

  spawn_detector(game, ivec2(26, 10), DIR_LEFT, PHASE_RED);
  spawn_detector(game, ivec2(28, 14), DIR_RIGHT, PHASE_BLUE);

  /* ENEMIES */
  spawn_guard(game, ivec2(20, 12)); // Island 2
  spawn_guard(game, ivec2(28, 6));  // Sec 3 Top
  spawn_guard(game, ivec2(26, 18)); // Sec 3 Bottom

  /* KEYS - REQUIRED FOR PROGRESSION */
  // Key 1: In Section 1 Mesh (Required for Door 1)
  allocate_item(game, ivec2(8, 4), ITEM_KEY);

  // Key 2: In Island 2 (Required for Door at 26,12)
  allocate_item(game, ivec2(22, 18), ITEM_KEY); // Move Key 2

  // Key 3: In Sec 3 Bottom (Required for Final Door at 30,12)
  allocate_item(game, ivec2(28, 20), ITEM_KEY); // Move Key 3 to Bottom

  /* EXTRAS */
  allocate_item(game, ivec2(6, 20), ITEM_BOMB_REFILL);
  allocate_item(game, ivec2(18, 10), ITEM_COHERENCE_PICKUP);

  /* EXIT is set above */
  // game->exit_position = ivec2(cols - 1, rows / 2);
  // game->map->data[rows / 2][cols - 1] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
  game->player.bombs = 3;
}
