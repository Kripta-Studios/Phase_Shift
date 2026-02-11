#include "levels.h"
#include <stdio.h>
#include <string.h>

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

  /* Puerta y salida */
  game->map->data[rows / 2][19] = CELL_DOOR;
  game->exit_position = ivec2(20, rows / 2);
  game->map->data[rows / 2][20] = CELL_EXIT;

  game->player.position = ivec2(2, 2);
}

/* ========== NIVEL 3: PARADOJA TEMPORAL ========== */
void load_level_3(GameState *game) {
  int rows = 18;
  int cols = 24;
  init_game_state(game, rows, cols);
  game->current_level = 2;
  snprintf(game->level_name, 64, "NIVEL 3: PARADOJA TEMPORAL");

  make_room(game);

  /* Tres cámaras separadas por barreras */
  for (int y = 1; y < rows - 1; y++) {
    game->map->data[y][8] = CELL_BARRICADE;
    game->map->data[y][16] = CELL_BARRICADE;
  }

  /* Botones en cada cámara requiriendo diferentes fases */
  spawn_button(game, ivec2(4, rows / 2 - 3), PHASE_RED);
  spawn_button(game, ivec2(12, rows / 2), PHASE_BLUE);
  spawn_button(game, ivec2(20, rows / 2 + 3), PHASE_RED);

  /* Recarga de bomba para abrir paso si fallan los ecos */
  allocate_item(game, ivec2(4, rows - 3), ITEM_BOMB_REFILL);
  game->player.bombs = 1;
  game->player.bomb_slots = 2;

  /* Salida detrás de la tercera barrera */
  game->exit_position = ivec2(22, rows / 2);
  game->map->data[rows / 2][22] = CELL_EXIT;

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
   * Botones CERCANOS (distancia 6) a la derecha del mapa.
   * Jugador va al botón A, activa eco, espera, va al B.
   * Eco reproduce y se queda en A.
   */
  spawn_button(game, ivec2(15, 4), PHASE_RED);         /* Botón A arriba */
  spawn_button(game, ivec2(15, rows - 5), PHASE_BLUE); /* Botón B abajo */
  /* Distancia entre botones: |4 - (rows-5)| = |4 - 9| = 5 movimientos ✓ */

  /* Barrera antes de la salida — se abre con ambos botones */
  for (int y = 0; y < rows; y++) {
    game->map->data[y][17] = CELL_BARRICADE;
  }

  /* Llave accesible directamente */
  allocate_item(game, ivec2(10, 3), ITEM_KEY);

  /* Puerta y salida */
  game->map->data[rows / 2][18] = CELL_DOOR;
  game->exit_position = ivec2(18, rows / 2);
  game->map->data[rows / 2][18] = CELL_EXIT;

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
  spawn_tunnel(game, ivec2(7, 4), ivec2(4, 3));

  /* GUARDIA ESPERANDO EN ZONA 2 (el "Comité de Bienvenida") */
  spawn_guard(game, ivec2(14, 7));

  /* Túnel 2: (16,6) -> (20,9) [Zona 3] */
  spawn_tunnel(game, ivec2(16, 6), ivec2(4, 3));

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
  game->map->data[rows / 2][25] = CELL_DOOR;

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
  spawn_guard(game, ivec2(cols - 7, 3)); /* Guardia esperando al final */

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
  spawn_tunnel(game, ivec2(24, 6), ivec2(4, 3)); /* Teleporta a 28 (safe) */

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

  game->map->data[rows / 2][28] = CELL_DOOR;
  game->exit_position = ivec2(28, rows / 2);
  game->map->data[rows / 2][28] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
}

/* ========== SYSTEM FUNCTIONS ========== */

