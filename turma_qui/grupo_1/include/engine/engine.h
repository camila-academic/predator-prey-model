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
    size_t map_length_x;
    size_t map_length_y;
} Config;

typedef struct {
    size_t tick;
    size_t wolf_count;
    size_t sheep_count;
    size_t hunter_count;

    size_t total_wolf_kills;
    size_t total_sheep_kills;
    size_t total_hunter_kills;
} WorldStats;

typedef struct {
    size_t tick;

    char *map;
    size_t map_length_x;
    size_t map_length_y;
} WorldState;

typedef struct {
    size_t x;
    size_t y;
} Pos;

//public API
void create_world(Config config);
void reset(void);

void pause(void);
void resume(void);
void step(void);
void run(size_t n_steps);

WorldState get_state(void);
WorldStats get_statistics(void);

bool add_cell(size_t pos_x, size_t pos_y, char type);

bool save(const char *path);
bool load(const char *path);

void set_seed(uint64_t seed);

#endif /* SIMULATOR_H */