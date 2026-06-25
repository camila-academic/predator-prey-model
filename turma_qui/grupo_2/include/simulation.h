#ifndef SIMULATION_H
#define SIMULATION_H

#include "types.h"

typedef struct {
    Board board;
    EntityList entities;
    int max_rounds;
    int current_round;
} Simulation;

int simulation_init(Simulation *sim, int size, int max_rounds);
void simulation_free(Simulation *sim);
void simulation_run(Simulation *sim);
Stats simulation_stats(const Simulation *sim);

#endif
