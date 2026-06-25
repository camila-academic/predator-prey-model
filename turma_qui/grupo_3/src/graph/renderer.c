#include "graph/renderer.h"

#include <raylib.h>
#include <string.h>
#include "engine/engine.h"
#include <math.h>
#include <stdlib.h>

#define SPRITE_SIZE 48
#define SPRITE_OFFSET 16

#define THEME_BACKGROUND (Color){ 40, 42, 54, 255 }
#define THEME_BACKGROUND_2 (Color){ 68, 71, 90, 255 }
#define THEME_FOREGROUND (Color){ 248, 248, 242, 255 }
#define THEME_ACCENT (Color){ 98, 114, 164, 255 }
#define THEME_RED (Color){ 255, 85, 85, 255 }
#define THEME_YELLOW (Color){ 241, 250, 140, 255 }
#define THEME_GREEN (Color){ 80, 250, 123, 255 }
#define THEME_ROUNDNESS 0.3f
#define THEME_FONT_SIZE 24

RenderTexture2D mapTexture = {0};
Texture2D backgroundTexture = {0};
Texture2D sheepTexture = {0};
Texture2D wolfTexture = {0};
Texture2D hunterTexture = {0};
Texture2D treeTexture = {0};
Texture2D rockTexture = {0};

float status_bar_height = 50.0f;
float sym_options_bar_height = 75.0f;
float game_menu_width_percent = 0.25f;

static size_t active_textbox_id = -1;

typedef struct {
    size_t id;
    char *value_buffer;
    size_t buffer_capacity;
} TextBox;

static TextBox world_width_text_box = {0};
static TextBox world_height_text_box = {0};
static TextBox sheep_text_box = {0};
static TextBox wolf_text_box = {0};
static TextBox hunter_text_box = {0};

static bool button(const Rectangle rect, const char *text, const bool disabled) {
    const Vector2 mouse = GetMousePosition();

    const bool hovered = CheckCollisionPointRec(mouse, rect);
    const bool clicked = disabled ? false : hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    DrawRectangleRounded(
        rect, THEME_ROUNDNESS, 5,
        disabled
            ? ColorAlpha(THEME_BACKGROUND, 0.4f)
            : hovered
                  ? ColorBrightness(THEME_BACKGROUND, -0.3f)
                  : THEME_BACKGROUND
    );
    DrawRectangleRoundedLinesEx(
        rect, THEME_ROUNDNESS, 5, 1.0f,
        disabled ? ColorAlpha(THEME_FOREGROUND, 0.4f) : THEME_FOREGROUND
    );

    const int textWidth = MeasureText(text, THEME_FONT_SIZE);

    const Vector2 textPos = {
        rect.x + (rect.width - (float) textWidth) * 0.5f,
        rect.y + (rect.height - THEME_FONT_SIZE) * 0.5f
    };

    DrawText(text, (int) textPos.x, (int) textPos.y, THEME_FONT_SIZE, THEME_FOREGROUND);

    return clicked;
}

static bool textbox(const Rectangle rect, const TextBox textbox) {
    const Vector2 mouse = GetMousePosition();
    const bool hovered = CheckCollisionPointRec(mouse, rect);
    const bool focused = (active_textbox_id == textbox.id);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        active_textbox_id = hovered ? textbox.id : active_textbox_id;

    bool changed = false;
    if (focused) {
        size_t len = strlen(textbox.value_buffer);
        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && len > 0) {
            textbox.value_buffer[len - 1] = '\0';
            changed = true;
        }
        int ch;
        while ((ch = GetCharPressed()) != 0) {
            if (ch >= 32 && len < textbox.buffer_capacity - 1) {
                textbox.value_buffer[len++] = (char) ch;
                textbox.value_buffer[len] = '\0';
                changed = true;
            }
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE))
            active_textbox_id = -1;
    }

    DrawRectangleRounded(
        rect, THEME_ROUNDNESS, 5,
        focused
            ? ColorBrightness(THEME_BACKGROUND, -0.2f)
            : hovered
                  ? ColorBrightness(THEME_BACKGROUND, -0.1f)
                  : THEME_BACKGROUND
    );
    DrawRectangleRoundedLinesEx(
        rect, THEME_ROUNDNESS, 5, focused ? 2.0f : 1.0f,
        focused ? ColorBrightness(THEME_FOREGROUND, 0.3f) : THEME_FOREGROUND
    );

    const int padding = 6;
    const float padding_f = (float) padding;
    const int textWidth = MeasureText(textbox.value_buffer, THEME_FONT_SIZE);
    const int maxTextWidth = (int) rect.width - padding_f * 2;
    const int textY = (int) (rect.y + (rect.height - THEME_FONT_SIZE) * 0.5f);

    const int offsetX = textWidth > maxTextWidth ? textWidth - (int) maxTextWidth : 0;

    BeginScissorMode((int) rect.x + padding, (int) rect.y,
                     (int) rect.width - padding * 2 + 5, (int) rect.height);

    DrawText(textbox.value_buffer, (int) rect.x + padding - offsetX, textY,
             THEME_FONT_SIZE, THEME_FOREGROUND);

    if (focused && ((int) (GetTime() * 2) % 2 == 0)) {
        const int cursorX = (int) rect.x + padding + MIN(textWidth, (int)maxTextWidth) + 2;
        DrawLine(cursorX, textY, cursorX, textY + THEME_FONT_SIZE, THEME_FOREGROUND);
    }

    EndScissorMode();

    return changed;
}


