#include "engine/engine.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Definindo os obstáculos exigidos pelo escopo
#define CELL_TREE 'T'
#define CELL_ROCK 'R'

typedef enum {
    DIRECTION_UP = 0,
    DIRECTION_RIGHT = 1,
    DIRECTION_DOWN = 2,
    DIRECTION_LEFT = 3
} Directions;

// Variáveis de estado global (Estáticas para proteger o encapsulamento da sua engine)
static WorldState current_state = {0, NULL, 0, 0, 0};
static WorldStatistics current_stats = {0, 0, 0, 0, 0, 0, 0};

// --- FUNÇÕES AUXILIARES ---

// Converte coordenadas 2D (x, y) para índice de array 1D
static size_t get_index(size_t x, size_t y) {
    return y * current_state.map_length_x + x;
}

// Tenta adicionar células em posições vazias aleatórias
static void spawn_randomly(size_t count, char type) {
    size_t placed = 0;
    // Prevenção de loop infinito caso o mapa esteja muito cheio
    size_t max_attempts = count * 10; 
    size_t attempts = 0;

    while (placed < count && attempts < max_attempts) {
        size_t x = rand() % current_state.map_length_x;
        size_t y = rand() % current_state.map_length_y;
        
        if (game_add_cell(x, y, type)) {
            placed++;
        }
        attempts++;
    }
}

// --- IMPLEMENTAÇÃO DA API PÚBLICA ---

void game_set_seed(uint64_t seed) {
    srand((unsigned int)seed);
}

bool game_add_cell(size_t pos_x, size_t pos_y, char type) {
    if (pos_x >= current_state.map_length_x || pos_y >= current_state.map_length_y) {
        return false; // Fora dos limites
    }
    
    size_t idx = get_index(pos_x, pos_y);
    if (current_state.map[idx] == CELL_EMPTY) {
        current_state.map[idx] = type;
        
        // Atualiza as estatísticas
        if (type == CELL_WOLF) current_stats.wolf_count++;
        else if (type == CELL_SHEEP) current_stats.sheep_count++;
        else if (type == CELL_HUNTER) current_stats.hunter_count++;
        
        return true;
    }
    return false; // Célula já ocupada
}

void game_create_world(GameConfig config) {
    // 1. Controle de memória: libera o mapa antigo se existir
    if (current_state.map != NULL) {
        free(current_state.map);
    }

    // 2. Validação da regra do HTML (Tamanho mínimo 5x5)
    current_state.map_length_x = config.map_width < 5 ? 5 : config.map_width;
    current_state.map_length_y = config.map_height < 5 ? 5 : config.map_height;
    current_state.tick = 0;
    
    size_t total_cells = current_state.map_length_x * current_state.map_length_y;
    
    // Alocação dinâmica do mapa como vetor 1D
    current_state.map = (char*)malloc(total_cells * sizeof(char));
    memset(current_state.map, CELL_EMPTY, total_cells); 
    memset(&current_stats, 0, sizeof(WorldStatistics));

    // 3. Cálculos de proporção baseados nas regras de negócio
    size_t qtd_fac1 = (size_t)(total_cells * 0.12); // Ovelhas (12%)
    size_t qtd_fac2 = (size_t)(total_cells * 0.12); // Lobos (12%)
    size_t qtd_fac3 = (size_t)(total_cells * 0.05); // Caçadores (5%)
    size_t qtd_obs  = (size_t)(total_cells * 0.10); // Obstáculos (10%)

    // 4. Popula o mundo
    spawn_randomly(qtd_obs / 2, CELL_TREE);
    spawn_randomly(qtd_obs / 2, CELL_ROCK);
    spawn_randomly(qtd_fac1, CELL_SHEEP);
    spawn_randomly(qtd_fac2, CELL_WOLF);
    spawn_randomly(qtd_fac3, CELL_HUNTER);
}

void game_reset(void) {
    GameConfig config = {current_state.map_length_x, current_state.map_length_y};
    game_create_world(config);
}

void game_pause(void) { current_state.is_running = false; }
void game_resume(void) { current_state.is_running = true; }

void game_update(void) {
    const int extinct = (current_stats.hunter_count == 0 ? 1 : 0)
        + (current_stats.wolf_count == 0 ? 1 : 0)
        + (current_stats.sheep_count == 0 ? 1 : 0);
    if (extinct >= 2) return;

    if (!current_state.is_running && current_state.tick > 0) return;
    game_step();
}

