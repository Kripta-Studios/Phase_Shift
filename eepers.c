#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900
#define CELL_SIZE 50.0f
#define MAX_EEPERS 30
#define MAX_ITEMS 100
#define MAX_BOMBS 10
#define MAX_ECHOS 3
#define MAX_ENTANGLED 10
#define MAX_DETECTORS 10
#define MAX_TUNNELS 5
#define MAX_ECHO_FRAMES 20
#define SUPERPOSITION_DURATION 3
#define BASE_TURN_DURATION 0.125f
#define GUARD_ATTACK_COOLDOWN 10
#define EXPLOSION_LENGTH 10
#define EYES_ANGULAR_VELOCITY 10.0f

typedef struct {
    int x, y;
} IVector2;

typedef enum {
    CELL_NONE,
    CELL_FLOOR,
    CELL_WALL,
    CELL_BARRICADE,
    CELL_DOOR,
    CELL_EXPLOSION,
    CELL_WALL_RED,
    CELL_WALL_BLUE,
    CELL_PLATFORM_RED,
    CELL_PLATFORM_BLUE
} Cell;

typedef enum {
    DIR_LEFT = 0,
    DIR_RIGHT = 1,
    DIR_UP = 2,
    DIR_DOWN = 3
} Direction;

const IVector2 DIRECTION_VECTORS[] = {
    {-1, 0}, {1, 0}, {0, -1}, {0, 1}
};

typedef enum {
    EYES_OPEN,
    EYES_CLOSED,
    EYES_ANGRY,
    EYES_CRINGE,
    EYES_SURPRISED
} EyesKind;

typedef enum {
    PHASE_RED,
    PHASE_BLUE
} PhaseKind;

typedef enum {
    PHASE_STATE_STABLE,
    PHASE_STATE_SUPERPOSITION
} PhaseState;

typedef struct {
    PhaseKind current_phase;
    PhaseState state;
    int superposition_turns_left;
    int phase_lock_turns;
} QuantumPhaseSystem;

typedef struct {
    float current;
    int decay_counter;
    int regen_counter;
} CoherenceSystem;

typedef enum {
    CMD_STEP,
    CMD_PLANT,
    CMD_PHASE_CHANGE
} CommandKind;

typedef struct {
    CommandKind kind;
    Direction dir;
} Command;

typedef struct {
    IVector2 position;
    Command action;
} EchoAction;

typedef struct {
    bool active;
    PhaseKind phase;
    EchoAction recording[MAX_ECHO_FRAMES];
    int recording_index;
    int playback_index;
    IVector2 position;
    IVector2 prev_position;
    EyesKind eyes;
    float opacity;
} QuantumEcho;

typedef struct {
    IVector2 position;
    IVector2 size;
    PhaseKind phase;
    int partner_index;
    bool is_active;
} EntangledObject;

typedef struct {
    IVector2 position;
    Direction direction;
    PhaseKind detects_phase;
    int view_distance;
    bool is_active;
    float beam_alpha;
} QuantumDetector;

typedef struct {
    IVector2 position;
    IVector2 size;
    float success_probability;
    bool last_failed;
} QuantumTunnel;

typedef struct {
    IVector2 position;
    IVector2 prev_position;
    EyesKind eyes;
    EyesKind prev_eyes;
    float eyes_angle;
    IVector2 eyes_target;
    int keys;
    int bombs;
    int bomb_slots;
    bool dead;
    double death_time;
    QuantumPhaseSystem phase_system;
    CoherenceSystem coherence;
    bool is_recording_echo;
    EchoAction current_recording[MAX_ECHO_FRAMES];
    int recording_frame;
    bool is_stuck;
    int stuck_turns;
} PlayerState;

typedef enum {
    EEPER_GUARD,
    EEPER_MOTHER,
    EEPER_GNOME,
    EEPER_FATHER
} EeperKind;

typedef struct {
    EeperKind kind;
    bool dead;
    IVector2 position;
    IVector2 prev_position;
    float eyes_angle;
    IVector2 eyes_target;
    EyesKind eyes;
    EyesKind prev_eyes;
    IVector2 size;
    int** path;
    int path_rows;
    int path_cols;
    bool damaged;
    float health;
    int attack_cooldown;
} EeperState;

typedef struct {
    IVector2 position;
    int countdown;
} BombState;

typedef enum {
    ITEM_NONE,
    ITEM_KEY,
    ITEM_BOMB_REFILL,
    ITEM_CHECKPOINT,
    ITEM_BOMB_SLOT,
    ITEM_COHERENCE_PICKUP,
    ITEM_STABILIZER
} ItemKind;

typedef struct {
    ItemKind kind;
    IVector2 position;
    int cooldown;
} Item;

typedef struct {
    Cell** data;
    int rows;
    int cols;
} Map;

typedef struct {
    Map* map;
    PlayerState player;
    EeperState eepers[MAX_EEPERS];
    float turn_animation;
    Item items[MAX_ITEMS];
    BombState bombs[MAX_BOMBS];
    Camera2D camera;
    QuantumEcho echos[MAX_ECHOS];
    EntangledObject entangled[MAX_ENTANGLED];
    QuantumDetector detectors[MAX_DETECTORS];
    QuantumTunnel tunnels[MAX_TUNNELS];
    float glitch_intensity;
    bool game_over;
    int turn_count;
} GameState;

Color PALETTE[20];
Sound footstep_sounds[4];
Sound blast_sound;
Sound key_pickup_sound;
Sound bomb_pickup_sound;
Sound checkpoint_sound;
Sound phase_shift_sound;
Music ambient_music;
Font game_font;

IVector2 ivec2(int x, int y) {
    return (IVector2){x, y};
}

IVector2 ivec2_add(IVector2 a, IVector2 b) {
    return ivec2(a.x + b.x, a.y + b.y);
}

IVector2 ivec2_sub(IVector2 a, IVector2 b) {
    return ivec2(a.x - b.x, a.y - b.y);
}

bool ivec2_eq(IVector2 a, IVector2 b) {
    return a.x == b.x && a.y == b.y;
}

Vector2 ivec2_to_vec2(IVector2 iv) {
    return (Vector2){(float)iv.x, (float)iv.y};
}

Vector2 vec2_scale(Vector2 v, float s) {
    return (Vector2){v.x * s, v.y * s};
}

Map* map_create(int rows, int cols) {
    Map* map = malloc(sizeof(Map));
    map->rows = rows;
    map->cols = cols;
    map->data = malloc(rows * sizeof(Cell*));
    for (int i = 0; i < rows; i++) {
        map->data[i] = calloc(cols, sizeof(Cell));
    }
    return map;
}

void map_free(Map* map) {
    if (!map) return;
    for (int i = 0; i < map->rows; i++) {
        free(map->data[i]);
    }
    free(map->data);
    free(map);
}

