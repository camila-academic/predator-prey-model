#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_SIZE 5
#define MAX_SIZE 40
#define HUMAN_PERCENT 12
#define ZOMBIE_PERCENT 12
#define SOLDIER_PERCENT 5
#define OBSTACLE_PERCENT 10
#define REPRODUCTION_CHANCE 18
#define ZOMBIE_START_ENERGY 8
#define ZOMBIE_FEED_ENERGY 4

typedef enum {
    EMPTY,
    HUMAN,
    ZOMBIE,
    SOLDIER,
    TREE,
    ROCK
} CellType;

typedef struct {
    int row;
    int col;
} Position;

typedef struct {
    int id;
    CellType type;
    Position pos;
    int alive;
    int age;
    int energy;
} Entity;

typedef struct {
    Entity *items;
    int count;
    int capacity;
    int next_id;
} EntityList;

typedef struct {
    int size;
    CellType **cells;
    EntityList entities;
    int round;
    int game_over;
} Game;

int random_int(int min, int max) {
    return min + rand() % (max - min + 1);
}

int chance(int percent) {
    return random_int(1, 100) <= percent;
}

int percent_of_cells(int size, int percent) {
    int total = size * size;
    int amount = (total * percent) / 100;
    return amount < 1 ? 1 : amount;
}

char symbol(CellType type) {
    switch (type) {
        case HUMAN:
            return 'H';
        case ZOMBIE:
            return 'Z';
        case SOLDIER:
            return 'S';
        case TREE:
            return 'A';
        case ROCK:
            return 'P';
        default:
            return '.';
    }
}

const char *name_of(CellType type) {
    switch (type) {
        case HUMAN:
            return "humano";
        case ZOMBIE:
            return "zumbi";
        case SOLDIER:
            return "soldado";
        case TREE:
            return "arvore";
        case ROCK:
            return "pedra";
        default:
            return "vazio";
    }
}

void init_entity_list(EntityList *list) {
    list->count = 0;
    list->capacity = 32;
    list->next_id = 1;
    list->items = malloc(sizeof(Entity) * list->capacity);

    if (list->items == NULL) {
        printf("Erro ao alocar memoria para entidades.\n");
        exit(1);
    }
}

