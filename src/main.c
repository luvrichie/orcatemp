#include <stdio.h>
#include <main.h>
#include <opcodes.h>
#include <stdbool.h>
#include <graphics.h>

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_CUSTOM_ICONS
#include <iconset.h>

#define foreach(item, array) \
    for(int keep = 1, \
            count = 0,\
            size = sizeof (array) / sizeof *(array); \
        keep && count != size; \
        keep = !keep, count++) \
      for(item = (array) + count; keep; keep = !keep)

#include <raygui.h>
#include <raylib.h>

KeyboardKey keyMapping[16] = {KEY_X, KEY_ONE, KEY_TWO, KEY_THREE, KEY_Q, KEY_W, KEY_E, KEY_A, KEY_S, KEY_D, KEY_Z,
                              KEY_C, KEY_FOUR, KEY_R, KEY_F, KEY_V};

void init(chip_8 *c) {
    memset(c->memory, 0, MEMORY_SIZE);
    memset(c->display, 0, SCREEN_HEIGHT * SCREEN_WIDTH);
    memset(c->stack, 0, STACK_SIZE);
    memset(c->V, 0, 16);
    memset(c->keycur, 0, 16);
    memset(c->keyold, 0, 16);

    memcpy(c->memory + FONT_ADDR, fontset, 80);
    printf("[*] loaded font-set into memory\n");

    c->pc = PRG_ADDR;
    c->I = 0;
    c->sp = 0;

    c->delay_timer = 0;
    c->sound_timer = 0;

    c->running = false;

    printf("[*] init finished!\n");
}

void step(chip_8 *c) {
    uint8_t hi = c->memory[c->pc];
    uint8_t lo = c->memory[c->pc + 1];
    uint16_t full = (hi << 8) | lo;
    c->pc += 2;
    printf("0x%04x\n", full);
    switch (hi >> 4) {
        case 0x0:
            if (0x00E0 == full) {
                clear_display(c);
            } else if (full == 0x00EE) {
                printf("RETURN!");
                return_subroutine(c);
            } else {
                fprintf(stderr, "[!] unimplemented opcode! 0x%04x\n", full);
            }
            break;
        case 0x1:
            jmp_addr(c, full & 0xfff);
            break;
        case 0x2:
            call_subroutine(c, full & 0xfff);
            break;
        case 0x3:
            skip_equal(c, hi & 0xf, lo);
            break;
        case 0x4:
            skip_not_equal(c, hi & 0xf, lo);
            break;
        case 0x5:
            skip_equal_reg(c, hi & 0xf, lo >> 4);
            break;
        case 0x6:
            set_reg(c, hi & 0xf, lo);
            break;
        case 0x7:
            add_reg(c, hi & 0xf, lo);
            break;
        case 0x8:
            switch (lo & 0xf) {
                case 0x0:
                    set_reg_to_reg(c, hi & 0xf, lo >> 4);
                    break;
                case 0x1:
                    bit_or_reg(c, hi & 0xf, lo >> 4);
                    break;
                case 0x2:
                    bit_and_reg(c, hi & 0xf, lo >> 4);
                    break;
                case 0x3:
                    bit_xor_reg(c, hi & 0xf, lo >> 4);
                    break;
                case 0x4:
                    add_reg_to_reg(c, hi & 0xf, lo >> 4);
                    break;
                case 0x5:
                    sub_reg(c, hi & 0xf, lo >> 4);
                    break;
                case 0x6:
                    shift_reg_right(c, hi & 0xf, lo >> 4);
                    break;
                case 0x7:
                    sub_reg_rev(c, hi & 0xf, lo >> 4);
                    break;
                case 0xe:
                    shift_reg_left(c, hi & 0xf, lo >> 4);
                    break;
                default:
                    fprintf(stderr, "[!] unimplemented opcode! 0x%04x\n", full);
            }
            break;
        case 0x9:
            skip_not_equal_reg(c, hi & 0xf, lo >> 4);
            break;
        case 0xa:
            set_index_reg(c, full & 0xfff);
            break;
        case 0xb:
            jmp_addr_V0(c, full & 0xfff);
            break;
        case 0xc:
            num_gen(c, hi & 0xf, lo);
            break;
        case 0xd:
            draw_sprite(c, hi & 0xf, lo >> 4, lo & 0xf);
            break;
        case 0xe:
            switch (lo) {
                case 0x9e:
                    skip_if_key_pressed(c, hi & 0xf);
                    break;
                case 0xa1:
                    skip_if_key_not_pressed(c, hi & 0xf);
                    break;
                default:
                    fprintf(stderr, "[!] unimplemented opcode! 0x%04x\n", full);
            }
            break;
        case 0xf:
            switch (lo) {
                case 0x07:
                    set_reg_timer(c, hi & 0xf);
                    break;
                case 0x0a:
                    wait_for_key(c, hi & 0xf);
                    break;
                case 0x15:
                    set_delay_timer(c, hi & 0xf);
                    break;
                case 0x18:
                    set_sound_timer(c, hi & 0xf);
                    break;
                case 0x1e:
                    add_reg_index(c, hi & 0xf);
                    break;
                case 0x29:
                    get_font_char(c, hi & 0xf);
                    break;
                case 0x33:
                    bcd(c, hi & 0xf);
                    break;
                case 0x55:
                    store_reg(c, hi & 0xf);
                    break;
                case 0x65:
                    load_reg(c, hi & 0xf);
                    break;
                default:
                    fprintf(stderr, "[!] unimplemented opcode! 0x%04x\n", full);
            }
            break;
        default:
            fprintf(stderr, "[!] unimplemented opcode! 0x%04x\n", full);
    }
}