int** path_create(int rows, int cols) {
    int** path = malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        path[i] = malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            path[i][j] = -1;
        }
    }
    return path;
}

void path_free(int** path, int rows) {
    if (!path) return;
    for (int i = 0; i < rows; i++) {
        free(path[i]);
    }
    free(path);
}

void path_reset(int** path, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            path[i][j] = -1;
        }
    }
}

bool within_map(GameState* game, IVector2 pos) {
    return pos.y >= 0 && pos.y < game->map->rows &&
           pos.x >= 0 && pos.x < game->map->cols;
}

bool inside_of_rect(IVector2 start, IVector2 size, IVector2 point) {
    return point.x >= start.x && point.x < start.x + size.x &&
           point.y >= start.y && point.y < start.y + size.y;
}

Color get_cell_color(Cell cell, PhaseKind current_phase, bool in_superposition) {
    switch (cell) {
        case CELL_NONE: return BLACK;
        case CELL_FLOOR: return PALETTE[1];
        case CELL_WALL: return PALETTE[2];
        case CELL_BARRICADE: return PALETTE[3];
        case CELL_DOOR: return PALETTE[4];
        case CELL_EXPLOSION: return PALETTE[11];
        case CELL_WALL_RED:
            if (current_phase == PHASE_RED || in_superposition) {
                return (Color){255, 51, 51, 255};
            } else {
                return (Color){255, 51, 51, 76};
            }
        case CELL_WALL_BLUE:
            if (current_phase == PHASE_BLUE || in_superposition) {
                return (Color){51, 51, 255, 255};
            } else {
                return (Color){51, 51, 255, 76};
            }
        case CELL_PLATFORM_RED:
            if (current_phase == PHASE_RED || in_superposition) {
                return (Color){255, 102, 102, 255};
            } else {
                return (Color){255, 102, 102, 76};
            }
        case CELL_PLATFORM_BLUE:
            if (current_phase == PHASE_BLUE || in_superposition) {
                return (Color){102, 102, 255, 255};
            } else {
                return (Color){102, 102, 255, 76};
            }
        default: return GRAY;
    }
}

bool is_cell_solid_for_phase(Cell cell, PhaseKind phase, bool in_superposition) {
    switch (cell) {
        case CELL_WALL:
        case CELL_BARRICADE:
        case CELL_DOOR:
            return true;
        case CELL_WALL_RED:
        case CELL_PLATFORM_RED:
            return phase == PHASE_RED || in_superposition;
        case CELL_WALL_BLUE:
        case CELL_PLATFORM_BLUE:
            return phase == PHASE_BLUE || in_superposition;
        case CELL_FLOOR:
        case CELL_EXPLOSION:
        case CELL_NONE:
            return false;
        default:
            return false;
    }
}

bool eeper_can_stand_here(GameState* game, IVector2 start, int eeper_idx) {
    EeperState* eeper = &game->eepers[eeper_idx];
    
    for (int dx = 0; dx < eeper->size.x; dx++) {
        for (int dy = 0; dy < eeper->size.y; dy++) {
            IVector2 pos = ivec2(start.x + dx, start.y + dy);
            
            if (!within_map(game, pos)) return false;
            
            Cell cell = game->map->data[pos.y][pos.x];
            if (cell != CELL_FLOOR && cell != CELL_EXPLOSION) {
                return false;
            }
            
            for (int i = 0; i < MAX_EEPERS; i++) {
                if (i == eeper_idx || game->eepers[i].dead) continue;
                
                EeperState* other = &game->eepers[i];
                if (inside_of_rect(other->position, other->size, pos)) {
                    return false;
                }
            }
        }
    }
    return true;
}

void init_palette(void) {
    PALETTE[0] = (Color){20, 20, 30, 255};
    PALETTE[1] = (Color){200, 200, 200, 255};
    PALETTE[2] = (Color){50, 50, 50, 255};
    PALETTE[3] = (Color){100, 50, 50, 255};
    PALETTE[4] = (Color){0, 255, 255, 255};
    PALETTE[5] = (Color){0, 120, 255, 255};
    PALETTE[6] = (Color){255, 200, 0, 255};
    PALETTE[7] = (Color){255, 255, 255, 255};
    PALETTE[8] = (Color){0, 255, 0, 255};
    PALETTE[9] = (Color){255, 150, 0, 255};
    PALETTE[10] = (Color){255, 0, 255, 255};
    PALETTE[11] = (Color){255, 100, 0, 255};
    PALETTE[12] = (Color){255, 0, 0, 255};
    PALETTE[13] = (Color){255, 255, 0, 255};
    PALETTE[14] = (Color){100, 100, 255, 255};
}

void init_game_state(GameState* game, int rows, int cols) {
    memset(game, 0, sizeof(GameState));
    
    game->map = map_create(rows, cols);
    game->player.position = ivec2(1, 1);
    game->player.prev_position = game->player.position;
    game->player.eyes = EYES_OPEN;
    game->player.prev_eyes = EYES_CLOSED;
    game->player.eyes_angle = M_PI * 0.5f;
    game->player.eyes_target = ivec2(1, 0);
    game->player.bombs = 0;
    game->player.bomb_slots = 1;
    game->player.keys = 0;
    game->player.dead = false;
    game->player.phase_system.current_phase = PHASE_RED;
    game->player.phase_system.state = PHASE_STATE_STABLE;
    game->player.phase_system.superposition_turns_left = 0;
    game->player.phase_system.phase_lock_turns = 0;
    game->player.coherence.current = 100.0f;
    game->player.coherence.decay_counter = 0;
    game->player.coherence.regen_counter = 0;
    game->player.is_recording_echo = false;
    game->player.recording_frame = 0;
    game->player.is_stuck = false;
    game->player.stuck_turns = 0;
    
    for (int i = 0; i < MAX_EEPERS; i++) {
        game->eepers[i].dead = true;
        game->eepers[i].path = path_create(rows, cols);
        game->eepers[i].path_rows = rows;
        game->eepers[i].path_cols = cols;
    }
    
    for (int i = 0; i < MAX_ECHOS; i++) {
        game->echos[i].active = false;
    }
    
    for (int i = 0; i < MAX_DETECTORS; i++) {
        game->detectors[i].is_active = false;
    }
    
    game->camera.offset = (Vector2){SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f};
    game->camera.target = (Vector2){CELL_SIZE, CELL_SIZE};
    game->camera.rotation = 0.0f;
    game->camera.zoom = 1.0f;
    game->turn_animation = 0.0f;
    game->game_over = false;
    game->turn_count = 0;
}

