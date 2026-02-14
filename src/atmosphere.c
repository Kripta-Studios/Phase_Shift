#include "common.h"
#include "utils.h"
#include <math.h>
#include <stdlib.h>

void init_atmosphere(GameState *game) {
    // Initialize Stars
    for (int i = 0; i < MAX_STARS; i++) {
        game->stars[i].position.x = (float)(rand() % GetScreenWidth());
        game->stars[i].position.y = (float)(rand() % GetScreenHeight());
        game->stars[i].brightness = (float)(rand() % 100) / 100.0f;
        game->stars[i].twinkle_speed = (float)(rand() % 5 + 1);
    }

    // Initialize Atoms
    for (int i = 0; i < MAX_ATOMS; i++) {
        game->atoms[i].position.x = (float)(rand() % GetScreenWidth());
        game->atoms[i].position.y = (float)(rand() % GetScreenHeight());
        game->atoms[i].radius = (float)(rand() % 20 + 20); // 20-40 radius
        game->atoms[i].electron_angle = (float)(rand() % 360);

        int col_idx = rand() % 4; // Use phase colors
        if (col_idx == 0)
            game->atoms[i].color = RED;
        else if (col_idx == 1)
            game->atoms[i].color = BLUE;
        else if (col_idx == 2)
            game->atoms[i].color = GREEN;
        else
            game->atoms[i].color = YELLOW;

        game->atoms[i].color.a = 50; // Very faint
    }
}

void update_atmosphere(GameState *game) {
    float dt = GetFrameTime();

    // Twinkle Stars
    for (int i = 0; i < MAX_STARS; i++) {
        game->stars[i].brightness +=
            sinf((float)GetTime() * game->stars[i].twinkle_speed) * 0.01f;
        if (game->stars[i].brightness > 1.0f)
            game->stars[i].brightness = 1.0f;
        if (game->stars[i].brightness < 0.2f)
            game->stars[i].brightness = 0.2f;
    }

    // Spin Atoms
    for (int i = 0; i < MAX_ATOMS; i++) {
        game->atoms[i].electron_angle += dt * 2.0f;
    }

    // Update Flashlight Angle
    if (game->flashlight_active) {
        Vector2 mouse = GetMousePosition();
        Vector2 player_screen = {
            (game->player.position.x * CELL_SIZE) + CELL_SIZE / 2 -
                game->camera.target.x + game->camera.offset.x,
            (game->player.position.y * CELL_SIZE) + CELL_SIZE / 2 -
                game->camera.target.y + game->camera.offset.y};

        float dx = mouse.x - player_screen.x;
        float dy = mouse.y - player_screen.y;
        game->flashlight_angle = atan2f(dy, dx) * (180.0f / PI);
    }
}

void render_atmosphere_bg(GameState *game) {
    // Draw Stars
    for (int i = 0; i < MAX_STARS; i++) {
        unsigned char alpha =
            (unsigned char)(game->stars[i].brightness * 255.0f);
        Color star_col = {255, 255, 255, alpha};
        DrawCircleV(game->stars[i].position, 1.5f, star_col);
    }

    // Draw Atoms (decorative rings)
    for (int i = 0; i < MAX_ATOMS; i++) {
        Atom *a = &game->atoms[i];
        // Nucleus
        DrawCircleV(a->position, a->radius * 0.2f, a->color);
        // Electron orbit ring
        DrawCircleLines((int)a->position.x, (int)a->position.y, a->radius,
                        a->color);
        // Electron dot
        float angle = a->electron_angle;
        Vector2 electron = {a->position.x + cosf(angle) * a->radius,
                            a->position.y + sinf(angle) * a->radius};
        Color bright_col = a->color;
        bright_col.a = 150;
        DrawCircleV(electron, 3.0f, bright_col);
    }
}

void render_flashlight_overlay(GameState *game) {
    if (!game->flashlight_active)
        return;

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    // Capa oscura en todas partes
    DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 180});

    // Cono de linterna (círculo simple alrededor de pos pantalla jugador)
    Vector2 player_screen = {
        (game->player.position.x * CELL_SIZE) + CELL_SIZE / 2 -
            game->camera.target.x + game->camera.offset.x,
        (game->player.position.y * CELL_SIZE) + CELL_SIZE / 2 -
            game->camera.target.y + game->camera.offset.y};

    float radius = 200.0f;
    // Dibujar círculo brillante para "cortar" la oscuridad
    DrawCircleGradient((int)player_screen.x, (int)player_screen.y, radius,
                       (Color){0, 0, 0, 0}, (Color){0, 0, 0, 0});
}