void load(chip_8 *c, char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "[!] failed to open the selected file!\n");
        exit(1);
    }
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    if (size > MEMORY_SIZE - PRG_ADDR) {
        fprintf(stderr, "[!] file is too large to be copied\n");
        exit(1);
    }
    fread(c->memory + PRG_ADDR, 1, size, fp);
    fread(c->program, 1, size, fp);
    printf("[*] loaded ROM\n");
    fclose(fp);
}
int main() {
    printf("[*] warming up...\n");

    chip_8 c8;
    init(&c8);

    memset(&c8.program, 0, MEMORY_SIZE);

    InitWindow(WIN_WIDTH, WIN_HEIGHT, "orca");
    SetTargetFPS(60);
    bool rom_loaded = false;
    bool tweak_win = false;
    while (!WindowShouldClose()) {
        BeginDrawing();
        GuiPanel((Rectangle) {0, 0, WIN_WIDTH, GUI_HEIGHT}, NULL);

        // load rom button
        if (GuiButton((Rectangle) {0, 0, GUI_HEIGHT, GUI_HEIGHT}, GuiIconText(ICON_FOLDER_FILE_OPEN, NULL))) {
            printf("OPEN ROM");
        }

        // save / load state button
        if (GuiButton((Rectangle) {GUI_HEIGHT, 0, GUI_HEIGHT, GUI_HEIGHT}, GuiIconText(ICON_FILE_SAVE_CLASSIC, NULL))) {
            printf("SAVE / LOAD STATE");
        }

        // OPERATION BUTTONS

        // play/pause button
        if (GuiButton((Rectangle) {WIN_WIDTH / 2 - GUI_HEIGHT * 1.5f, 0, GUI_HEIGHT, GUI_HEIGHT}, GuiIconText(c8.running ? ICON_PLAYER_PAUSE : ICON_PLAYER_PLAY, NULL))) {
            c8.running = !c8.running;
        }

        // step into button
        if (GuiButton((Rectangle) {WIN_WIDTH / 2 - GUI_HEIGHT / 2, 0, GUI_HEIGHT, GUI_HEIGHT}, GuiIconText(ICON_STEP_INTO, NULL))) {
            step(&c8);
        }

        // restart
        if (GuiButton((Rectangle) {WIN_WIDTH / 2 + GUI_HEIGHT / 2, 0, GUI_HEIGHT, GUI_HEIGHT}, GuiIconText(ICON_RESTART, NULL))) {
            init(&c8);
            memcpy(c8.program, c8.memory + PRG_ADDR, MEMORY_SIZE);
            c8.running = true;
        }

        //float memory_usage = 0;
        //foreach(uint8_t *v, c8.memory) {
         //   if (*v != 0) memory_usage += 1;
        //}
        //GuiProgressBar((Rectangle){GUI_HEIGHT * 5, 0, GUI_HEIGHT * 3, GUI_HEIGHT}, NULL, "  MEMORY USAGE", (memory_usage / MEMORY_SIZE) * 100, 0, 100);
        // char mem_text[] = "MEMORY USAGE";
        // int mem_font_size = 1;
        // DrawText(mem_text, (GUI_HEIGHT * 4) + MeasureText(mem_text, mem_font_size) / 6, GUI_HEIGHT / 2 - mem_foxnt_size, mem_font_size,
        // WHITE);

        if (GuiButton((Rectangle) {WIN_WIDTH - GUI_HEIGHT * 2, 0, GUI_HEIGHT, GUI_HEIGHT}, GuiIconText(ICON_EYE_ON, NULL))) {
            tweak_win = !tweak_win;
        }

        if (GuiButton((Rectangle) {WIN_WIDTH - GUI_HEIGHT, 0, GUI_HEIGHT, GUI_HEIGHT}, GuiIconText(ICON_GEAR, NULL))) {
            tweak_win = !tweak_win;
        }

        if (IsFileDropped() && !rom_loaded) {
            FilePathList dropped_files = LoadDroppedFiles();
            if (dropped_files.count == 1) {
                load(&c8, dropped_files.paths[0]);
                GuiEnable();
                rom_loaded = true;
                c8.running = true;
            }
            UnloadDroppedFiles(dropped_files);
        }
        int font_size = 20;
        char text[] = "Drag and drop here";
        if (!rom_loaded) {
            GuiDisable();
            DrawText(text, WIN_WIDTH / 2 - MeasureText(text, font_size) / 2, WIN_HEIGHT / 2 - font_size / 2, font_size,
                     WHITE);
        }
        if (c8.running) {
            if (c8.delay_timer > 0) c8.delay_timer--;
            for (uint8_t k = 0; k < 16; k++) {
                c8.keyold[k] = c8.keycur[k];
                c8.keycur[k] = IsKeyDown(keyMapping[k]);
            }
            for (int a = 0; a <= 8 && c8.running; a++) {
                step(&c8);
            }
        }
        draw_sprite_ray(&c8);
        if (tweak_win) {
            if (GuiWindowBox((Rectangle){0, 0 + GUI_HEIGHT - 1, (SCREEN_WIDTH * GFX_SCALE) / 2, SCREEN_HEIGHT * GFX_SCALE}, "viewing tweaks")) {
                tweak_win = !tweak_win;
            }
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