void spawn_guard(GameState* game, IVector2 pos) {
    for (int i = 0; i < MAX_EEPERS; i++) {
        if (game->eepers[i].dead) {
            game->eepers[i].dead = false;
            game->eepers[i].kind = EEPER_GUARD;
            game->eepers[i].position = pos;
            game->eepers[i].prev_position = pos;
            game->eepers[i].size = ivec2(3, 3);
            game->eepers[i].eyes = EYES_CLOSED;
            game->eepers[i].prev_eyes = EYES_CLOSED;
            game->eepers[i].eyes_angle = M_PI * 0.5f;
            game->eepers[i].eyes_target = ivec2_add(pos, ivec2(1, 3));
            game->eepers[i].health = 1.0f;
            game->eepers[i].attack_cooldown = GUARD_ATTACK_COOLDOWN;
            return;
        }
    }
}

void spawn_gnome(GameState* game, IVector2 pos) {
    for (int i = 0; i < MAX_EEPERS; i++) {
        if (game->eepers[i].dead) {
            game->eepers[i].dead = false;
            game->eepers[i].kind = EEPER_GNOME;
            game->eepers[i].position = pos;
            game->eepers[i].prev_position = pos;
            game->eepers[i].size = ivec2(1, 1);
            game->eepers[i].eyes = EYES_CLOSED;
            game->eepers[i].prev_eyes = EYES_CLOSED;
            game->eepers[i].eyes_angle = M_PI * 0.5f;
            game->eepers[i].eyes_target = ivec2_add(pos, ivec2(0, 1));
            game->eepers[i].health = 1.0f;
            return;
        }
    }
}

void allocate_item(GameState* game, IVector2 pos, ItemKind kind) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (game->items[i].kind == ITEM_NONE) {
            game->items[i].kind = kind;
            game->items[i].position = pos;
            game->items[i].cooldown = 0;
            return;
        }
    }
}

void spawn_detector(GameState* game, IVector2 pos, Direction dir, PhaseKind phase) {
    for (int i = 0; i < MAX_DETECTORS; i++) {
        if (!game->detectors[i].is_active) {
            game->detectors[i].is_active = true;
            game->detectors[i].position = pos;
            game->detectors[i].direction = dir;
            game->detectors[i].detects_phase = phase;
            game->detectors[i].view_distance = 5;
            game->detectors[i].beam_alpha = 0.0f;
            return;
        }
    }
}

void spawn_tunnel(GameState* game, IVector2 pos, IVector2 size) {
    for (int i = 0; i < MAX_TUNNELS; i++) {
        if (!game->tunnels[i].last_failed) {
            game->tunnels[i].position = pos;
            game->tunnels[i].size = size;
            game->tunnels[i].success_probability = 0.5f;
            game->tunnels[i].last_failed = false;
            return;
        }
    }
}

void load_simple_level(GameState* game) {
    for (int y = 0; y < game->map->rows; y++) {
        for (int x = 0; x < game->map->cols; x++) {
            if (x == 0 || x == game->map->cols - 1 || 
                y == 0 || y == game->map->rows - 1) {
                game->map->data[y][x] = CELL_WALL;
            } else {
                game->map->data[y][x] = CELL_FLOOR;
            }
        }
    }
    
    for (int i = 5; i < 10; i++) {
        game->map->data[5][i] = CELL_WALL_RED;
        game->map->data[10][i] = CELL_WALL_BLUE;
    }
    
    game->map->data[7][7] = CELL_PLATFORM_RED;
    game->map->data[7][8] = CELL_PLATFORM_RED;
    game->map->data[13][7] = CELL_PLATFORM_BLUE;
    game->map->data[13][8] = CELL_PLATFORM_BLUE;
    
    game->player.position = ivec2(3, 3);
    game->player.prev_position = game->player.position;
    
    allocate_item(game, ivec2(6, 3), ITEM_KEY);
    allocate_item(game, ivec2(8, 3), ITEM_BOMB_REFILL);
    allocate_item(game, ivec2(10, 3), ITEM_CHECKPOINT);
    allocate_item(game, ivec2(12, 3), ITEM_COHERENCE_PICKUP);
    allocate_item(game, ivec2(14, 3), ITEM_STABILIZER);
    
    spawn_guard(game, ivec2(10, 10));
    spawn_gnome(game, ivec2(15, 8));
    
    spawn_detector(game, ivec2(8, 6), DIR_DOWN, PHASE_RED);
    spawn_detector(game, ivec2(12, 12), DIR_RIGHT, PHASE_BLUE);
    
    game->map->data[15][5] = CELL_DOOR;
}

typedef struct {
    IVector2* items;
    int size;
    int capacity;
} Queue;

void queue_init(Queue* q) {
    q->capacity = 256;
    q->size = 0;
    q->items = malloc(q->capacity * sizeof(IVector2));
}

void queue_push(Queue* q, IVector2 item) {
    if (q->size >= q->capacity) {
        q->capacity *= 2;
        q->items = realloc(q->items, q->capacity * sizeof(IVector2));
    }
    q->items[q->size++] = item;
}

IVector2 queue_pop(Queue* q) {
    IVector2 result = q->items[0];
    q->size--;
    for (int i = 0; i < q->size; i++) {
        q->items[i] = q->items[i + 1];
    }
    return result;
}

void queue_free(Queue* q) {
    free(q->items);
}

void recompute_path_for_eeper(GameState* game, int eeper_idx) {
    EeperState* eeper = &game->eepers[eeper_idx];
    Queue q;
    queue_init(&q);
    
    path_reset(eeper->path, eeper->path_rows, eeper->path_cols);
    
    for (int dy = 0; dy < eeper->size.y; dy++) {
        for (int dx = 0; dx < eeper->size.x; dx++) {
            IVector2 pos = ivec2_sub(game->player.position, ivec2(dx, dy));
            if (eeper_can_stand_here(game, pos, eeper_idx)) {
                eeper->path[pos.y][pos.x] = 0;
                queue_push(&q, pos);
            }
        }
    }
    
    while (q.size > 0) {
        IVector2 pos = queue_pop(&q);
        
        if (ivec2_eq(pos, eeper->position)) {
            break;
        }
        
        if (eeper->path[pos.y][pos.x] >= 10) {
            break;
        }
        
        for (int dir = 0; dir < 4; dir++) {
            IVector2 new_pos = ivec2_add(pos, DIRECTION_VECTORS[dir]);
            
            for (int step = 1; step <= 100; step++) {
                if (!eeper_can_stand_here(game, new_pos, eeper_idx)) break;
                if (eeper->path[new_pos.y][new_pos.x] >= 0) break;
                
                eeper->path[new_pos.y][new_pos.x] = eeper->path[pos.y][pos.x] + 1;
                queue_push(&q, new_pos);
                
                new_pos = ivec2_add(new_pos, DIRECTION_VECTORS[dir]);
            }
        }
    }
    
    queue_free(&q);
}

void kill_player(GameState* game) {
    game->player.dead = true;
    game->player.death_time = GetTime();
}

