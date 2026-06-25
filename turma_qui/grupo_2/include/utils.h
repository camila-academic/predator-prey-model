#ifndef UTILS_H
#define UTILS_H

#include "types.h"

void seed_random(void);
int random_int(int min, int max);
int chance(int percent);
int positions_equal(Position a, Position b);

#endif
