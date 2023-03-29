#include <graphics.h>
#include <raylib.h>
void draw_sprite_ray(chip_8 *c) {
    BeginDrawing();
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            if (c->display[x][y]) {
                DrawRectangle(x * GFX_SCALE, y * GFX_SCALE + GUI_HEIGHT, 10, 10, BLACK);
            }
        }
    }
    EndDrawing();
}