void load_level(GameState *game, int level_index) {
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
          "Sujeto 44 inicializado.\nCoherencia cuantica: 98%.\n\n"
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
          "Usa Ecos para resolver puzzles.",
          MAX_DIALOG_TEXT);

  strncpy(d->pages[3].title, "ENTIDADES", 64);
  strncpy(d->pages[3].text,
          "Evita el contacto directo con los Eepers.\n\n"
          "Son anomalias cuanticas inestables.\n"
          "La observacion colapsa su funcion de onda.",
          MAX_DIALOG_TEXT);

  strncpy(d->pages[4].title, "ADVERTENCIA", 64);
  strncpy(d->pages[4].text,
          "Vigila tu medidor de COHERENCIA.\n"
          "Si llega al 0%, te disolveras.\n\n"
          "Buena suerte.",
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
            "Tres botones. Tres barreras.\n\n"
            "ESTRATEGIA:\n"
            "1. Ve al primer boton (ROJO)\n"
            "2. Pulsa [ESPACIO] para Superposicion\n"
            "3. Usa [T] para ESPERAR 3-4 turnos\n"
            "4. Corre al siguiente boton mientras tu eco pulsa el primero\n"
            "5. Repite hasta abrir todas las barreras",
            MAX_DIALOG_TEXT);
    break;
  case 3:
    strncpy(d->pages[0].text,
            "COMPUERTA HADAMARD\n\n"
            "Dos botones cercanos (distancia 5 mov).\n"
            "Ambos deben pulsarse a la vez.\n\n"
            "1. Ve al boton de ARRIBA\n"
            "2. Activa SUPERPOSICION [ESPACIO]\n"
            "3. Espera [T] y corre al de ABAJO\n"
            "4. Tu eco mantiene el primero pulsado",
            MAX_DIALOG_TEXT);
    break;
  case 4:
    strncpy(d->pages[0].text,
            "ALGORITMO DE GROVER\n\n"
            "Busqueda cuantica en un laberinto denso.\n\n"
            "DETECTORES + GUARDIA + MUROS DE FASE\n"
            "- Detectores ROJOS ven fase ROJA\n"
            "- Detectores AZULES ven fase AZUL\n"
            "- Evita al guardia o usa bombas [X]\n\n"
            "Busca la llave y llega a la salida.",
            MAX_DIALOG_TEXT);
    break;
  case 5:
    strncpy(d->pages[0].text,
            "TELETRANSPORTE CUANTICO\n\n"
            "Tres zonas aisladas por muros impenetrables.\n"
            "Solo los TUNELES CUANTICOS conectan las zonas.\n\n"
            "COMO TUNELIZAR:\n"
            "1. Entra en la zona PURPURA\n"
            "2. Activa SUPERPOSICION [ESPACIO]\n"
            "3. 50% de probabilidad de teletransportarte\n\n"
            "ALTERNATIVA: El boton en zona 2 abre\n"
            "una barricada entre zona 2 y zona 3.",
            MAX_DIALOG_TEXT);
    break;
  case 6:
    strncpy(d->pages[0].text,
            "CORRECCION DE ERRORES CUANTICOS\n\n"
            "La redundancia protege la informacion cuantica.\n"
            "Tres corredores paralelos — cada uno con\n"
            "un tipo diferente de \"ruido\":\n\n"
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
            "Prueba final: todas las mecanicas combinadas.\n\n"
            "ZONA 1: Laberinto de fase (cambia fase)\n"
            "ZONA 2: Detectores + 2 botones (usa eco)\n"
            "ZONA 3: Tunel cuantico + llave\n\n"
            "ESTRATEGIA:\n"
            "- Cruza el laberinto alternando fases\n"
            "- Usa un ECO para un boton\n"
            "- Pisa el otro boton para abrir barrera\n"
            "- Tuneliza para alcanzar la llave final\n\n"
            "Buena suerte, Sujeto 44.",
            MAX_DIALOG_TEXT);
    break;
  default:
    strncpy(d->pages[0].text, "Proceda con precaucion.", MAX_DIALOG_TEXT);
    break;
  }
}

void check_level_events(GameState *game) {
  /* NIVEL 3: Barreras que se abren con botones */
  if (game->current_level == 2) {
    int pressed_count = 0;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active && game->buttons[i].is_pressed) {
        pressed_count++;
      }
    }

    /* Cada botón abre una barrera */
    if (pressed_count >= 1 &&
        game->map->data[game->map->rows / 2][8] == CELL_BARRICADE) {
      for (int y = 0; y < game->map->rows; y++) {
        game->map->data[y][8] = CELL_FLOOR;
      }
      PlaySound(phase_shift_sound);
    }
    if (pressed_count >= 2 &&
        game->map->data[game->map->rows / 2][16] == CELL_BARRICADE) {
      for (int y = 0; y < game->map->rows; y++) {
        game->map->data[y][16] = CELL_FLOOR;
      }
      PlaySound(phase_shift_sound);
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
      int gate_col = 17;
      bool was_closed = false;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][gate_col] == CELL_BARRICADE) {
          game->map->data[y][gate_col] = CELL_FLOOR;
          was_closed = true;
        }
      }
      if (was_closed) {
        PlaySound(phase_shift_sound);
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
      PlaySound(phase_shift_sound);
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
        PlaySound(phase_shift_sound);
      }
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
        PlaySound(phase_shift_sound);
      }
    }
  }
}