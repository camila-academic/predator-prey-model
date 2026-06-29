#include "engine.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// O arquivo noise.c possui essa função. Declaramos aqui para o linkador encontrar.
float perlin2d(int seed, float x, float y, float freq, int depth);

#define CELL_TREE 'T'
#define CELL_ROCK 'R'

#define REPRODUCTION_RATE 3
#define TERRAIN_WATER 4 // Indice da água definido pelo Leo

static WorldState current_state = {0, NULL, NULL, 0, 0, false, 0};
static WorldStatistics current_stats = {0, 0, 0, 0, 0, 0, 0};

// --- FUNÇÕES AUXILIARES ---

static size_t get_index(size_t x, size_t y) {
    return y * current_state.map_length_x + x;
}

static bool add_cell_internal(size_t pos_x, size_t pos_y, char type) {
    if (pos_x >= current_state.map_length_x || pos_y >= current_state.map_length_y) {
        return false;
    }
    size_t idx = get_index(pos_x, pos_y);
    
    // Regra da Água: Ninguém pode nascer em cima da água
    if (current_state.map_background != NULL && current_state.map_background[idx] == TERRAIN_WATER) {
        return false;
    }

    if (current_state.map_entity[idx] == CELL_EMPTY) {
        current_state.map_entity[idx] = type;
        if (type == CELL_WOLF) current_stats.wolf_count++;
        else if (type == CELL_SHEEP) current_stats.sheep_count++;
        else if (type == CELL_HUNTER) current_stats.hunter_count++;
        return true;
    }
    return false;
}

static void spawn_randomly(size_t count, char type) {
    size_t placed = 0;
    size_t max_attempts = count * 20; // Aumentado para lidar com mapas com muita água
    size_t attempts = 0;

    while (placed < count && attempts < max_attempts) {
        size_t x = rand() % current_state.map_length_x;
        size_t y = rand() % current_state.map_length_y;
        if (add_cell_internal(x, y, type)) placed++;
        attempts++;
    }
}

// Reprodução confinada em área 3x3 ao redor das entidades
static void spawn_local(size_t center_x, size_t center_y, size_t count, char type) {
    size_t placed = 0;
    
    // Varredura do grid 3x3
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue; // Pula o centro onde os pais estão
            
            long nx = (long)center_x + dx;
            long ny = (long)center_y + dy;
            
            // Verifica limites do mapa
            if (nx >= 0 && nx < (long)current_state.map_length_x && 
                ny >= 0 && ny < (long)current_state.map_length_y) {
                
                size_t idx = get_index((size_t)nx, (size_t)ny);
                
                // Checa se está vazio e não é água
                if (current_state.map_entity[idx] == CELL_EMPTY && 
                    current_state.map_background[idx] != TERRAIN_WATER) {
                    
                    current_state.map_entity[idx] = type;
                    if (type == CELL_WOLF) current_stats.wolf_count++;
                    else if (type == CELL_SHEEP) current_stats.sheep_count++;
                    else if (type == CELL_HUNTER) current_stats.hunter_count++;
                    
                    placed++;
                    if (placed >= count) return; // Limite x atingido
                }
            }
        }
    }
}

// --- IMPLEMENTAÇÃO DA API PÚBLICA (engine.h) ---

void game_set_seed(uint64_t seed) {
    current_state.seed = seed;
}

bool game_add_cell(size_t pos_x, size_t pos_y, char type) {
    return add_cell_internal(pos_x, pos_y, type);
}

void game_create_world(GameConfig config) {
    if (current_state.map_entity != NULL) free(current_state.map_entity);
    if (current_state.map_background != NULL) free(current_state.map_background);

    current_state.map_length_x = config.map_width < 5 ? 5 : config.map_width;
    current_state.map_length_y = config.map_height < 5 ? 5 : config.map_height;
    current_state.tick = 0;
    current_state.is_running = false;

    // Gera seed automaticamente se nenhuma foi definida via game_set_seed()
    if (current_state.seed == 0) {
        current_state.seed = (uint64_t)time(NULL);
    }

    // Toda a aleatoriedade da engine parte daqui, usando a seed do WorldState
    srand((unsigned int)current_state.seed);

    size_t total_cells = current_state.map_length_x * current_state.map_length_y;

    current_state.map_entity = (char*)malloc(total_cells * sizeof(char));
    current_state.map_background = (char*)malloc(total_cells * sizeof(char));
    memset(current_state.map_entity, CELL_EMPTY, total_cells);
    memset(&current_stats, 0, sizeof(WorldStatistics));

    // Geração procedural do Background usando perlin2d (0 - 4)
    for (size_t y = 0; y < current_state.map_length_y; y++) {
        for (size_t x = 0; x < current_state.map_length_x; x++) {
            float p = perlin2d((int)current_state.seed, (float)x, (float)y, 0.1f, 4);
            
            // Mapeia a saída do perlin para índices de 0 a 4
            int terrain = (int)(p * 5.0f);
            if (terrain < 0) terrain = 0;
            if (terrain > 4) terrain = 4;
            
            current_state.map_background[get_index(x, y)] = (char)terrain;
        }
    }

    // Proporções exigidas pelo professor
    size_t qtd_fac1 = (size_t)(total_cells * 0.12); // Ovelhas
    size_t qtd_fac2 = (size_t)(total_cells * 0.12); // Lobos
    size_t qtd_fac3 = (size_t)(total_cells * 0.05); // Caçadores
    size_t qtd_obs  = (size_t)(total_cells * 0.10); // Obstáculos

    spawn_randomly(qtd_obs / 2, CELL_TREE);
    spawn_randomly(qtd_obs / 2, CELL_ROCK);
    spawn_randomly(qtd_fac1, CELL_SHEEP);
    spawn_randomly(qtd_fac2, CELL_WOLF);
    spawn_randomly(qtd_fac3, CELL_HUNTER);
}