static void render_board(
    const WorldState *world_state,
    const RenderTexture2D texture
) {
    const size_t map_width = (int) world_state->map_length_x;
    const size_t map_height = (int) world_state->map_length_y;

    BeginTextureMode(texture);
    ClearBackground(BLANK);

    char *map = world_state->map;

    for (size_t y = 0; y < map_height; y++) {
        for (size_t x = 0; x < map_width; x++) {
            const char c = map[y * map_width + x];

            switch (c) {
                case ' ': break;
                case 'S':
                    DrawTextureEx(sheepTexture,
                                  (Vector2){x * SPRITE_SIZE - SPRITE_OFFSET, y * SPRITE_SIZE - SPRITE_OFFSET},
                                  0.0f, 1.0f, WHITE);
                    break;

                case 'W':
                    DrawTextureEx(wolfTexture,
                                  (Vector2){x * SPRITE_SIZE - SPRITE_OFFSET, y * SPRITE_SIZE - SPRITE_OFFSET},
                                  0.0f, 1.0f, WHITE);
                    break;

                case 'H':
                    DrawTextureEx(hunterTexture,
                                  (Vector2){x * SPRITE_SIZE - SPRITE_OFFSET, y * SPRITE_SIZE - SPRITE_OFFSET},
                                  0.0f, 1.0f, WHITE);
                    break;

                case 'T':
                    DrawTextureEx(treeTexture,
                                  (Vector2){x * SPRITE_SIZE - SPRITE_OFFSET, y * SPRITE_SIZE - SPRITE_OFFSET},
                                  0.0f, 1.0f, WHITE);
                    break;

                case 'R':
                    DrawTextureEx(rockTexture,
                                  (Vector2){x * SPRITE_SIZE - SPRITE_OFFSET, y * SPRITE_SIZE - SPRITE_OFFSET},
                                  0.0f, 1.0f, WHITE);
                    break;

                default:
                    DrawRectangle(x * SPRITE_SIZE - SPRITE_OFFSET, y * SPRITE_SIZE - SPRITE_OFFSET,
                                  SPRITE_SIZE, SPRITE_SIZE, RED);
                    break;
            }
        }
    }

    EndTextureMode();
}

static void render_status_bar(
    const WorldState *world_state,
    const WorldStatistics *world_stats,
    const Rectangle status_bar_rect
) {
    DrawRectangleRec(status_bar_rect, THEME_BACKGROUND_2);
    DrawLineEx(
        (Vector2){
            status_bar_rect.x,
            status_bar_rect.y + status_bar_rect.height,
        },
        (Vector2){
            status_bar_rect.x + status_bar_rect.width,
            status_bar_rect.y + status_bar_rect.height,
        },
        4.0f,
        THEME_ACCENT
    );
}

static void render_sym_bar(
    const WorldState *world_state,
    const WorldStatistics *world_stats,
    const Rectangle menu_bar_rect
) {
    DrawRectangleRec(menu_bar_rect, THEME_BACKGROUND_2);
    DrawLineEx(
        (Vector2){
            menu_bar_rect.x,
            menu_bar_rect.y,
        },
        (Vector2){
            menu_bar_rect.x + menu_bar_rect.width,
            menu_bar_rect.y,
        },
        4.0f,
        THEME_ACCENT
    );

    const bool simulation_running = world_state->is_running;

    const float button_spacing = menu_bar_rect.width / 4;

    const Rectangle continuePauseButtonRect = {
        (button_spacing * 1) - 75,
        menu_bar_rect.y + 12,
        150.0f,
        50.0f,
    };
    if (simulation_running) {
        if (button(continuePauseButtonRect, "Pause", false)) game_pause();
    } else {
        if (button(continuePauseButtonRect, "Resume", false)) game_resume();
    }

    const Rectangle stepButtonRect = {
        (button_spacing * 2) - 75,
        menu_bar_rect.y + 12,
        150.0f,
        50.0f,
    };
    if (button(stepButtonRect, "Step", simulation_running)) game_run(1);

    const Rectangle resetButtonRect = {
        (button_spacing * 3) - 75,
        menu_bar_rect.y + 12,
        150.0f,
        50.0f,
    };
    if (button(resetButtonRect, "Reset", false)) game_reset();
}