void flood_fill(GameState* game, IVector2 start, Cell fill) {
    if (!within_map(game, start)) return;
    
    Cell background = game->map->data[start.y][start.x];
    game->map->data[start.y][start.x] = fill;
    
    Queue q;
    queue_init(&q);
    queue_push(&q, start);
    
    while (q.size > 0) {
        IVector2 pos = queue_pop(&q);
        
        for (int dir = 0; dir < 4; dir++) {
            IVector2 new_pos = ivec2_add(pos, DIRECTION_VECTORS[dir]);
            
            if (within_map(game, new_pos) && 
                game->map->data[new_pos.y][new_pos.x] == background) {
                game->map->data[new_pos.y][new_pos.x] = fill;
                queue_push(&q, new_pos);
            }
        }
    }
    
    queue_free(&q);
}

void explode_line(GameState* game, IVector2 position, Direction dir) {
    IVector2 new_pos = position;
    
    for (int i = 0; i < EXPLOSION_LENGTH; i++) {
        if (!within_map(game, new_pos)) return;
        
        Cell cell = game->map->data[new_pos.y][new_pos.x];
        
        if (cell == CELL_FLOOR || cell == CELL_EXPLOSION) {
            game->map->data[new_pos.y][new_pos.x] = CELL_EXPLOSION;
            
            if (ivec2_eq(new_pos, game->player.position)) {
                kill_player(game);
            }
            
            for (int e = 0; e < MAX_EEPERS; e++) {
                if (!game->eepers[e].dead && 
                    inside_of_rect(game->eepers[e].position, 
                                  game->eepers[e].size, 
                                  new_pos)) {
                    game->eepers[e].damaged = true;
                }
            }
            
            new_pos = ivec2_add(new_pos, DIRECTION_VECTORS[dir]);
        } else if (cell == CELL_BARRICADE) {
            flood_fill(game, new_pos, CELL_EXPLOSION);
            game->map->data[new_pos.y][new_pos.x] = CELL_EXPLOSION;
            return;
        } else {
            return;
        }
    }
}

void explode(GameState* game, IVector2 position) {
    for (int dir = 0; dir < 4; dir++) {
        explode_line(game, position, dir);
    }
}

void update_phase_system(GameState* game) {
    PlayerState* player = &game->player;
    QuantumPhaseSystem* phase = &player->phase_system;
    
    if (phase->phase_lock_turns > 0) {
        phase->phase_lock_turns--;
        return;
    }
    
    if (phase->state == PHASE_STATE_SUPERPOSITION) {
        phase->superposition_turns_left--;
        
        if (phase->superposition_turns_left <= 0) {
            phase->state = PHASE_STATE_STABLE;
            phase->current_phase = (phase->current_phase == PHASE_RED) 
                                  ? PHASE_BLUE : PHASE_RED;
            
            for (int i = 0; i < MAX_ECHOS; i++) {
                if (!game->echos[i].active) {
                    game->echos[i].active = true;
                    game->echos[i].phase = (phase->current_phase == PHASE_RED) 
                                          ? PHASE_BLUE : PHASE_RED;
                    memcpy(game->echos[i].recording, 
                          player->current_recording,
                          sizeof(EchoAction) * MAX_ECHO_FRAMES);
                    game->echos[i].recording_index = player->recording_frame;
                    game->echos[i].playback_index = 0;
                    game->echos[i].position = player->position;
                    game->echos[i].prev_position = player->position;
                    game->echos[i].eyes = EYES_OPEN;
                    game->echos[i].opacity = 0.5f;
                    break;
                }
            }
            
            player->is_recording_echo = false;
        }
    }
}

void update_coherence(GameState* game) {
    CoherenceSystem* coh = &game->player.coherence;
    
    coh->decay_counter++;
    if (coh->decay_counter >= 5) {
        coh->current -= 1.0f;
        coh->decay_counter = 0;
    }
    
    if (coh->current < 30.0f) {
        game->glitch_intensity = (30.0f - coh->current) / 30.0f;
    } else {
        game->glitch_intensity = 0.0f;
    }
    
    if (coh->current <= 0.0f) {
        kill_player(game);
    }
}

void update_quantum_echos(GameState* game) {
    for (int i = 0; i < MAX_ECHOS; i++) {
        QuantumEcho* echo = &game->echos[i];
        if (!echo->active) continue;
        
        echo->prev_position = echo->position;
        
        if (echo->playback_index < echo->recording_index) {
            EchoAction* action = &echo->recording[echo->playback_index];
            
            if (action->action.kind == CMD_STEP) {
                echo->position = ivec2_add(echo->position, 
                                          DIRECTION_VECTORS[action->action.dir]);
            } else if (action->action.kind == CMD_PLANT) {
                for (int b = 0; b < MAX_BOMBS; b++) {
                    if (game->bombs[b].countdown <= 0) {
                        game->bombs[b].countdown = 3;
                        game->bombs[b].position = echo->position;
                        break;
                    }
                }
            }
            
            echo->playback_index++;
        } else {
            echo->active = false;
            echo->opacity = 0.0f;
        }
        
        if (echo->playback_index > echo->recording_index - 5) {
            echo->opacity = (float)(echo->recording_index - echo->playback_index) / 5.0f;
        }
    }
}

void update_quantum_detectors(GameState* game) {
    PlayerState* player = &game->player;
    
    for (int i = 0; i < MAX_DETECTORS; i++) {
        QuantumDetector* det = &game->detectors[i];
        if (!det->is_active) continue;
        
        IVector2 ray_pos = det->position;
        bool detected = false;
        
        for (int dist = 1; dist <= det->view_distance; dist++) {
            ray_pos = ivec2_add(ray_pos, DIRECTION_VECTORS[det->direction]);
            
            if (!within_map(game, ray_pos)) break;
            
            if (ivec2_eq(ray_pos, player->position)) {
                if (player->phase_system.current_phase == det->detects_phase ||
                    player->phase_system.state == PHASE_STATE_SUPERPOSITION) {
                    detected = true;
                    break;
                }
            }
            
            if (game->map->data[ray_pos.y][ray_pos.x] == CELL_WALL) {
                break;
            }
        }
        
        if (detected) {
            player->phase_system.current_phase = det->detects_phase;
            player->phase_system.state = PHASE_STATE_STABLE;
            player->phase_system.phase_lock_turns = 5;
            player->coherence.current -= 15.0f;
            
            det->beam_alpha = 1.0f;
        } else {
            det->beam_alpha *= 0.9f;
        }
    }
}

