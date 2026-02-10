#include "levels.h"
#include <stdio.h>
#include <string.h>

void load_level_1(GameState *game) {
  /* LEVEL 1: Duality - Introduction to Phases */
  int rows = 12;
  int cols = 16;
  init_game_state(game, rows, cols);
  game->current_level = 0;
  snprintf(game->level_name, 64, "NIVEL 1: DUALIDAD");

  make_room(game);

  /* Simple corridor with phase gates */
  for (int y = 1; y < rows - 1; y++) {
    game->map->data[y][5] = CELL_WALL_RED;
    game->map->data[y][10] = CELL_WALL_BLUE;
  }

  /* Openings */
  game->map->data[rows / 2][5] = CELL_FLOOR;
  game->map->data[rows / 2 + 1][5] = CELL_FLOOR;

  /* Force phase switch */
  game->map->data[rows / 2][5] = CELL_WALL_RED;   /* Blocked for Red */
  game->map->data[rows / 2][10] = CELL_WALL_BLUE; /* Blocked for Blue */

  /* Exit */
  game->exit_position = ivec2(cols - 2, rows / 2);
  game->map->data[rows / 2][cols - 2] = CELL_EXIT;

  /* Hints */
  game->player.position = ivec2(2, rows / 2);
}

void load_level_2(GameState *game) {
  /* LEVEL 2: Superposition */
  int rows = 12;
  int cols = 16;
  init_game_state(game, rows, cols);
  game->current_level = 1;
  snprintf(game->level_name, 64, "NIVEL 2: SUPERPOSICION");

  make_room(game);

  /* A wall that blocks BOTH phases */
  for (int y = 1; y < rows - 1; y++) {
    game->map->data[y][8] = CELL_WALL_RED;
    if (y % 2 == 0)
      game->map->data[y][8] = CELL_WALL_BLUE;
  }

  /* The only way through is Superposition (Space) */
  /* Or actually, if it blocks Red AND Blue, you can't pass unless... */
  /* Wait, Superposition lets you pass RED and BLUE walls? */
  /* Logic: is_cell_solid returns true if phase matches. */
  /* If cell is RED, it blocks RED and SUPER. */
  /* So Superposition blocks EVERYTHING? No, that's bad. */
  /* Let's check logic:
     case CELL_WALL_RED: return phase == PHASE_RED || in_superposition;
     So Superposition logic in current code makes you HIT everything?
     That means Superposition is a DISADVANTAGE for movement?

     Ah, in original code:
     CELL_WALL_RED -> blocks RED.
     If Superposition -> blocks RED.

     So Superposition makes you interact with BOTH.
     So you can't pass RED or BLUE walls.

     But you can pass NONE?

     Maybe Level 2 should be about *Item* usage or Echos?

     Wait, the docs said "Superposition: exist in both states".
     If you exist in both, you hit both walls.

     So how to pass a wall? You can't pass a wall.
     But maybe you need to be in Superposition to activate a detector or button?

     Let's make Level 2 about Buttons.
  */

  /* Buttons that require specific phases */
  spawn_button(game, ivec2(8, 4), PHASE_RED);
  spawn_button(game, ivec2(8, 8), PHASE_BLUE);

  /* Door needs both buttons */
  /* Currently buttons don't open doors in my logic?
     Ah, logic needs to be updated to link buttons to doors.
     The current logic doesn't support that explicitly (no "trigger" system).

     I'll implement a simple "All buttons pressed -> Door opens" rule in logic
     or level specific check? Or just assume "Check Level Complete" handles it?

     Button logic in `update_pressure_buttons` just sets `is_pressed`.
     We need something that READS `is_pressed`.

     I'll add a generic "update_doors" to `logic.c` later or simply let the
     level be passed by reaching exit, but exit is blocked by a DOOR.

     And update `game_player_turn`: if cell is DOOR and *unlocked*, pass.
     Currently DOOR requires KEY.

     I should add "Electronic Door" that opens with buttons.
     Or just use `CELL_BARRICADE` that explodes?

     Let's stick to Keys for now for simplicity, or add a mechanic.

     User asked to "make levels fun".

     I'll add "If all buttons pressed, remove all BARRICADES" logic to
     `logic.c`?

     Yes. That's a good global rule.
  */

  /* Wall of barricades */
  for (int y = 1; y < rows - 1; y++) {
    game->map->data[y][12] = CELL_BARRICADE;
  }

  /* Player needs to press both buttons. */
  /* One is RED, one is BLUE. */
  /* You can switch phase to press one, then the other. */
  /* But buttons toggle? Or stay pressed if you stand on them? */
  /* Logic: "Player on button? ... is_pressed = true" */
  /* If you leave, it becomes false. */
  /* So you need TWO things on buttons. You + Echo? */
  /* That's Level 3 (Echoes). */

  /* Level 2: Superposition to press BOTH? */
  /* If there is a button that requires SUPERPOSITION?
     "if (in_super || phase == b->phase)"
     So Superposition presses RED and BLUE buttons.

     But buttons are at different locations.

     Maybe distinct buttons?

     Let's make Level 2 about simple Key finding with Phase Switching avoidance.

     Map:
     start -> [Blue Wall] -> Key -> [Red Wall] -> Door -> Exit.
  */

  game->map->data[rows / 2][5] = CELL_WALL_BLUE;
  allocate_item(game, ivec2(6, rows / 2), ITEM_KEY);

  game->map->data[rows / 2][10] = CELL_WALL_RED;
  game->map->data[rows / 2][12] = CELL_DOOR;

  game->exit_position = ivec2(14, rows / 2);
  game->map->data[rows / 2][14] = CELL_EXIT;
}

