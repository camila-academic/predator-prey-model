#include "graph/renderer.h"
#include "engine/engine.h"

void render_frame() {
    BeginDrawing();
    ClearBackground(BLUE);

    const WorldState world_state = get_state();
    const WorldStats world_stats = get_statistics();

    const size_t map_width = world_state.map_length_x;
    const size_t map_height = world_state.map_length_y;

    for (size_t y = 0; y < map_height; y++) {
        for (size_t x = 0; x < map_width; x++) {

            char c = world_state.map[y * map_width + x];

            switch (c) {
                case ' ': DrawRectangle(x * 11, y * 11, 10, 10, GREEN); break;
                case 'S': DrawRectangle(x * 11, y * 11, 10, 10, WHITE); break;
                case 'W': DrawRectangle(x * 11, y * 11, 10, 10, GRAY); break;
                case 'H': DrawRectangle(x * 11, y * 11, 10, 10, RED); break;
                default: break;
            }

        }
    }

    EndDrawing();
}