bool attempt_quantum_tunnel(GameState* game, int tunnel_idx) {
    QuantumTunnel* tunnel = &game->tunnels[tunnel_idx];
    PlayerState* player = &game->player;
    
    if (player->phase_system.state != PHASE_STATE_SUPERPOSITION) {
        return false;
    }
    
    if (!inside_of_rect(tunnel->position, tunnel->size, player->position)) {
        return false;
    }
    
    float success_chance = tunnel->success_probability;
    
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (game->items[i].kind == ITEM_STABILIZER) {
            success_chance = 0.75f;
            break;
        }
    }
    
    float random_val = (float)(rand() % 100) / 100.0f;
    
    if (random_val < success_chance) {
        player->position = ivec2_add(tunnel->position, tunnel->size);
        return true;
    } else {
        player->is_stuck = true;
        player->stuck_turns = 2;
        player->coherence.current -= 20.0f;
        tunnel->last_failed = true;
        return false;
    }
}

void game_player_turn(GameState* game, Direction dir) {
    PlayerState* player = &game->player;
    
    player->prev_position = player->position;
    player->prev_eyes = player->eyes;
    
    IVector2 new_pos = ivec2_add(player->position, DIRECTION_VECTORS[dir]);
    player->eyes_target = ivec2_add(new_pos, DIRECTION_VECTORS[dir]);
    
    if (!within_map(game, new_pos)) return;
    
    Cell cell = game->map->data[new_pos.y][new_pos.x];
    bool in_superposition = player->phase_system.state == PHASE_STATE_SUPERPOSITION;
    
    if (!is_cell_solid_for_phase(cell, player->phase_system.current_phase, in_superposition)) {
        player->position = new_pos;
        
        for (int i = 0; i < MAX_ITEMS; i++) {
            Item* item = &game->items[i];
            if (item->kind == ITEM_NONE) continue;
            if (!ivec2_eq(item->position, new_pos)) continue;
            
            switch (item->kind) {
                case ITEM_KEY:
                    player->keys++;
                    item->kind = ITEM_NONE;
                    PlaySound(key_pickup_sound);
                    break;
                case ITEM_BOMB_REFILL:
                    if (player->bombs < player->bomb_slots && item->cooldown <= 0) {
                        player->bombs++;
                        item->cooldown = 10;
                        PlaySound(bomb_pickup_sound);
                    }
                    break;
                case ITEM_BOMB_SLOT:
                    item->kind = ITEM_NONE;
                    player->bomb_slots++;
                    player->bombs = player->bomb_slots;
                    break;
                case ITEM_CHECKPOINT:
                    item->kind = ITEM_NONE;
                    player->bombs = player->bomb_slots;
                    player->coherence.current = 100.0f;
                    PlaySound(checkpoint_sound);
                    break;
                case ITEM_COHERENCE_PICKUP:
                    item->kind = ITEM_NONE;
                    player->coherence.current = fminf(100.0f, player->coherence.current + 5.0f);
                    break;
                case ITEM_STABILIZER:
                    break;
                default:
                    break;
            }
        }
    } else if (cell == CELL_DOOR) {
        if (player->keys > 0) {
            player->keys--;
            flood_fill(game, new_pos, CELL_FLOOR);
            player->position = new_pos;
        }
    }
    
    if (player->is_recording_echo && player->recording_frame < MAX_ECHO_FRAMES) {
        player->current_recording[player->recording_frame].position = player->position;
        player->current_recording[player->recording_frame].action.kind = CMD_STEP;
        player->current_recording[player->recording_frame].action.dir = dir;
        player->recording_frame++;
    }
    
    PlaySound(footstep_sounds[rand() % 4]);
}

void game_bombs_turn(GameState* game) {
    for (int e = 0; e < MAX_EEPERS; e++) {
        game->eepers[e].damaged = false;
    }
    
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (game->bombs[i].countdown > 0) {
            game->bombs[i].countdown--;
            if (game->bombs[i].countdown <= 0) {
                PlaySound(blast_sound);
                explode(game, game->bombs[i].position);
            }
        }
    }
    
    for (int e = 0; e < MAX_EEPERS; e++) {
        EeperState* eeper = &game->eepers[e];
        if (!eeper->dead && eeper->damaged) {
            switch (eeper->kind) {
                case EEPER_GUARD:
                    eeper->eyes = EYES_CRINGE;
                    eeper->health -= 0.45f;
                    if (eeper->health <= 0.0f) {
                        eeper->dead = true;
                    }
                    break;
                case EEPER_GNOME:
                    eeper->dead = true;
                    allocate_item(game, eeper->position, ITEM_KEY);
                    break;
                default:
                    break;
            }
        }
    }
}

void game_explosions_turn(GameState* game) {
    for (int y = 0; y < game->map->rows; y++) {
        for (int x = 0; x < game->map->cols; x++) {
            if (game->map->data[y][x] == CELL_EXPLOSION) {
                game->map->data[y][x] = CELL_FLOOR;
            }
        }
    }
}

void game_items_turn(GameState* game) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (game->items[i].kind == ITEM_BOMB_REFILL) {
            if (game->items[i].cooldown > 0) {
                game->items[i].cooldown--;
            }
        }
    }
}

void game_eepers_turn(GameState* game) {
    for (int i = 0; i < MAX_EEPERS; i++) {
        EeperState* eeper = &game->eepers[i];
        if (eeper->dead) continue;
        
        eeper->prev_position = eeper->position;
        eeper->prev_eyes = eeper->eyes;
        
        switch (eeper->kind) {
            case EEPER_GUARD: {
                recompute_path_for_eeper(game, i);
                IVector2 pos = eeper->position;
                int dist = eeper->path[pos.y][pos.x];
                
                if (dist == 0) {
                    kill_player(game);
                    eeper->eyes = EYES_SURPRISED;
                } else if (dist > 0) {
                    if (eeper->attack_cooldown <= 0) {
                        IVector2 best_moves[4];
                        int count = 0;
                        
                        for (int dir = 0; dir < 4; dir++) {
                            IVector2 test_pos = pos;
                            while (eeper_can_stand_here(game, test_pos, i)) {
                                test_pos = ivec2_add(test_pos, DIRECTION_VECTORS[dir]);
                                if (within_map(game, test_pos) &&
                                    eeper->path[test_pos.y][test_pos.x] == dist - 1) {
                                    best_moves[count++] = test_pos;
                                    break;
                                }
                            }
                        }
                        
                        if (count > 0) {
                            eeper->position = best_moves[rand() % count];
                        }
                        
                        eeper->attack_cooldown = GUARD_ATTACK_COOLDOWN;
                    } else {
                        eeper->attack_cooldown--;
                    }
                    
                    if (dist == 1) {
                        eeper->eyes = EYES_ANGRY;
                    } else {
                        eeper->eyes = EYES_OPEN;
                    }
                    eeper->eyes_target = game->player.position;
                    
                    if (inside_of_rect(eeper->position, eeper->size, game->player.position)) {
                        kill_player(game);
                    }
                } else {
                    eeper->eyes = EYES_CLOSED;
                    eeper->eyes_target = ivec2_add(eeper->position, ivec2(1, 3));
                    eeper->attack_cooldown = GUARD_ATTACK_COOLDOWN + 1;
                }
                
                if (eeper->health < 1.0f) {
                    eeper->health += 0.01f;
                }
                break;
            }
            case EEPER_GNOME: {
                recompute_path_for_eeper(game, i);
                IVector2 pos = eeper->position;
                
                if (eeper->path[pos.y][pos.x] >= 0) {
                    IVector2 available[4];
                    int count = 0;
                    
                    for (int dir = 0; dir < 4; dir++) {
                        IVector2 new_pos = ivec2_add(pos, DIRECTION_VECTORS[dir]);
                        if (within_map(game, new_pos) &&
                            game->map->data[new_pos.y][new_pos.x] == CELL_FLOOR &&
                            eeper->path[new_pos.y][new_pos.x] > eeper->path[pos.y][pos.x]) {
                            available[count++] = new_pos;
                        }
                    }
                    
                    if (count > 0) {
                        eeper->position = available[rand() % count];
                    }
                    eeper->eyes = EYES_OPEN;
                    eeper->eyes_target = game->player.position;
                } else {
                    eeper->eyes = EYES_CLOSED;
                    eeper->eyes_target = ivec2_add(eeper->position, ivec2(0, 1));
                }
                break;
            }
            default:
                break;
        }
    }
}