void load_level_3(GameState *game) {
  /* LEVEL 3: Quantum Echoes */
  int rows = 12;
  int cols = 16;
  init_game_state(game, rows, cols);
  game->current_level = 2;
  snprintf(game->level_name, 64, "NIVEL 3: ECOS");
  make_room(game);

  /* Button opens barricade */
  spawn_button(game, ivec2(cols / 2, rows / 2 - 3), PHASE_RED);

  /* Barricade blocking exit */
  for (int y = 0; y < rows; y++)
    game->map->data[y][cols - 4] = CELL_BARRICADE;

  game->exit_position = ivec2(cols - 2, rows / 2);
  game->map->data[rows / 2][cols - 2] = CELL_EXIT;

  /* Hint: Use Space to record Echo */
}

void load_level_4(GameState *game) {
  /* LEVEL 4: Entanglement / Cooperation */
  int rows = 14;
  int cols = 20;
  init_game_state(game, rows, cols);
  game->current_level = 3;
  snprintf(game->level_name, 64, "NIVEL 4: COOPERACION");
  make_room(game);

  /* Two buttons, need simultaneous press */
  spawn_button(game, ivec2(4, 4), PHASE_RED);
  spawn_button(game, ivec2(4, rows - 5), PHASE_BLUE);

  /* Barricade */
  for (int y = 0; y < rows; y++)
    game->map->data[y][15] = CELL_BARRICADE;

  game->exit_position = ivec2(18, rows / 2);
  game->map->data[rows / 2][18] = CELL_EXIT;
}

void load_level_5(GameState *game) {
  /* LEVEL 5: Guardians */
  int rows = 16;
  int cols = 20;
  init_game_state(game, rows, cols);
  game->current_level = 4;
  snprintf(game->level_name, 64, "NIVEL 5: GUARDIANES");
  make_room(game);

  /* Guards patroling */
  spawn_guard(game, ivec2(10, 5));
  spawn_guard(game, ivec2(10, 10));

  /* Key protected by guards */
  allocate_item(game, ivec2(15, 8), ITEM_KEY);

  game->map->data[8][18] = CELL_DOOR;
  game->exit_position = ivec2(19, 8);
  game->map->data[8][19] = CELL_EXIT;

  /* Player starts far left */
  game->player.position = ivec2(2, 8);
}

