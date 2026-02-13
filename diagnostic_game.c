#include "raylib.h"
#include <stdio.h>

// Definiciones mínimas del juego
#define MAX_SOUNDS 20

typedef struct {
    Sound sound;
    const char* name;
    int play_count;
    bool loaded;
} SoundTest;

int main(void) {
    InitWindow(1200, 800, "PHASE SHIFT - Diagnóstico de Sonido");
    SetTargetFPS(60);
    
    InitAudioDevice();
    
    if (!IsAudioDeviceReady()) {
        printf("ERROR: Audio device no está listo!\n");
        CloseWindow();
        return 1;
    }
    
    printf("=== INICIANDO DIAGNÓSTICO ===\n");
    printf("Dispositivo de audio: OK\n");
    
    // Cargar todos los sonidos del juego
    SoundTest sounds[MAX_SOUNDS] = {0};
    int sound_count = 0;
    
    const char* sound_files[] = {
        "assets/sounds/footsteps.mp3",
        "assets/sounds/blast.mp3",
        "assets/sounds/key-pickup.wav",
        "assets/sounds/bomb-pickup.mp3",
        "assets/sounds/checkpoint.mp3",
        "assets/sounds/popup-show.wav",
        "assets/sounds/guard-step.mp3",
        "assets/sounds/open-door.wav",
        "assets/sounds/plant-bomb.wav"
    };
    
    for (int i = 0; i < 9; i++) {
        sounds[i].name = sound_files[i];
        sounds[i].sound = LoadSound(sound_files[i]);
        sounds[i].loaded = (sounds[i].sound.stream.buffer != NULL);
        sounds[i].play_count = 0;
        
        printf("%s: %s\n", sound_files[i], sounds[i].loaded ? "OK" : "FALLO");
    }
    sound_count = 9;
    
    // Variables de simulación del juego
    bool player_moving = false;
    float turn_animation = 0.0f;
    const float BASE_TURN_DURATION = 0.125f;
    int turn_count = 0;
    
    // Variables para simular input
    bool key_was_down = false;
    
    printf("\n=== CONTROLES ===\n");
    printf("FLECHAS o WASD = Simular movimiento del jugador\n");
    printf("1-9 = Reproducir sonidos manualmente\n");
    printf("ESC = Salir\n\n");
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        // Simular turn_animation
        if (turn_animation > 0.0f) {
            turn_animation -= dt * (1.0f / BASE_TURN_DURATION);
            if (turn_animation < 0.0f) {
                turn_animation = 0.0f;
            }
        }
        
        // Detectar input de movimiento (solo cuando turn_animation <= 0)
        bool key_down_now = false;
        if (turn_animation <= 0.0f) {
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) ||
                IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) ||
                IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) ||
                IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                
                key_down_now = true;
                
                // Solo ejecutar turno en el PRIMER frame de tecla presionada
                if (!key_was_down) {
                    // SIMULAR EXECUTE_TURN
                    turn_animation = 1.0f;
                    turn_count++;
                    player_moving = true;
                    
                    // REPRODUCIR SONIDO DE PASOS
                    if (sounds[0].loaded) {
                        PlaySound(sounds[0].sound);
                        sounds[0].play_count++;
                        printf("[TURNO %d] Reproduciendo footsteps\n", turn_count);
                    } else {
                        printf("[TURNO %d] ERROR: footsteps no cargado\n", turn_count);
                    }
                } else {
                    printf("[FRAME] Tecla aún presionada, esperando turn_animation\n");
                }
            }
        } else {
            printf("[FRAME] turn_animation = %.3f, bloqueando input\n", turn_animation);
        }
        
        key_was_down = key_down_now;
        
        // Input manual de sonidos
        if (IsKeyPressed(KEY_ONE) && sounds[0].loaded) {
            PlaySound(sounds[0].sound);
            sounds[0].play_count++;
            printf("Manual: footsteps\n");
        }
        if (IsKeyPressed(KEY_TWO) && sounds[1].loaded) {
            PlaySound(sounds[1].sound);
            sounds[1].play_count++;
            printf("Manual: blast\n");
        }
        if (IsKeyPressed(KEY_THREE) && sounds[2].loaded) {
            PlaySound(sounds[2].sound);
            sounds[2].play_count++;
            printf("Manual: key-pickup\n");
        }
        if (IsKeyPressed(KEY_FOUR) && sounds[3].loaded) {
            PlaySound(sounds[3].sound);
            sounds[3].play_count++;
            printf("Manual: bomb-pickup\n");
        }
        if (IsKeyPressed(KEY_FIVE) && sounds[4].loaded) {
            PlaySound(sounds[4].sound);
            sounds[4].play_count++;
            printf("Manual: checkpoint\n");
        }
        
        // Render
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawText("PHASE SHIFT - DIAGNÓSTICO DE SONIDO", 20, 20, 30, DARKGRAY);
        
        int y = 80;
        DrawText(TextFormat("Turnos ejecutados: %d", turn_count), 20, y, 20, BLUE);
        y += 30;
        DrawText(TextFormat("Turn Animation: %.3f", turn_animation), 20, y, 20, 
                 turn_animation > 0 ? ORANGE : GREEN);
        y += 30;
        DrawText(TextFormat("Input bloqueado: %s", 
                 turn_animation > 0 ? "SI" : "NO"), 20, y, 20,
                 turn_animation > 0 ? RED : GREEN);
        
        y += 60;
        DrawText("=== ESTADO DE SONIDOS ===", 20, y, 20, BLACK);
        y += 30;
        
        for (int i = 0; i < sound_count && i < 9; i++) {
            Color col = sounds[i].loaded ? GREEN : RED;
            const char* status = sounds[i].loaded ? "OK" : "FALLO";
            
            DrawText(TextFormat("[%d] %s", i + 1, sounds[i].name), 
                     40, y, 16, col);
            DrawText(TextFormat("%s | Reproducido: %d veces", status, sounds[i].play_count),
                     500, y, 16, col);
            
            if (sounds[i].loaded && IsSoundPlaying(sounds[i].sound)) {
                DrawText("REPRODUCIENDO", 800, y, 16, ORANGE);
            }
            
            y += 25;
        }
        
        y += 40;
        DrawText("PRESIONA FLECHAS/WASD = Simular movimiento", 20, y, 18, DARKBLUE);
        y += 25;
        DrawText("PRESIONA 1-5 = Reproducir sonidos manualmente", 20, y, 18, DARKBLUE);
        
        if (turn_animation <= 0.0f) {
            DrawText(">>> ESPERANDO INPUT <<<", 20, 700, 25, GREEN);
        } else {
            DrawText(">>> ANIMACION EN CURSO - INPUT BLOQUEADO <<<", 20, 700, 25, RED);
        }
        
        EndDrawing();
    }
    
    // Cleanup
    for (int i = 0; i < sound_count; i++) {
        if (sounds[i].loaded) {
            UnloadSound(sounds[i].sound);
        }
    }
    
    CloseAudioDevice();
    CloseWindow();
    
    printf("\n=== RESUMEN ===\n");
    printf("Turnos totales: %d\n", turn_count);
    for (int i = 0; i < sound_count; i++) {
        printf("%s: %d reproducciones\n", sounds[i].name, sounds[i].play_count);
    }
    
    return 0;
}