void handle_phase_change(GameState* game) {
    PlayerState* player = &game->player;
    
    if (player->phase_system.phase_lock_turns > 0) {
        return;
    }
    
    if (player->phase_system.state == PHASE_STATE_STABLE) {
        player->phase_system.state = PHASE_STATE_SUPERPOSITION;
        player->phase_system.superposition_turns_left = SUPERPOSITION_DURATION;
        player->is_recording_echo = true;
        player->recording_frame = 0;
        
        if (player->recording_frame < MAX_ECHO_FRAMES) {
            player->current_recording[player->recording_frame].position = player->position;
            player->current_recording[player->recording_frame].action.kind = CMD_PHASE_CHANGE;
            player->recording_frame++;
        }
    }
}

void handle_plant_bomb(GameState* game) {
    PlayerState* player = &game->player;
    
    if (player->bombs > 0) {
        for (int i = 0; i < MAX_BOMBS; i++) {
            if (game->bombs[i].countdown <= 0) {
                game->bombs[i].countdown = 3;
                game->bombs[i].position = player->position;
                break;
            }
        }
        player->bombs--;
        
        if (player->is_recording_echo && player->recording_frame < MAX_ECHO_FRAMES) {
            player->current_recording[player->recording_frame].position = player->position;
            player->current_recording[player->recording_frame].action.kind = CMD_PLANT;
            player->recording_frame++;
        }
    }
}

void execute_turn(GameState* game, Command cmd) {
    if (game->player.is_stuck) {
        game->player.stuck_turns--;
        if (game->player.stuck_turns <= 0) {
            game->player.is_stuck = false;
        }
        return;
    }
    
    game->turn_animation = 1.0f;
    game->turn_count++;
    
    game_explosions_turn(game);
    game_items_turn(game);
    
    if (cmd.kind == CMD_STEP) {
        game_player_turn(game, cmd.dir);
    } else if (cmd.kind == CMD_PLANT) {
        handle_plant_bomb(game);
    } else if (cmd.kind == CMD_PHASE_CHANGE) {
        handle_phase_change(game);
    }
    
    game_bombs_turn(game);
    game_eepers_turn(game);
    update_phase_system(game);
    update_coherence(game);
    update_quantum_echos(game);
    update_quantum_detectors(game);
}

Vector2 interpolate_positions(IVector2 prev, IVector2 curr, float t) {
    Vector2 p = vec2_scale(ivec2_to_vec2(prev), CELL_SIZE);
    Vector2 c = vec2_scale(ivec2_to_vec2(curr), CELL_SIZE);
    float factor = 1.0f - t * t;
    return Vector2Lerp(p, c, factor);
}

void draw_eyes(Vector2 start, Vector2 size, float angle, EyesKind kind) {
    Vector2 dir = {cosf(angle), sinf(angle)};
    Vector2 eyes_size = {size.x * 0.2f, size.y * 0.35f};
    Vector2 center = {start.x + size.x * 0.5f, start.y + size.y * 0.5f};
    Vector2 position = {
        center.x + dir.x * eyes_size.x * 0.6f,
        center.y + dir.y * eyes_size.x * 0.6f
    };
    
    Vector2 left_eye = {
        position.x - eyes_size.x * 0.5f,
        position.y - eyes_size.y * 0.5f
    };
    Vector2 right_eye = {
        position.x + eyes_size.x * 0.5f,
        position.y - eyes_size.y * 0.5f
    };
    
    float eye_height = eyes_size.y;
    if (kind == EYES_CLOSED) {
        eye_height = eyes_size.y * 0.2f;
    } else if (kind == EYES_SURPRISED) {
        eye_height = eyes_size.y * 1.3f;
    }
    
    DrawRectangleV(left_eye, (Vector2){eyes_size.x, eye_height}, PALETTE[13]);
    DrawRectangleV(right_eye, (Vector2){eyes_size.x, eye_height}, PALETTE[13]);
}

void render_game_cells(GameState* game) {
    PhaseKind phase = game->player.phase_system.current_phase;
    bool in_superposition = game->player.phase_system.state == PHASE_STATE_SUPERPOSITION;
    
    for (int y = 0; y < game->map->rows; y++) {
        for (int x = 0; x < game->map->cols; x++) {
            Vector2 pos = {x * CELL_SIZE, y * CELL_SIZE};
            Cell cell = game->map->data[y][x];
            Color color = get_cell_color(cell, phase, in_superposition);
            DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE}, color);
        }
    }
}

void render_items(GameState* game) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        Item* item = &game->items[i];
        if (item->kind == ITEM_NONE) continue;
        
        Vector2 pos = vec2_scale(ivec2_to_vec2(item->position), CELL_SIZE);
        Vector2 center = {pos.x + CELL_SIZE * 0.5f, pos.y + CELL_SIZE * 0.5f};
        
        switch (item->kind) {
            case ITEM_KEY:
                DrawCircleV(center, CELL_SIZE * 0.25f, PALETTE[4]);
                break;
            case ITEM_BOMB_REFILL:
                if (item->cooldown > 0) {
                    DrawCircleV(center, CELL_SIZE * 0.5f, ColorBrightness(PALETTE[6], -0.5f));
                } else {
                    DrawCircleV(center, CELL_SIZE * 0.5f, PALETTE[6]);
                }
                break;
            case ITEM_CHECKPOINT:
                DrawRectangleV((Vector2){center.x - CELL_SIZE * 0.25f, 
                                        center.y - CELL_SIZE * 0.25f},
                             (Vector2){CELL_SIZE * 0.5f, CELL_SIZE * 0.5f},
                             PALETTE[10]);
                break;
            case ITEM_COHERENCE_PICKUP:
                DrawCircleV(center, CELL_SIZE * 0.3f, PALETTE[13]);
                break;
            case ITEM_STABILIZER:
                DrawRectangleV((Vector2){center.x - CELL_SIZE * 0.3f,
                                        center.y - CELL_SIZE * 0.3f},
                             (Vector2){CELL_SIZE * 0.6f, CELL_SIZE * 0.6f},
                             PALETTE[5]);
                break;
            default:
                break;
        }
    }
}