void game_step(void) {
    current_state.tick++;

    size_t total_cells = current_state.map_length_x * current_state.map_length_y;

    // Buffer dinâmico para registrar quem já andou neste turno
    bool *has_moved = (bool*)calloc(total_cells, sizeof(bool));
    if (!has_moved) return;

    // Loop de varredura bidimensional
    for (size_t y = 0; y < current_state.map_length_y; y++) {
        for (size_t x = 0; x < current_state.map_length_x; x++) {
            size_t current_idx = get_index(x, y);
            char entity = current_state.map[current_idx];

            if ((entity == CELL_WOLF || entity == CELL_SHEEP || entity == CELL_HUNTER) && !has_moved[current_idx]) {

                const int dirs[4] = {DIRECTION_UP, DIRECTION_RIGHT, DIRECTION_DOWN, DIRECTION_LEFT};
                const int dir = dirs[rand() % 4];

                size_t new_x = x;
                size_t new_y = y;

                const size_t w = current_state.map_length_x;
                const size_t h = current_state.map_length_y;

                switch (dir) {
                    case DIRECTION_UP:    if (y > 0) new_y--; break;
                    case DIRECTION_DOWN:  if (y < h-1) new_y++; break;
                    case DIRECTION_LEFT:  if (x > 0) new_x--; break;
                    case DIRECTION_RIGHT: if (x < w-1) new_x++; break;
                }

                size_t new_idx = get_index(new_x, new_y);
                char target = current_state.map[new_idx];

                if (target == CELL_EMPTY) {
                    has_moved[current_idx] = true;
                    has_moved[new_idx] = true;

                    current_state.map[new_idx] = entity;
                    current_state.map[current_idx] = CELL_EMPTY;
                }

                // Lobo sobrepõe Ovelha
                else if (
                    (entity == CELL_WOLF && target == CELL_SHEEP) ||
                    (entity == CELL_SHEEP && target == CELL_WOLF)
                ) {
                    has_moved[new_idx] = true;
                    has_moved[current_idx] = true;

                    current_state.map[new_idx] = CELL_WOLF;
                    //current_state.map[current_idx] = CELL_EMPTY;

                    current_stats.total_sheep_kills++;
                    current_stats.sheep_count--;
                    current_stats.wolf_count++;
                }

                // Caçador sobrepõe Lobo
                else if (
                    (entity == CELL_HUNTER && target == CELL_WOLF) ||
                    (entity == CELL_WOLF && target == CELL_HUNTER)
                ) {
                    has_moved[new_idx] = true;
                    has_moved[current_idx] = true;

                    current_state.map[new_idx] = CELL_HUNTER;
                    //current_state.map[current_idx] = CELL_EMPTY;

                    current_stats.total_wolf_kills++;
                    current_stats.wolf_count--;
                    current_stats.hunter_count++;
                }

                // Ovelha sobrepõe caçador
                else if (
                    (entity == CELL_SHEEP && target == CELL_HUNTER) ||
                    (entity == CELL_HUNTER && target == CELL_SHEEP)
                ) {
                    has_moved[new_idx] = true;
                    has_moved[current_idx] = true;

                    current_state.map[new_idx] = CELL_SHEEP;
                    //current_state.map[current_idx] = CELL_EMPTY;

                    current_stats.total_hunter_kills++;
                    current_stats.hunter_count--;
                    current_stats.sheep_count++;
                }

                // Condição de Bloqueio: Bateu numa árvore, pedra ou aliado.
                else {
                    has_moved[current_idx] = true;
                }
            }
        }
    }

    // Limpeza de memória obrigatória.
    free(has_moved);
}

void game_run(size_t n_steps) {
    for (size_t i = 0; i < n_steps; i++) game_step();
}

WorldState game_get_state(void) { return current_state; }
WorldStatistics game_get_statistics(void) { return current_stats; }

// Implementação de manipulação de arquivos (Binários)
bool game_save(const char *path) {
    FILE *f = fopen(path, "wb"); 
    if (!f) return false;
    
    // Salva configurações atuais e estatísticas
    fwrite(&current_state.tick, sizeof(size_t), 1, f);
    fwrite(&current_state.map_length_x, sizeof(size_t), 1, f);
    fwrite(&current_state.map_length_y, sizeof(size_t), 1, f);
    fwrite(&current_stats, sizeof(WorldStatistics), 1, f);
    
    // Salva todo o bloco de memória do mapa
    size_t total_cells = current_state.map_length_x * current_state.map_length_y;
    fwrite(current_state.map, sizeof(char), total_cells, f);
    
    fclose(f);
    return true;
}

// Carregamento do arquivo binário
bool game_load(const char *path) {
    FILE *f = fopen(path, "rb"); 
    if (!f) return false;

    // Segurança: limpa o mapa atual antes de sobrescrever
    if (current_state.map != NULL) {
        free(current_state.map);
    }
    
    // Recupera dados básicos
    fread(&current_state.tick, sizeof(size_t), 1, f);
    fread(&current_state.map_length_x, sizeof(size_t), 1, f);
    fread(&current_state.map_length_y, sizeof(size_t), 1, f);
    fread(&current_stats, sizeof(WorldStatistics), 1, f);
    
    // Realoca a memória para o novo tamanho e joga os dados salvos nela
    size_t total_cells = current_state.map_length_x * current_state.map_length_y;
    current_state.map = (char*)malloc(total_cells * sizeof(char));
    fread(current_state.map, sizeof(char), total_cells, f);
    
    fclose(f);
    return true;
}