void game_reset(void) {
    // Preserva a seed atual para recriar exatamente o mesmo mundo
    uint64_t saved_seed = current_state.seed;
    GameConfig config = {current_state.map_length_x, current_state.map_length_y};
    current_state.seed = saved_seed; // garante que game_create_world não gere uma nova
    game_create_world(config);
}

void game_pause(void) { current_state.is_running = false; }
void game_resume(void) { current_state.is_running = true; }
void game_update(void) { game_step(); }

void game_step(void) {
    if (!current_state.is_running && current_state.tick > 0) return;

    current_state.tick++;
    size_t total_cells = current_state.map_length_x * current_state.map_length_y;

    bool *has_moved = (bool*)calloc(total_cells, sizeof(bool));
    if (!has_moved) return;

    for (size_t y = 0; y < current_state.map_length_y; y++) {
        for (size_t x = 0; x < current_state.map_length_x; x++) {
            size_t current_idx = get_index(x, y);
            char entity = current_state.map_entity[current_idx];

            if ((entity == CELL_WOLF || entity == CELL_SHEEP || entity == CELL_HUNTER) && !has_moved[current_idx]) {
                int dir = rand() % 4;
                size_t new_x = x;
                size_t new_y = y;

                if (dir == 0 && y > 0) new_y--;
                else if (dir == 1 && y < current_state.map_length_y - 1) new_y++;
                else if (dir == 2 && x > 0) new_x--;
                else if (dir == 3 && x < current_state.map_length_x - 1) new_x++;

                size_t new_idx = get_index(new_x, new_y);
                char target = current_state.map_entity[new_idx];
                char bg_target = current_state.map_background[new_idx];

                // Regra da Água (Não pode se mover para lá)
                if (bg_target == TERRAIN_WATER) {
                    has_moved[current_idx] = true;
                }
                else if (target == CELL_EMPTY) {
                    current_state.map_entity[new_idx] = entity;
                    current_state.map_entity[current_idx] = CELL_EMPTY;
                    has_moved[new_idx] = true;
                }
                else if (entity == CELL_WOLF && target == CELL_SHEEP) {
                    // Predação Lobo -> Ovelha
                    current_state.map_entity[new_idx] = CELL_WOLF;
                    current_state.map_entity[current_idx] = CELL_EMPTY;
                    has_moved[new_idx] = true;
                    current_stats.total_sheep_kills++;
                    current_stats.sheep_count--;
                }
                else if (entity == CELL_HUNTER && target == CELL_WOLF) {
                    // Predação Caçador -> Lobo
                    current_state.map_entity[new_idx] = CELL_HUNTER;
                    current_state.map_entity[current_idx] = CELL_EMPTY;
                    has_moved[new_idx] = true;
                    current_stats.total_wolf_kills++;
                    current_stats.wolf_count--;
                }
                else if (entity == target) {
                    // Reprodução local x = 3 em área 3x3
                    has_moved[current_idx] = true;
                    spawn_local(x, y, REPRODUCTION_RATE, entity); 
                }
                else {
                    // Bloqueios (Ovelha em Lobo, Entidade em Árvore/Rocha)
                    has_moved[current_idx] = true;
                }
            }
        }
    }
    free(has_moved);
}

void game_run(size_t n_steps) {
    game_resume();
    for (size_t i = 0; i < n_steps; i++) {
        game_step();
    }
}

WorldState game_get_state(void) { return current_state; }
WorldStatistics game_get_statistics(void) { return current_stats; }

bool game_save(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return false;

    fwrite(&current_state.tick, sizeof(size_t), 1, f);
    fwrite(&current_state.map_length_x, sizeof(size_t), 1, f);
    fwrite(&current_state.map_length_y, sizeof(size_t), 1, f);
    fwrite(&current_state.seed, sizeof(uint64_t), 1, f);   // seed do mundo
    fwrite(&current_stats, sizeof(WorldStatistics), 1, f);

    size_t total_cells = current_state.map_length_x * current_state.map_length_y;
    fwrite(current_state.map_entity, sizeof(char), total_cells, f);
    fwrite(current_state.map_background, sizeof(char), total_cells, f);

    fclose(f);
    return true;
}

bool game_load(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    if (current_state.map_entity != NULL) free(current_state.map_entity);
    if (current_state.map_background != NULL) free(current_state.map_background);

    fread(&current_state.tick, sizeof(size_t), 1, f);
    fread(&current_state.map_length_x, sizeof(size_t), 1, f);
    fread(&current_state.map_length_y, sizeof(size_t), 1, f);
    fread(&current_state.seed, sizeof(uint64_t), 1, f);    // seed do mundo
    fread(&current_stats, sizeof(WorldStatistics), 1, f);

    size_t total_cells = current_state.map_length_x * current_state.map_length_y;
    current_state.map_entity = (char*)malloc(total_cells * sizeof(char));
    current_state.map_background = (char*)malloc(total_cells * sizeof(char));
    
    fread(current_state.map_entity, sizeof(char), total_cells, f);
    fread(current_state.map_background, sizeof(char), total_cells, f);
    
    fclose(f);
    return true;
}