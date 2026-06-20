#include "../../include/engine/engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

typedef struct {
    Config cfg;
    char *grid;
    WorldStats stats;

    size_t tick;
    bool paused;

    uint64_t rng_state;
} World;

static World *world = NULL;
static WorldState state;
static WorldStats stats;

// Direções (cima, baixo, esquerda, direita
static const int DX[8] = { 0,  0,  1, -1, };
static const int DY[8] = {-1,  1,  0,  0, };

// randomizer
static uint64_t rng_next(void) {
    uint64_t x = world->rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    world->rng_state = x;
    return x;
}

static int rng_int(const int lo, const int hi) {
    return lo + (int)(rng_next() % (uint64_t)(hi - lo + 1));
}

static bool in_bounds(const size_t x, const size_t y) {
    return x >= 0 && y >= 0
        && (size_t)x < world->cfg.map_length_x
        && (size_t)y < world->cfg.map_length_y;
}

static int find_neighbours(const size_t cx, const size_t cy, char type, Pos *out) {
    int n = 0;

    const size_t width = world->cfg.map_length_x;
    const size_t height = world->cfg.map_length_y;

    for (int d = 0; d < 8; d++) {
        const size_t nx = cx + DX[d];
        const size_t ny = cy + DY[d];
        if (!in_bounds(nx, ny)) continue;

        const char c = world->grid[ny * height + width];
        if (type == CELL_EMPTY) {
            if (c == CELL_EMPTY) out[n++] = (Pos){nx, ny};
        } else if (c == type) {
            out[n++] = (Pos){nx, ny};
        }
    }
    return n;
}

static void shuffle_pos(Pos *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        const int j = rng_int(0, i);
        const Pos tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}


static char *alloc_grid(const size_t width, const size_t height) {
    char *grid = calloc(width * height, 1);
    return grid;
}

static void free_grid(char *g) {
    if (g) free(g);
}


static void calculate_stats(void) {
    size_t wolves = 0;
    size_t sheep = 0;
    size_t hunters = 0;

    const size_t width = world->cfg.map_length_x;
    const size_t height = world->cfg.map_length_y;

    for (size_t y = 0; y < height; y++)
        for (size_t x = 0; x < width; x++) {
            switch (world->grid[y * width + x]) {
                case CELL_WOLF:   wolves++;  break;
                case CELL_SHEEP:  sheep++;   break;
                case CELL_HUNTER: hunters++; break;
                default: break;
            }
        }
    world->stats.tick = world->tick;
    world->stats.wolf_count = wolves;
    world->stats.sheep_count = sheep;
    world->stats.hunter_count = hunters;
}

static void tick_cell(const size_t x, const size_t y) {

}

void create_world(const Config config) {
    if (world) reset();
    printf("Creating %lu x %lu world", config.map_length_x, config.map_length_y);

    world = calloc(1, sizeof(World));
    world->cfg    = config;
    world->paused = false;
    world->tick   = 0;
    world->rng_state = (uint64_t)time(NULL) ^ 0xDEADBEEFCAFE1234ULL;
    if (world->rng_state == 0) world->rng_state = 1;

    const size_t width = config.map_length_x;
    const size_t height  = config.map_length_y;

    world->grid = alloc_grid(width, height);

    for (size_t y = 0; y < height; y++)
        for (size_t x = 0; x < width; x++)
            world->grid[y * width + x] = CELL_EMPTY;

    world->stats.tick = 0;
    world->stats.wolf_count = 0;
    world->stats.sheep_count = 0;
    world->stats.hunter_count = 0;
    world->stats.total_wolf_kills = 0;
    world->stats.total_sheep_kills = 0;
    world->stats.total_hunter_kills = 0;
}

void reset(void) {
    if (!world) return;
    size_t width = world->cfg.map_length_x;
    size_t height = world->cfg.map_length_y;

    printf("Resetting %lu x %lu world", width, height);

    free_grid(world->grid);
    free(world);
    world = NULL;
}

void pause(void)  { if (world) world->paused = true;  }
void resume(void) { if (world) world->paused = false; }

void step(void) {
    if (!world) return;

    const size_t width = world->cfg.map_length_x;
    const size_t height = world->cfg.map_length_y;

    for (size_t y = 0; y < height; y++)
        for (size_t x = 0; x < width; x++)
            tick_cell(x, y);

    world->tick++;
    calculate_stats();
}

void run(const size_t n_steps) {
    if (!world) return;
    for (size_t i = 0; i < n_steps; i++) {
        if (world->paused) break;
        step();
    }
}

WorldState get_state(void) {
    if (!world) return state;

    state.tick = world->tick;
    state.map_length_x = world->cfg.map_length_x;
    state.map_length_y = world->cfg.map_length_y;
    state.map = world->grid;

    return state;
}
WorldStats get_statistics(void) { return stats; }