void render_player(GameState* game) {
    Vector2 pos;
    if (game->turn_animation > 0.0f) {
        pos = interpolate_positions(game->player.prev_position, 
                                   game->player.position, 
                                   game->turn_animation);
    } else {
        pos = vec2_scale(ivec2_to_vec2(game->player.position), CELL_SIZE);
    }
    
    DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE}, PALETTE[5]);
    draw_eyes(pos, (Vector2){CELL_SIZE, CELL_SIZE}, game->player.eyes_angle, game->player.eyes);
}

void render_eepers(GameState* game) {
    for (int i = 0; i < MAX_EEPERS; i++) {
        EeperState* eeper = &game->eepers[i];
        if (eeper->dead) continue;
        
        Vector2 pos;
        if (game->turn_animation > 0.0f) {
            pos = interpolate_positions(eeper->prev_position, eeper->position, game->turn_animation);
        } else {
            pos = vec2_scale(ivec2_to_vec2(eeper->position), CELL_SIZE);
        }
        
        Vector2 size = vec2_scale(ivec2_to_vec2(eeper->size), CELL_SIZE);
        
        switch (eeper->kind) {
            case EEPER_GUARD:
                DrawRectangleV(pos, size, PALETTE[8]);
                if (eeper->health < 1.0f) {
                    Vector2 health_bar_pos = {pos.x, pos.y - 15.0f};
                    DrawRectangleV(health_bar_pos, 
                                 (Vector2){size.x * eeper->health, 10.0f}, 
                                 PALETTE[12]);
                }
                draw_eyes(pos, size, eeper->eyes_angle, eeper->eyes);
                break;
            case EEPER_GNOME: {
                Vector2 gnome_size = {size.x * 0.7f, size.y * 0.7f};
                Vector2 gnome_pos = {
                    pos.x + (size.x - gnome_size.x) * 0.5f,
                    pos.y + (size.y - gnome_size.y) * 0.5f
                };
                DrawRectangleV(gnome_pos, gnome_size, PALETTE[9]);
                draw_eyes(gnome_pos, gnome_size, eeper->eyes_angle, eeper->eyes);
                break;
            }
            default:
                break;
        }
    }
}

void render_bombs(GameState* game) {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (game->bombs[i].countdown > 0) {
            Vector2 pos = vec2_scale(ivec2_to_vec2(game->bombs[i].position), CELL_SIZE);
            Vector2 center = {pos.x + CELL_SIZE * 0.5f, pos.y + CELL_SIZE * 0.5f};
            DrawCircleV(center, CELL_SIZE * 0.5f, PALETTE[6]);
            
            char text[4];
            sprintf(text, "%d", game->bombs[i].countdown);
            int text_width = MeasureText(text, 32);
            DrawText(text, 
                    (int)(center.x - text_width * 0.5f), 
                    (int)(center.y - 16), 
                    32, 
                    PALETTE[7]);
        }
    }
}

void render_quantum_effects(GameState* game) {
    for (int i = 0; i < MAX_ECHOS; i++) {
        QuantumEcho* echo = &game->echos[i];
        if (!echo->active) continue;
        
        Vector2 pos = vec2_scale(ivec2_to_vec2(echo->position), CELL_SIZE);
        Color echo_color = (echo->phase == PHASE_RED) 
                          ? (Color){255, 50, 50, 255} 
                          : (Color){50, 150, 255, 255};
        echo_color.a = (unsigned char)(echo->opacity * 255.0f);
        
        DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE}, echo_color);
        
        for (int j = 1; j <= 3; j++) {
            Vector2 trail = {pos.x + CELL_SIZE * 0.5f - j * 2.0f, 
                           pos.y + CELL_SIZE * 0.5f - j * 2.0f};
            Color trail_color = echo_color;
            trail_color.a = (unsigned char)(echo->opacity * 128.0f / (j * 2));
            DrawCircleV(trail, 3.0f, trail_color);
        }
    }
    
    if (game->player.phase_system.state == PHASE_STATE_SUPERPOSITION) {
        Vector2 pos = vec2_scale(ivec2_to_vec2(game->player.position), CELL_SIZE);
        
        DrawRectangleV(pos, (Vector2){CELL_SIZE, CELL_SIZE}, 
                      (Color){255, 0, 0, 100});
        DrawRectangleV((Vector2){pos.x + 2, pos.y + 2}, 
                      (Vector2){CELL_SIZE, CELL_SIZE},
                      (Color){0, 100, 255, 100});
        
        double time = GetTime();
        for (int i = 0; i < 8; i++) {
            float angle = (float)i * (2.0f * PI / 8.0f) + (float)time * 5.0f;
            float radius = 30.0f;
            Vector2 particle = {
                pos.x + CELL_SIZE * 0.5f + cosf(angle) * radius,
                pos.y + CELL_SIZE * 0.5f + sinf(angle) * radius
            };
            DrawCircleV(particle, 3.0f, (Color){255, 255, 0, 200});
        }
    }
    
    for (int i = 0; i < MAX_DETECTORS; i++) {
        QuantumDetector* det = &game->detectors[i];
        if (!det->is_active || det->beam_alpha < 0.01f) continue;
        
        Vector2 start = vec2_scale(ivec2_to_vec2(det->position), CELL_SIZE);
        Vector2 end = start;
        for (int d = 0; d < det->view_distance; d++) {
            end.x += DIRECTION_VECTORS[det->direction].x * CELL_SIZE;
            end.y += DIRECTION_VECTORS[det->direction].y * CELL_SIZE;
        }
        
        Color beam_color = {255, 0, 0, (unsigned char)(det->beam_alpha * 150)};
        DrawLineEx((Vector2){start.x + CELL_SIZE * 0.5f, start.y + CELL_SIZE * 0.5f},
                  (Vector2){end.x + CELL_SIZE * 0.5f, end.y + CELL_SIZE * 0.5f},
                  5.0f,
                  beam_color);
    }
    
    if (game->glitch_intensity > 0.01f) {
        int glitch_lines = (int)(game->glitch_intensity * 10.0f);
        for (int i = 0; i < glitch_lines; i++) {
            int y_pos = rand() % SCREEN_HEIGHT;
            DrawRectangle(0, y_pos, SCREEN_WIDTH, 2,
                        (Color){255, 0, 0, (unsigned char)(game->glitch_intensity * 100)});
        }
    }
}

