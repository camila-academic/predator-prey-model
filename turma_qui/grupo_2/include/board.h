#ifndef BOARD_H
#define BOARD_H

#include "types.h"

int board_init(Board *board, int size);
void board_free(Board *board);
int board_index(const Board *board, Position pos);
int board_in_bounds(const Board *board, Position pos);
int board_has_obstacle(const Board *board, Position pos);
void board_set_obstacle(Board *board, Position pos, ObstacleType type);
ObstacleType board_get_obstacle(const Board *board, Position pos);
Position board_random_position(const Board *board);
void board_print(const Board *board, const EntityList *entities, int round);

#endif
