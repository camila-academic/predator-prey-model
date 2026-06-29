#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CELL_EMPTY   ' '
#define CELL_WOLF    'W'
#define CELL_SHEEP   'S'
#define CELL_HUNTER  'H'

typedef struct {
    size_t map_width;
    size_t map_height;
} GameConfig;

typedef struct {
    size_t tick;
    size_t wolf_count;
    size_t sheep_count;
    size_t hunter_count;

    size_t total_wolf_kills;
    size_t total_sheep_kills;
    size_t total_hunter_kills;
} WorldStatistics;

typedef struct {
    size_t tick;

    char *map_entity;
    char *map_background;

    size_t map_length_x;
    size_t map_length_y;

    bool is_running;
    uint64_t seed;
} WorldState;

typedef struct {
    size_t x;
    size_t y;
} Pos;

//public API
void game_create_world(GameConfig config);
void game_reset(void);

void game_pause(void);
void game_resume(void);
void game_update(void);
void game_step(void);
void game_run(size_t n_steps);

WorldState game_get_state(void);
WorldStatistics game_get_statistics(void);

bool game_add_cell(size_t pos_x, size_t pos_y, char type);

bool game_save(const char *path);
bool game_load(const char *path);

void game_set_seed(uint64_t seed);

#endif /* SIMULATOR_H */