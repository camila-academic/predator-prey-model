#include "graph/window.h"

int window_init() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Predator-Prey Ecosystem Simulator");
    if (!IsWindowReady()) return 0;
    
    SetTargetFPS(60);

    return 1;
}
void window_deinit() {
    CloseWindow();
}

int window_closed() {
    return WindowShouldClose();
}