void load_level_6(GameState *game) {
  /* LEVEL 6: Tunneling */
  int rows = 12;
  int cols = 20;
  init_game_state(game, rows, cols);
  game->current_level = 5;
  snprintf(game->level_name, 64, "NIVEL 6: TUNELADO");
  make_room(game);

  /* Wall with no opening */
  for (int y = 1; y < rows - 1; y++)
    game->map->data[y][10] = CELL_WALL;

  /* Tunnel zone */
  spawn_tunnel(game, ivec2(9, 5), ivec2(2, 2));

  game->player.position = ivec2(2, 6);
  game->exit_position = ivec2(18, 6);
  game->map->data[6][18] = CELL_EXIT;
}

void load_level_7(GameState *game) {
  /* LEVEL 7: Detectors */
  int rows = 16;
  int cols = 24;
  init_game_state(game, rows, cols);
  game->current_level = 6;
  snprintf(game->level_name, 64, "NIVEL 7: OBSERVACION");
  make_room(game);

  /* Detectors watching a corridor */
  for (int x = 5; x < 20; x += 3) {
    spawn_detector(game, ivec2(x, 1), DIR_DOWN, PHASE_RED);
    spawn_detector(game, ivec2(x + 1, rows - 2), DIR_UP, PHASE_BLUE);
  }

  game->player.position = ivec2(2, rows / 2);
  game->exit_position = ivec2(22, rows / 2);
  game->map->data[rows / 2][22] = CELL_EXIT;
}

void load_level_8(GameState *game) {
  /* LEVEL 8: The Collapse */
  int rows = 20;
  int cols = 30;
  init_game_state(game, rows, cols);
  game->current_level = 7;
  snprintf(game->level_name, 64, "NIVEL 8: COLAPSO");
  make_room(game);

  /* Complex maze with guards, buttons, and detectors */
  spawn_guard(game, ivec2(15, 5));
  spawn_guard(game, ivec2(20, 15));

  spawn_button(game, ivec2(5, 5), PHASE_RED);
  spawn_button(game, ivec2(25, 15), PHASE_BLUE);

  for (int y = 0; y < rows; y++)
    game->map->data[y][28] = CELL_BARRICADE;

  game->player.position = ivec2(2, 2);
  game->exit_position = ivec2(29, rows / 2);
  game->map->data[rows / 2][29] = CELL_EXIT;
}

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
            "ROJO bloquea ROJO.\nAZUL bloquea AZUL.\nCambia con [Z].",
            MAX_DIALOG_TEXT);
    break;
  case 1:
    strncpy(d->pages[0].text,
            "Encuentra la Llave.\nEvita los muros de tu mismo color.",
            MAX_DIALOG_TEXT);
    break;
  case 2:
    strncpy(d->pages[0].text,
            "NIVEL 3: ECOS\n\n"
            "COMO USAR TU ECO:\n"
            "1. Ve al boton.\n"
            "2. Pulsa [ESPACIO] para activar Superposicion.\n"
            "3. Pulsa [T] o [.] para ESPERAR sobre el boton.\n"
            "4. Cuando aparezca tu Eco, ¡corre a la salida!\n"
            "El Eco mantendra el boton pulsado por ti.",
            MAX_DIALOG_TEXT);
    break;
  case 3:
    strncpy(d->pages[0].text,
            "NIVEL 4: COOPERACION\n\n"
            "Dos botones activan la puerta PERMANENTEMENTE.\n\n"
            "ESTRATEGIA:\n"
            "1. Ve al primer boton y activa [ESPACIO].\n"
            "2. Usa [T] varias veces para ESPERAR.\n"
            "3. Mientras el Eco pulsa uno, ve tu al otro.\n"
            "¡Al pulsar AMBOS, la puerta se abrirá para siempre!",
            MAX_DIALOG_TEXT);
    break;
  case 4:
    strncpy(d->pages[0].text, "Entidades detectadas.\nSe recomienda sigilo.",
            MAX_DIALOG_TEXT);
    break;
  case 5:
    strncpy(d->pages[0].text,
            "NIVEL 6: TUNELADO\n\n"
            "Busca la ZONA PURPURA en el suelo.\n"
            "1. Situate DENTRO de la zona purpura.\n"
            "2. Pulsa [ESPACIO] para activar Superposicion.\n"
            "3. La inestabilidad te transportara al otro lado.\n"
            "¡No te muevas mientras se estabiliza!",
            MAX_DIALOG_TEXT);
    break;
  case 6:
    strncpy(d->pages[0].text,
            "NIVEL 7: OBSERVACION\n\n"
            "Los laseres detectan tu presencia.\n"
            "ROJO te ve si eres ROJO (o Ecos).\n"
            "AZUL te ve si eres AZUL (o Ecos).\n"
            "¡Si te ven, tu coherencia caera!",
            MAX_DIALOG_TEXT);
    break;
  case 7:
    strncpy(d->pages[0].text, "Prueba Final.\nCombina todos los protocolos.",
            MAX_DIALOG_TEXT);
    break;
  default:
    strncpy(d->pages[0].text, "Proceda con precaucion.", MAX_DIALOG_TEXT);
    break;
  }
}

