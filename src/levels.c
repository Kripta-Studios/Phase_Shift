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

/* ========== NIVEL 2: DUALIDAD ESPIRAL ========== */
void load_level_2(GameState *game) {
  int rows = 16;
  int cols = 20;
  init_game_state(game, rows, cols);
  game->current_level = 1;
  snprintf(game->level_name, 64, "NIVEL 2: DUALIDAD ESPIRAL");

  make_room(game);

  /* Espiral roja desde afuera */
  for (int r = 1; r < 6; r++) {
    for (int x = r; x < cols - r; x++) {
      game->map->data[r][x] = CELL_WALL_RED;
      game->map->data[rows - 1 - r][x] = CELL_WALL_RED;
    }
    for (int y = r; y < rows - r; y++) {
      game->map->data[y][r] = CELL_WALL_RED;
      game->map->data[y][cols - 1 - r] = CELL_WALL_RED;
    }
  }

  /* Espiral azul desplazada */
  for (int r = 2; r < 5; r++) {
    for (int x = r; x < cols - r; x++) {
      game->map->data[r + 1][x] = CELL_WALL_BLUE;
      game->map->data[rows - 2 - r][x] = CELL_WALL_BLUE;
    }
    for (int y = r; y < rows - r; y++) {
      game->map->data[y][r + 1] = CELL_WALL_BLUE;
      game->map->data[y][cols - 2 - r] = CELL_WALL_BLUE;
    }
  }

  /* Aperturas estratégicas */
  game->map->data[3][3] = CELL_FLOOR;
  game->map->data[5][cols - 4] = CELL_FLOOR;
  game->map->data[rows - 4][3] = CELL_FLOOR;
  game->map->data[rows - 6][cols - 4] = CELL_FLOOR;

  /* Llave en el centro */
  allocate_item(game, ivec2(cols / 2, rows / 2), ITEM_KEY);

  /* Salida y puerta */
  game->map->data[rows / 2][cols - 3] = CELL_DOOR;
  game->exit_position = ivec2(cols - 2, rows / 2);
  game->map->data[rows / 2][cols - 2] = CELL_EXIT;

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

/* ========== NIVEL 4: ENTRELAZAMIENTO ========== */
void load_level_4(GameState *game) {
  int rows = 20;
  int cols = 28;
  init_game_state(game, rows, cols);
  game->current_level = 3;
  snprintf(game->level_name, 64, "NIVEL 4: ENTRELAZAMIENTO");

  make_room(game);

  /* Sistema de "X" con muros de fase */
  for (int i = 4; i < rows - 4; i++) {
    int dist = abs(i - rows / 2);
    game->map->data[i][10 - dist] = CELL_WALL_RED;
    game->map->data[i][10 + dist] = CELL_WALL_BLUE;
    game->map->data[i][18 - dist] = CELL_WALL_BLUE;
    game->map->data[i][18 + dist] = CELL_WALL_RED;
  }

  /* Cuatro botones en las esquinas del patrón */
  spawn_button(game, ivec2(6, 4), PHASE_RED);
  spawn_button(game, ivec2(22, 4), PHASE_BLUE);
  spawn_button(game, ivec2(6, rows - 5), PHASE_BLUE);
  spawn_button(game, ivec2(22, rows - 5), PHASE_RED);

  /* Gran barrera vertical que se abre con los botones */
  for (int y = 0; y < rows; y++) {
    game->map->data[y][25] = CELL_BARRICADE;
  }

  /* Salida */
  game->exit_position = ivec2(26, rows / 2);
  game->map->data[rows / 2][26] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
  game->player.bombs = 0; /* Sin bombas - solo ecos */
}

/* ========== NIVEL 5: OBSERVADOR HOSTIL ========== */
void load_level_5(GameState *game) {
  int rows = 18;
  int cols = 26;
  init_game_state(game, rows, cols);
  game->current_level = 4;
  snprintf(game->level_name, 64, "NIVEL 5: OBSERVADOR HOSTIL");

  make_room(game);

  /* Laberinto con guardias patrullando */
  /* Crear paredes internas */
  for (int y = 3; y < rows - 3; y += 3) {
    for (int x = 3; x < cols - 2; x++) {
      if (x % 6 != 0) {
        game->map->data[y][x] = CELL_WALL;
      }
    }
  }

  /* Guardias en posiciones estratégicas */
  spawn_guard(game, ivec2(8, 6));
  spawn_guard(game, ivec2(16, 10));
  spawn_guard(game, ivec2(12, rows - 7));

  /* Gnomos con llaves */
  spawn_gnome(game, ivec2(10, 4));
  spawn_gnome(game, ivec2(20, 8));

  /* Detectores cuánticos añadiendo presión */
  spawn_detector(game, ivec2(6, rows / 2), DIR_RIGHT, PHASE_RED);
  spawn_detector(game, ivec2(14, rows / 2), DIR_RIGHT, PHASE_BLUE);

  /* Bombas para estrategia */
  allocate_item(game, ivec2(4, 4), ITEM_BOMB_REFILL);
  game->player.bombs = 2;
  game->player.bomb_slots = 3;

  /* Puerta que requiere 2 llaves */
  game->map->data[rows / 2][23] = CELL_DOOR;
  game->exit_position = ivec2(24, rows / 2);
  game->map->data[rows / 2][24] = CELL_EXIT;

  game->player.position = ivec2(2, 2);
}

/* ========== NIVEL 6: SUPERPOSICIÓN ESTABLE ========== */
void load_level_6(GameState *game) {
  int rows = 16;
  int cols = 28;
  init_game_state(game, rows, cols);
  game->current_level = 5;
  snprintf(game->level_name, 64, "NIVEL 6: SUPERPOSICION ESTABLE");

  make_room(game);

  /* Crear zona de tunelización en el centro */
  spawn_tunnel(game, ivec2(10, 6), ivec2(4, 4));

  /* Muro masivo que solo se puede atravesar tunelizando */
  for (int y = 1; y < rows - 1; y++) {
    for (int x = 13; x <= 15; x++) {
      game->map->data[y][x] = CELL_WALL;
    }
  }

  /* Botón que abre camino alternativo (por si falla el tunel) */
  spawn_button(game, ivec2(5, rows / 2), PHASE_RED);
  for (int y = rows / 2 - 1; y <= rows / 2 + 1; y++) {
    game->map->data[y][14] = CELL_BARRICADE; /* Se abre con botón */
  }

  /* Detectores del otro lado */
  spawn_detector(game, ivec2(18, 4), DIR_DOWN, PHASE_BLUE);
  spawn_detector(game, ivec2(22, rows - 5), DIR_UP, PHASE_RED);

  /* Salida */
  game->exit_position = ivec2(26, rows / 2);
  game->map->data[rows / 2][26] = CELL_EXIT;

  /* Items de ayuda */
  allocate_item(game, ivec2(3, 3), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(20, rows / 2), ITEM_COHERENCE_PICKUP);

  game->player.position = ivec2(2, rows / 2);
}

/* ========== NIVEL 7: DECOHERENCIA ========== */
void load_level_7(GameState *game) {
  int rows = 22;
  int cols = 30;
  init_game_state(game, rows, cols);
  game->current_level = 6;
  snprintf(game->level_name, 64, "NIVEL 7: DECOHERENCIA");

  make_room(game);

  /* Grid de detectores creando un campo peligroso */
  for (int x = 8; x < 24; x += 4) {
    PhaseKind phase = (x / 4) % 2 == 0 ? PHASE_RED : PHASE_BLUE;
    spawn_detector(game, ivec2(x, 5), DIR_DOWN, phase);
    spawn_detector(game, ivec2(x, rows - 6), DIR_UP, phase);
  }

  for (int y = 8; y < rows - 8; y += 3) {
    PhaseKind phase = (y / 3) % 2 == 0 ? PHASE_BLUE : PHASE_RED;
    spawn_detector(game, ivec2(6, y), DIR_RIGHT, phase);
    spawn_detector(game, ivec2(26, y), DIR_LEFT, phase);
  }

  /* Muros de fase creando laberinto */
  for (int y = 6; y < rows - 6; y++) {
    if (y % 4 != 0) {
      game->map->data[y][12] = CELL_WALL_RED;
      game->map->data[y][18] = CELL_WALL_BLUE;
    }
  }

  /* Guardias en el laberinto */
  spawn_guard(game, ivec2(10, 10));
  spawn_guard(game, ivec2(20, rows - 11));

  /* Pickup de coherencia crítico */
  allocate_item(game, ivec2(15, rows / 2), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(4, 4), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(28, rows - 5), ITEM_COHERENCE_PICKUP);

  /* Llave protegida */
  allocate_item(game, ivec2(15, rows / 2 + 3), ITEM_KEY);

  /* Puerta y salida */
  game->map->data[rows / 2][28] = CELL_DOOR;
  game->exit_position = ivec2(29, rows / 2);
  game->map->data[rows / 2][29] = CELL_EXIT;

  game->player.position = ivec2(2, rows / 2);
}

/* ========== NIVEL 8: COLAPSO CUÁNTICO ========== */
void load_level_8(GameState *game) {
  int rows = 26;
  int cols = 36;
  init_game_state(game, rows, cols);
  game->current_level = 7;
  snprintf(game->level_name, 64, "NIVEL 8: COLAPSO CUANTICO");

  make_room(game);

  /* Sección 1: Laberinto con detectores */
  for (int y = 4; y < 10; y++) {
    for (int x = 4; x < 14; x++) {
      if ((x - 4) % 3 == 0 || (y - 4) % 3 == 0) {
        game->map->data[y][x] =
            (x + y) % 2 == 0 ? CELL_WALL_RED : CELL_WALL_BLUE;
      }
    }
  }
  spawn_detector(game, ivec2(6, 7), DIR_RIGHT, PHASE_RED);
  spawn_detector(game, ivec2(12, 7), DIR_LEFT, PHASE_BLUE);

  /* Sección 2: Campo de guardias */
  spawn_guard(game, ivec2(18, 6));
  spawn_guard(game, ivec2(24, 10));
  spawn_guard(game, ivec2(20, 14));

  /* Sección 3: Zona de tunelización */
  spawn_tunnel(game, ivec2(14, 16), ivec2(3, 3));
  for (int y = 14; y < 20; y++) {
    game->map->data[y][18] = CELL_WALL;
    game->map->data[y][19] = CELL_WALL;
  }

  /* Sección 4: Sistema de botones complejo */
  spawn_button(game, ivec2(8, rows - 8), PHASE_RED);
  spawn_button(game, ivec2(16, rows - 6), PHASE_BLUE);
  spawn_button(game, ivec2(24, rows - 8), PHASE_RED);
  spawn_button(game, ivec2(28, rows - 10), PHASE_BLUE);

  /* Barrera final */
  for (int y = 0; y < rows; y++) {
    game->map->data[y][32] = CELL_BARRICADE;
  }

  /* Gnomos con llaves */
  spawn_gnome(game, ivec2(10, 12));
  spawn_gnome(game, ivec2(26, rows - 12));

  /* Recursos */
  allocate_item(game, ivec2(6, 6), ITEM_BOMB_REFILL);
  allocate_item(game, ivec2(22, 16), ITEM_COHERENCE_PICKUP);
  allocate_item(game, ivec2(12, rows - 4), ITEM_COHERENCE_PICKUP);

  game->player.bombs = 2;
  game->player.bomb_slots = 4;

  /* Salida épica */
  game->exit_position = ivec2(34, rows / 2);
  game->map->data[rows / 2][34] = CELL_EXIT;

  game->player.position = ivec2(2, 2);
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
            "ESPIRAL DE DUALIDAD\n\n"
            "Dos espirales entrelazadas de diferentes fases.\n"
            "Navega entre ellas cambiando de fase.\n\n"
            "La llave esta en el centro del patron.",
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
            "ENTRELAZAMIENTO CUANTICO\n\n"
            "Cuatro botones forman un patron entrelazado.\n"
            "TODOS deben pulsarse SIMULTANEAMENTE.\n\n"
            "Necesitaras usar TRES ECOS:\n"
            "Activa superposicion en cada boton, espera,\n"
            "y corre al siguiente antes de que termine.",
            MAX_DIALOG_TEXT);
    break;
  case 4:
    strncpy(d->pages[0].text,
            "ENEMIGOS DETECTADOS\n\n"
            "Guardias: Patrullan y te persiguen (3 hits)\n"
            "Gnomos: Huyen pero tienen llaves (1 hit)\n"
            "Detectores: Reducen tu coherencia si te ven\n\n"
            "[X] para plantar bombas. Sigilo recomendado.",
            MAX_DIALOG_TEXT);
    break;
  case 5:
    strncpy(d->pages[0].text,
            "TUNELIZACION CUANTICA\n\n"
            "Zona PURPURA = Portal cuantico inestable\n\n"
            "COMO TUNELIZAR:\n"
            "1. Entra en la zona purpura\n"
            "2. Pulsa [ESPACIO] para Superposicion\n"
            "3. Probabilidad 50% de atravesar el muro\n"
            "4. Si fallas, usa el boton para abrir paso\n\n"
            "CUIDADO: Los detectores drenan coherencia rapido",
            MAX_DIALOG_TEXT);
    break;
  case 6:
    strncpy(d->pages[0].text,
            "CAMPO DE OBSERVADORES\n\n"
            "Grid masivo de detectores cuanticos.\n"
            "Cada observacion reduce tu coherencia.\n\n"
            "TACTICAS:\n"
            "- Recoge TODOS los pickups de coherencia\n"
            "- Cambia de fase constantemente\n"
            "- Los detectores ROJOS ven fase ROJA\n"
            "- Los detectores AZULES ven fase AZUL\n\n"
            "Planifica tu ruta con cuidado.",
            MAX_DIALOG_TEXT);
    break;
  case 7:
    strncpy(d->pages[0].text,
            "PRUEBA FINAL: COLAPSO TOTAL\n\n"
            "Todas las mecanicas combinadas.\n"
            "Detectores, Guardias, Tuneles, Botones.\n\n"
            "Este nivel requiere:\n"
            "- Precision en ecos temporales\n"
            "- Gestion perfecta de coherencia\n"
            "- Uso estrategico de bombas\n"
            "- Timing impecable\n\n"
            "No hay atajos. Buena suerte, Sujeto 44.",
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

  /* NIVEL 4: Todos los botones simultáneamente abren barrera permanentemente */
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
      int gate_col = 25;
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

  /* NIVEL 6: Botón abre camino alternativo si falla tunelización */
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
        if (game->map->data[y][14] == CELL_BARRICADE) {
          game->map->data[y][14] = CELL_FLOOR;
          PlaySound(phase_shift_sound);
        }
      }
    }
  }

  /* NIVEL 8: Sistema de botones final */
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
      int gate_col = 32;
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