void render_hud(GameState* game) {
    for (int i = 0; i < game->player.keys; i++) {
        DrawCircleV((Vector2){100.0f + i * CELL_SIZE, 100.0f}, 
                   CELL_SIZE * 0.25f, 
                   PALETTE[4]);
    }
    
    for (int i = 0; i < game->player.bomb_slots; i++) {
        float x = 100.0f + i * (CELL_SIZE + CELL_SIZE * 0.5f);
        Color bomb_color = (i < game->player.bombs) 
                          ? PALETTE[6] 
                          : ColorBrightness(PALETTE[6], -0.5f);
        DrawCircleV((Vector2){x, 200.0f}, CELL_SIZE * 0.5f, bomb_color);
    }
    
    float bar_width = 300.0f;
    float bar_height = 30.0f;
    float bar_x = 50.0f;
    float bar_y = 50.0f;
    float fill_width = bar_width * (game->player.coherence.current / 100.0f);
    
    Color bar_color;
    if (game->player.coherence.current > 80.0f) {
        bar_color = GREEN;
    } else if (game->player.coherence.current > 50.0f) {
        bar_color = YELLOW;
    } else {
        bar_color = RED;
    }
    
    DrawRectangle((int)bar_x, (int)bar_y, (int)bar_width, (int)bar_height, 
                 (Color){50, 50, 50, 200});
    DrawRectangle((int)bar_x, (int)bar_y, (int)fill_width, (int)bar_height, 
                 bar_color);
    DrawRectangleLines((int)bar_x, (int)bar_y, (int)bar_width, (int)bar_height, 
                      WHITE);
    
    char coherence_text[50];
    sprintf(coherence_text, "COHERENCE: %d%%", (int)game->player.coherence.current);
    DrawText(coherence_text, (int)bar_x + 10, (int)bar_y + 5, 20, WHITE);
    
    const char* phase_text = (game->player.phase_system.current_phase == PHASE_RED) 
                            ? "PHASE: RED" : "PHASE: BLUE";
    Color phase_color = (game->player.phase_system.current_phase == PHASE_RED)
                       ? (Color){255, 100, 100, 255}
                       : (Color){100, 100, 255, 255};
    DrawText(phase_text, SCREEN_WIDTH - 200, 50, 20, phase_color);
    
    if (game->player.phase_system.state == PHASE_STATE_SUPERPOSITION) {
        char super_text[50];
        sprintf(super_text, "SUPERPOSITION: %d", 
               game->player.phase_system.superposition_turns_left);
        DrawText(super_text, SCREEN_WIDTH - 300, 80, 20, YELLOW);
    }
    
    if (game->player.dead) {
        const char* death_text = "YOU DIED!";
        int text_width = MeasureText(death_text, 68);
        DrawText(death_text, 
                SCREEN_WIDTH / 2 - text_width / 2 + 2, 
                SCREEN_HEIGHT / 2 - 34 + 2, 
                68, 
                PALETTE[2]);
        DrawText(death_text, 
                SCREEN_WIDTH / 2 - text_width / 2, 
                SCREEN_HEIGHT / 2 - 34, 
                68, 
                PALETTE[5]);
    }
}

void cleanup_game(GameState* game) {
    if (game->map) {
        map_free(game->map);
    }
    
    for (int i = 0; i < MAX_EEPERS; i++) {
        if (game->eepers[i].path) {
            path_free(game->eepers[i].path, game->eepers[i].path_rows);
        }
    }
}

int main(void) {
    srand(time(NULL));
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Quantum Eepers v1.0");
    SetTargetFPS(60);
    
    InitAudioDevice();
    
    init_palette();
    
    for (int i = 0; i < 4; i++) {
        footstep_sounds[i] = LoadSound("assets/sounds/footsteps.mp3");
        SetSoundPitch(footstep_sounds[i], 1.7f - i * 0.1f);
    }
    blast_sound = LoadSound("assets/sounds/blast.ogg");
    key_pickup_sound = LoadSound("assets/sounds/key-pickup.wav");
    bomb_pickup_sound = LoadSound("assets/sounds/bomb-pickup.ogg");
    checkpoint_sound = LoadSound("assets/sounds/checkpoint.ogg");
    phase_shift_sound = LoadSound("assets/sounds/checkpoint.ogg");
    ambient_music = LoadMusicStream("assets/sounds/ambient.wav");
    SetMusicVolume(ambient_music, 0.5f);
    
    game_font = GetFontDefault();
    
    GameState game;
    init_game_state(&game, 20, 30);
    load_simple_level(&game);
    
    PlayMusicStream(ambient_music);
    
    while (!WindowShouldClose()) {
        UpdateMusicStream(ambient_music);
        
        if (game.player.dead) {
            if (GetTime() - game.player.death_time > 2.0) {
                init_game_state(&game, 20, 30);
                load_simple_level(&game);
            }
        } else if (!game.player.is_stuck) {
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
                execute_turn(&game, (Command){CMD_STEP, DIR_UP});
            }
            if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
                execute_turn(&game, (Command){CMD_STEP, DIR_DOWN});
            }
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
                execute_turn(&game, (Command){CMD_STEP, DIR_LEFT});
            }
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
                execute_turn(&game, (Command){CMD_STEP, DIR_RIGHT});
            }
            if (IsKeyPressed(KEY_SPACE)) {
                execute_turn(&game, (Command){CMD_PLANT, 0});
            }
            if (IsKeyPressed(KEY_E)) {
                execute_turn(&game, (Command){CMD_PHASE_CHANGE, 0});
            }
        }
        
        if (game.turn_animation > 0.0f) {
            game.turn_animation -= GetFrameTime() / BASE_TURN_DURATION;
            if (game.turn_animation < 0.0f) {
                game.turn_animation = 0.0f;
            }
        }
        
        Vector2 target = vec2_scale(ivec2_to_vec2(game.player.position), CELL_SIZE);
        game.camera.target = Vector2Lerp(game.camera.target, target, GetFrameTime() * 2.0f);
        
        BeginDrawing();
            ClearBackground(PALETTE[0]);
            
            BeginMode2D(game.camera);
                render_game_cells(&game);
                render_items(&game);
                render_quantum_effects(&game);
                render_player(&game);
                render_eepers(&game);
                render_bombs(&game);
            EndMode2D();
            
            render_hud(&game);
            
            DrawFPS(10, 10);
            
        EndDrawing();
    }
    
    cleanup_game(&game);
    
    for (int i = 0; i < 4; i++) {
        UnloadSound(footstep_sounds[i]);
    }
    UnloadSound(blast_sound);
    UnloadSound(key_pickup_sound);
    UnloadSound(bomb_pickup_sound);
    UnloadSound(checkpoint_sound);
    UnloadSound(phase_shift_sound);
    UnloadMusicStream(ambient_music);
    
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}