static void render_map_viewport(
    const WorldState *world_state,
    const WorldStatistics *world_stats,
    const Rectangle map_viewport_rect
) {
    DrawRectangleRec(map_viewport_rect, BLACK);
    render_board(world_state, mapTexture);

    // tudo isso aqui pra desenhar o mapa no centro da tela fds

    float scale_x = (float) map_viewport_rect.width / (float) mapTexture.texture.width;
    float scale_y = (float) map_viewport_rect.height / (float) mapTexture.texture.height;

    float scale = fminf(scale_x, scale_y);

    float draw_width = (float) mapTexture.texture.width * scale;
    float draw_height = (float) mapTexture.texture.height * scale;

    float pos_x = map_viewport_rect.x + (map_viewport_rect.width - draw_width) * 0.5f;
    float pos_y = map_viewport_rect.y + (map_viewport_rect.height - draw_height) * 0.5f;

    Rectangle src = {
        0,
        0,
        (float) mapTexture.texture.width,
        (float) -mapTexture.texture.height
    };

    Rectangle dst = {
        pos_x,
        pos_y,
        draw_width,
        draw_height
    };

    DrawTexturePro(
        mapTexture.texture,
        src,
        dst,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}

static void render_map_gen_options(
    const WorldState *world_state,
    const WorldStatistics *world_stats,
    const Rectangle map_gen_options
) {
    DrawRectangleRec(map_gen_options, THEME_BACKGROUND_2);

    const float rect_width = map_gen_options.width;
    const float rect_width_middle = rect_width / 2;

    float y_offset = 10;

    DrawText("World Size:",
             (int) map_gen_options.x + 10,
             (int) (map_gen_options.y + y_offset),
             THEME_FONT_SIZE, THEME_FOREGROUND
    );

    y_offset += 30;

    Rectangle textBoxRect = {
        map_gen_options.x + 10,
        map_gen_options.y + y_offset,
        rect_width_middle - 30,
        50
    };
    textbox(textBoxRect, world_width_text_box);

    DrawText("x",
             (int) textBoxRect.x + (int) textBoxRect.width + 13,
             (int) textBoxRect.y + 11,
             THEME_FONT_SIZE, THEME_FOREGROUND
    );

    textBoxRect = (Rectangle){
        map_gen_options.x + rect_width_middle + 20,
        map_gen_options.y + y_offset,
        rect_width_middle - 30,
        50
    };
    textbox(textBoxRect, world_height_text_box);
}

static void render_ui(
    const WorldState *world_state,
    const WorldStatistics *world_stats
) {
    const float canvas_width = (float) GetScreenWidth();
    const float canvas_height = (float) GetScreenHeight();

    const float game_menu_width = canvas_width * game_menu_width_percent;

    const Rectangle statusBar = {
        0,
        0,
        canvas_width,
        status_bar_height,
    };

    const Rectangle symOptions = {
        0,
        canvas_height - sym_options_bar_height,
        canvas_width,
        100.0f,
    };

    const Rectangle mapViewport = {
        0,
        status_bar_height,
        canvas_width - game_menu_width,
        canvas_height - sym_options_bar_height - status_bar_height,
    };

    const Rectangle worldOptions = {
        canvas_width - game_menu_width,
        statusBar.height,
        game_menu_width,
        (float) GetScreenHeight() - symOptions.height,
    };

    render_map_viewport(world_state, world_stats, mapViewport);

    render_map_gen_options(world_state, world_stats, worldOptions);
    render_status_bar(world_state, world_stats, statusBar);
    render_sym_bar(world_state, world_stats, symOptions);

    //DrawRectangleRec(mapWindow, YELLOW);
    //DrawRectangleRec(symOptions, GREEN);
    //DrawRectangleRec(worldOptions, PINK);

    //DrawRectangleRec(worldOptions_dragBar, WHITE);
}

void render_game_reset(const GameConfig game_config) {
    if (mapTexture.id != 0) {
        UnloadRenderTexture(mapTexture);
        mapTexture = (RenderTexture2D){0};
    }
    if (backgroundTexture.id == 0) backgroundTexture = LoadTexture("assets/background.png");
    if (sheepTexture.id == 0) sheepTexture = LoadTexture("assets/sheep.png");
    if (wolfTexture.id == 0) wolfTexture = LoadTexture("assets/wolf.png");
    if (hunterTexture.id == 0) hunterTexture = LoadTexture("assets/hunter.png");
    if (treeTexture.id == 0) treeTexture = LoadTexture("assets/tree.png");
    if (rockTexture.id == 0) rockTexture = LoadTexture("assets/rock.png");

    if (world_width_text_box.id == 0) {
        world_width_text_box = (TextBox){1, malloc(5), 5};
        memccpy(world_width_text_box.value_buffer, "32", (int) world_width_text_box.buffer_capacity, 3);
    }
    if (world_height_text_box.id == 0) {
        world_height_text_box = (TextBox){2, malloc(5), 5};
        memccpy(world_height_text_box.value_buffer, "32", (int) world_height_text_box.buffer_capacity, 3);
    }

    const size_t texture_width = game_config.map_width * SPRITE_SIZE;
    const size_t texture_height = game_config.map_height * SPRITE_SIZE;

    mapTexture = LoadRenderTexture(
        (int) texture_width,
        (int) texture_height
    );
}

void render_frame() {
    BeginDrawing();
    ClearBackground(THEME_BACKGROUND);

    const WorldState world_state = game_get_state();
    const WorldStatistics world_stats = game_get_statistics();

    render_ui(&world_state, &world_stats);
    EndDrawing();
}