void check_level_events(GameState *game) {
  /* Handle level-specific logic that isn't generic */

  if (game->current_level == 2) { /* Level 3: Barricade Gate */
    bool button_pressed = false;

    /* Check if ANY button is pressed (there's only one) */
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active && game->buttons[i].is_pressed) {
        button_pressed = true;
        break;
      }
    }

    /* Barricade Column */
    int gate_col = game->map->cols - 4;

    /* If pressed, open gate. If released, close gate (unless blocked) */
    for (int y = 0; y < game->map->rows; y++) {
      Cell c = game->map->data[y][gate_col];

      if (button_pressed) {
        if (c == CELL_BARRICADE) {
          game->map->data[y][gate_col] = CELL_FLOOR;
          PlaySound(phase_shift_sound); /* Reuse sound */
        }
      } else {
        /* Close gate if it was open (FLOOR) and empty */
        /* We need to know if it WAS a barricade. */
        /* Hardcoded: The whole column was barricade except borders */
        if (y > 0 && y < game->map->rows - 1) {
          if (c == CELL_FLOOR) {
            bool blocked = false;
            if (ivec2_eq(game->player.position, ivec2(gate_col, y)))
              blocked = true;
            for (int e = 0; e < MAX_EEPERS; e++)
              if (!game->eepers[e].dead &&
                  ivec2_eq(game->eepers[e].position, ivec2(gate_col, y)))
                blocked = true;

            if (!blocked) {
              game->map->data[y][gate_col] = CELL_BARRICADE;
            }
          }
        }
      }
    }
  }

  if (game->current_level == 3) { /* Level 4: Dual Buttons */
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
      /* Open Level 4 Gate (col 15) */
      int gate_col = 15;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][gate_col] == CELL_BARRICADE) {
          game->map->data[y][gate_col] = CELL_FLOOR;
          PlaySound(phase_shift_sound);
        }
      }
    }
    /* Removed ELSE block: Gate now stays open once opened! */
  }

  if (game->current_level == 7) { /* Level 8: The Collapse */
    bool all_pressed = true;
    for (int i = 0; i < MAX_BUTTONS; i++) {
      if (game->buttons[i].is_active && !game->buttons[i].is_pressed) {
        all_pressed = false;
      }
    }

    if (all_pressed) {
      int gate_col = 28;
      for (int y = 0; y < game->map->rows; y++) {
        if (game->map->data[y][gate_col] == CELL_BARRICADE) {
          game->map->data[y][gate_col] = CELL_FLOOR;
          PlaySound(phase_shift_sound);
        }
      }
    }
  }
}