bool add_cell(const size_t pos_x, const size_t pos_y, char type) {
    if (!world) return false;

    const size_t width = world->cfg.map_length_x;
    const size_t height = world->cfg.map_length_y;

    if (pos_x >= world->cfg.map_length_x || pos_y >= world->cfg.map_length_y)
        return false;

    world->grid[pos_y * width + pos_x] = type;
    return true;
}

bool save(const char *path) {
    // if (!world) return false;
    // FILE *f = fopen(path, "w");
    // if (!f) return false;
    //
    // const size_t width = world->cfg.map_length_x;
    // const size_t height = world->cfg.map_length_y;
    //
    // fprintf(f, "VERSION 1\n");
    // fprintf(f, "SIZE %zu %zu\n", Wx, H);
    // fprintf(f, "TICK %zu\n", world->tick);
    // fprintf(f, "SEED %llu\n", (unsigned long long)world->rng_state);
    // fprintf(f, "STATS");
    // for (int i = 0; i < STAT_COUNT; i++) fprintf(f, " %zu", world->stats[i]);
    // fprintf(f, "\n");
    // fprintf(f, "MAP\n");
    // for (usize_t y = 0; y < H; y++) {
    //     for (usize_t x = 0; x < Wx; x++)
    //         fputc(world->grid[y][x].type, f);
    //     fputc('\n', f);
    // }
    // fprintf(f, "META\n");
    // for (usize_t y = 0; y < H; y++)
    //     for (usize_t x = 0; x < Wx; x++) {
    //         Cell *c = &world->grid[y][x];
    //         fprintf(f, "%zu %zu %d %d %d %d\n",
    //             x, y, c->age, c->hunger, c->breed_timer,
    //             world->grass_timer[y][x]);
    //     }
    // fclose(f);
    // return true;
}

bool load(const char *path) {
    // FILE *f = fopen(path, "r");
    // if (!f) return false;
    //
    // char line[256];
    // size_t Wx = 0, H = 0, tick = 0;
    // uint64_t seed = 1;
    //
    // while (fgets(line, sizeof(line), f)) {
    //     if (strncmp(line, "SIZE", 4) == 0) {
    //         sscanf(line, "SIZE %zu %zu", &Wx, &H);
    //     }
    //     else if (strncmp(line, "TICK", 4) == 0) {
    //         sscanf(line, "TICK %zu", &tick);
    //     }
    //     else if (strncmp(line, "SEED", 4) == 0) {
    //         unsigned long long s;
    //         sscanf(line, "SEED %llu", &s);
    //         seed = (uint64_t)s;
    //     }
    //     else if (strncmp(line, "STATS", 5) == 0) {
    //         sscanf(line, "STATS %zu %zu %zu %zu %zu %zu %zu",
    //         );
    //     }
    //     else if (strncmp(line, "MAP", 3) == 0) {
    //         break;
    //     }
    // }
    //
    // if (Wx == 0 || H == 0) { fclose(f); return false; }
    //
    // Config cfg = { Wx, H };
    // create_world(cfg);
    // world->tick = tick;
    // world->rng_state = seed;
    // memcpy(world->stats, stats, sizeof(stats));
    //
    // for (usize_t y = 0; y < H; y++) {
    //     if (!fgets(line, sizeof(line), f)) break;
    //     for (usize_t x = 0; x < Wx && line[x] && line[x] != '\n'; x++) {
    //         char t = line[x];
    //         world->grid[y][x].type = (t == CELL_WOLF || t == CELL_SHEEP ||
    //                                t == CELL_HUNTER || t == CELL_GRASS)
    //                               ? t : CELL_EMPTY;
    //     }
    // }
    //
    // /* skip META header */
    // while (fgets(line, sizeof(line), f))
    //     if (strncmp(line, "META", 4) == 0) break;
    //
    // usize_t lx, ly;
    // int age, hunger, breed, gtimer;
    // while (fscanf(f, "%zu %zu %d %d %d %d\n",
    //               &lx, &ly, &age, &hunger, &breed, &gtimer) == 6) {
    //     if (lx < Wx && ly < H) {
    //         world->grid[ly][lx].age         = age;
    //         world->grid[ly][lx].hunger      = hunger;
    //         world->grid[ly][lx].breed_timer = breed;
    //         world->grass_timer[ly][lx]      = gtimer;
    //     }
    // }
    //
    // fclose(f);
    // recalc_stats();
    // return true;
}

void set_seed(const uint64_t seed) {
    if (!world) return;
    world->rng_state = (seed == 0) ? (uint64_t)time(NULL) : seed;
    if (world->rng_state == 0) world->rng_state = 1;
}
