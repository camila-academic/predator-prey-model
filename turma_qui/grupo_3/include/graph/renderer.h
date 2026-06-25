#ifndef GRAPH_RENDERER_H
#define GRAPH_RENDERER_H

#include <engine/engine.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

void render_game_reset(GameConfig config);
void render_frame(void);

#endif // GRAPH_RENDERER_H