void free_entity_list(EntityList *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

Entity *add_entity(EntityList *list, CellType type, int row, int col) {
    if (list->count == list->capacity) {
        int new_capacity = list->capacity * 2;
        Entity *new_items = realloc(list->items, sizeof(Entity) * new_capacity);

        if (new_items == NULL) {
            printf("Erro ao expandir memoria das entidades.\n");
            exit(1);
        }

        list->items = new_items;
        list->capacity = new_capacity;
    }

    Entity *entity = &list->items[list->count++];
    entity->id = list->next_id++;
    entity->type = type;
    entity->pos.row = row;
    entity->pos.col = col;
    entity->alive = 1;
    entity->age = 0;
    entity->energy = type == ZOMBIE ? ZOMBIE_START_ENERGY : 10;

    return entity;
}

void remove_dead_entities(EntityList *list) {
    int write = 0;

    for (int read = 0; read < list->count; read++) {
        if (list->items[read].alive) {
            list->items[write++] = list->items[read];
        }
    }

    list->count = write;
}

CellType **create_board(int size) {
    CellType **board = malloc(sizeof(CellType *) * size);

    if (board == NULL) {
        printf("Erro ao alocar tabuleiro.\n");
        exit(1);
    }

    for (int row = 0; row < size; row++) {
        board[row] = malloc(sizeof(CellType) * size);

        if (board[row] == NULL) {
            printf("Erro ao alocar linha do tabuleiro.\n");
            exit(1);
        }

        for (int col = 0; col < size; col++) {
            board[row][col] = EMPTY;
        }
    }

    return board;
}

void free_board(CellType **board, int size) {
    for (int row = 0; row < size; row++) {
        free(board[row]);
    }

    free(board);
}

int in_bounds(Game *game, int row, int col) {
    return row >= 0 && row < game->size && col >= 0 && col < game->size;
}

int is_obstacle(CellType type) {
    return type == TREE || type == ROCK;
}

Entity *entity_at(Game *game, int row, int col) {
    for (int i = 0; i < game->entities.count; i++) {
        Entity *entity = &game->entities.items[i];

        if (entity->alive && entity->pos.row == row && entity->pos.col == col) {
            return entity;
        }
    }

    return NULL;
}

int cell_is_free(Game *game, int row, int col) {
    return in_bounds(game, row, col)
        && game->cells[row][col] == EMPTY
        && entity_at(game, row, col) == NULL;
}

Position random_free_position(Game *game) {
    Position pos;

    do {
        pos.row = random_int(0, game->size - 1);
        pos.col = random_int(0, game->size - 1);
    } while (!cell_is_free(game, pos.row, pos.col));

    return pos;
}

void place_obstacles(Game *game, int amount) {
    for (int i = 0; i < amount; i++) {
        Position pos = random_free_position(game);
        game->cells[pos.row][pos.col] = chance(50) ? TREE : ROCK;
    }
}

void place_entities(Game *game, CellType type, int amount) {
    for (int i = 0; i < amount; i++) {
        Position pos = random_free_position(game);
        add_entity(&game->entities, type, pos.row, pos.col);
    }
}

void init_game(Game *game, int size) {
    game->size = size;
    game->cells = create_board(size);
    game->round = 0;
    game->game_over = 0;

    init_entity_list(&game->entities);

    place_obstacles(game, percent_of_cells(size, OBSTACLE_PERCENT));
    place_entities(game, HUMAN, percent_of_cells(size, HUMAN_PERCENT));
    place_entities(game, ZOMBIE, percent_of_cells(size, ZOMBIE_PERCENT));
    place_entities(game, SOLDIER, percent_of_cells(size, SOLDIER_PERCENT));
}

void free_game(Game *game) {
    free_board(game->cells, game->size);
    free_entity_list(&game->entities);
}

int count_entities(Game *game, CellType type) {
    int total = 0;

    for (int i = 0; i < game->entities.count; i++) {
        if (game->entities.items[i].alive && game->entities.items[i].type == type) {
            total++;
        }
    }

    return total;
}

int count_obstacles(Game *game) {
    int total = 0;

    for (int row = 0; row < game->size; row++) {
        for (int col = 0; col < game->size; col++) {
            if (is_obstacle(game->cells[row][col])) {
                total++;
            }
        }
    }

    return total;
}

CellType visible_cell(Game *game, int row, int col) {
    Entity *entity = entity_at(game, row, col);

    if (entity != NULL) {
        return entity->type;
    }

    return game->cells[row][col];
}

void print_board(Game *game) {
    printf("\n=== Rodada %d ===\n", game->round);
    printf("Legenda: H=humano | Z=zumbi | S=soldado | A=arvore | P=pedra | .=vazio\n\n");

    printf("   ");
    for (int col = 0; col < game->size; col++) {
        printf("%2d", col);
    }
    printf("\n");

    for (int row = 0; row < game->size; row++) {
        printf("%2d ", row);

        for (int col = 0; col < game->size; col++) {
            printf(" %c", symbol(visible_cell(game, row, col)));
        }

        printf("\n");
    }
}

void print_stats(Game *game) {
    printf("\nHumanos: %d | Zumbis: %d | Soldados: %d | Obstaculos: %d\n",
        count_entities(game, HUMAN),
        count_entities(game, ZOMBIE),
        count_entities(game, SOLDIER),
        count_obstacles(game));
}

int can_eliminate(CellType attacker, CellType target) {
    return (attacker == ZOMBIE && target == HUMAN)
        || (attacker == SOLDIER && target == ZOMBIE);
}

Position random_neighbor(Position pos) {
    int direction = random_int(0, 3);

    if (direction == 0) {
        pos.row--;
    } else if (direction == 1) {
        pos.row++;
    } else if (direction == 2) {
        pos.col--;
    } else {
        pos.col++;
    }

    return pos;
}

int find_empty_neighbor(Game *game, Position origin, Position *result) {
    Position options[4] = {
        { origin.row - 1, origin.col },
        { origin.row + 1, origin.col },
        { origin.row, origin.col - 1 },
        { origin.row, origin.col + 1 }
    };

    for (int i = 0; i < 4; i++) {
        int index = random_int(0, 3);

        if (cell_is_free(game, options[index].row, options[index].col)) {
            *result = options[index];
            return 1;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (cell_is_free(game, options[i].row, options[i].col)) {
            *result = options[i];
            return 1;
        }
    }

    return 0;
}

void try_reproduce(Game *game, Entity *entity, Entity *other) {
    Position child_pos;

    if (entity->type != other->type) {
        return;
    }

    if (entity->age < 2 || other->age < 2 || !chance(REPRODUCTION_CHANCE)) {
        return;
    }

    if (find_empty_neighbor(game, entity->pos, &child_pos)) {
        add_entity(&game->entities, entity->type, child_pos.row, child_pos.col);
        printf("Evento: nasceu um novo %s.\n", name_of(entity->type));
    }
}

void move_entity(Game *game, int index) {
    Entity *entity = &game->entities.items[index];

    if (!entity->alive) {
        return;
    }

    entity->age++;

    if (entity->type == ZOMBIE) {
        entity->energy--;

        if (entity->energy <= 0) {
            entity->alive = 0;
            printf("Evento: um zumbi ficou sem energia.\n");
            return;
        }
    }

    Position target_pos = random_neighbor(entity->pos);

    if (!in_bounds(game, target_pos.row, target_pos.col)) {
        return;
    }

    if (is_obstacle(game->cells[target_pos.row][target_pos.col])) {
        return;
    }

    Entity *target = entity_at(game, target_pos.row, target_pos.col);

    if (target == NULL) {
        entity->pos = target_pos;
        return;
    }

    if (can_eliminate(entity->type, target->type)) {
        printf("Evento: %s eliminou %s.\n", name_of(entity->type), name_of(target->type));
        target->alive = 0;
        entity->pos = target_pos;

        if (entity->type == ZOMBIE) {
            entity->energy += ZOMBIE_FEED_ENERGY;
        }

        return;
    }

    try_reproduce(game, entity, target);
}

void run_round(Game *game) {
    int initial_count = game->entities.count;
    game->round++;

    for (int i = 0; i < initial_count; i++) {
        move_entity(game, i);
    }

    remove_dead_entities(&game->entities);

    if (count_entities(game, HUMAN) == 0) {
        printf("\nFim de jogo: os zumbis dominaram o mapa.\n");
        game->game_over = 1;
    } else if (count_entities(game, ZOMBIE) == 0) {
        printf("\nFim de jogo: os soldados controlaram a infestacao.\n");
        game->game_over = 1;
    }
}

int read_int(const char *message, int min, int max) {
    int value;

    do {
        printf("%s", message);

        if (scanf("%d", &value) != 1) {
            while (getchar() != '\n') {
            }
            value = min - 1;
        }

        if (value < min || value > max) {
            printf("Digite um valor entre %d e %d.\n", min, max);
        }
    } while (value < min || value > max);

    return value;
}

void wait_enter(void) {
    printf("\nPressione ENTER para continuar...");
    while (getchar() != '\n') {
    }
    getchar();
}

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void menu(Game *game) {
    int option;

    do {
        print_board(game);
        print_stats(game);

        printf("\n1 - Executar uma rodada\n");
        printf("2 - Executar 10 rodadas\n");
        printf("3 - Executar ate o fim\n");
        printf("4 - Sair\n");
        printf("Escolha: ");

        if (scanf("%d", &option) != 1) {
            while (getchar() != '\n') {
            }
            option = 0;
        }

        if (option == 1) {
            clear_screen();
            run_round(game);
        } else if (option == 2) {
            clear_screen();
            for (int i = 0; i < 10 && !game->game_over; i++) {
                run_round(game);
            }
        } else if (option == 3) {
            clear_screen();
            while (!game->game_over) {
                run_round(game);
            }
        } else if (option != 4) {
            printf("Opcao invalida.\n");
        }

    } while (option != 4 && !game->game_over);

    print_board(game);
    print_stats(game);
}

int main(void) {
    Game game;

    srand((unsigned int)time(NULL));

    printf("=====================================\n");
    printf("      Ecossistema de Zumbis em C      \n");
    printf("=====================================\n\n");

    int size = read_int("Tamanho do mapa (5 a 40): ", MIN_SIZE, MAX_SIZE);

    init_game(&game, size);
    clear_screen();
    menu(&game);
    free_game(&game);

    printf("\nJogo encerrado.\n");

    return 0;
}
