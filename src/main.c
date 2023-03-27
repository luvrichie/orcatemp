#include <stdio.h>
#include <main.h>
#include <opcodes.h>
#include <stdbool.h>
#include <graphics.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <raylib.h>

void updateInput(chip_8 *c) {
    if (IsKeyDown(KEY_ONE)) {
        c->keycur[0x1] = true;
    } else if (IsKeyUp(KEY_ONE)) {
        c->keycur[0x1] = false;
    }
    if (IsKeyDown(KEY_TWO)) {
        c->keycur[0x2] = true;
    } else if (IsKeyUp(KEY_TWO)) {
        c->keycur[0x2] = false;
    }
    if (IsKeyDown(KEY_THREE)) {
        c->keycur[0x3] = true;
    } else if (IsKeyUp(KEY_THREE)) {
        c->keycur[0x3] = false;
    }
    if (IsKeyDown(KEY_FOUR)) {
        c->keycur[0xc] = true;
    } else if (IsKeyUp(KEY_FOUR)) {
        c->keycur[0xc] = false;
    }
    if (IsKeyDown(KEY_Q)) {
        c->keycur[0x4] = true;
    } else if (IsKeyUp(KEY_Q)) {
        c->keycur[0x4] = false;
    }
    if (IsKeyDown(KEY_W)) {
        c->keycur[0x5] = true;
    } else if (IsKeyUp(KEY_W)) {
        c->keycur[0x5] = false;
    }
    if (IsKeyDown(KEY_E)) {
        c->keycur[0x6] = true;
    } else if (IsKeyUp(KEY_E)) {
        c->keycur[0x6] = false;
    }
    if (IsKeyDown(KEY_R)) {
        c->keycur[0xd] = true;
    } else if (IsKeyUp(KEY_R)) {
        c->keycur[0xd] = false;
    }
    if (IsKeyDown(KEY_A)) {
        c->keycur[0x7] = true;
    } else if (IsKeyUp(KEY_A)) {
        c->keycur[0x7] = false;
    }
    if (IsKeyDown(KEY_S)) {
        c->keycur[0x8] = true;
    } else if (IsKeyUp(KEY_S)) {
        c->keycur[0x8] = false;
    }
    if (IsKeyDown(KEY_D)) {
        c->keycur[0x9] = true;
    } else if (IsKeyUp(KEY_D)) {
        c->keycur[0x9] = false;
    }
    if (IsKeyDown(KEY_F)) {
        c->keycur[0xE] = true;
    } else if (IsKeyUp(KEY_F)) {
        c->keycur[0xE] = false;
    }
    if (IsKeyDown(KEY_Z)) {
        c->keycur[0xA] = true;
    } else if (IsKeyUp(KEY_Z)) {
        c->keycur[0xA] = false;
    }
    if (IsKeyDown(KEY_X)) {
        c->keycur[0x0] = true;
    } else if (IsKeyUp(KEY_X)) {
        c->keycur[0x0] = false;
    }
    if (IsKeyDown(KEY_C)) {
        c->keycur[0xB] = true;
    } else if (IsKeyUp(KEY_C)) {
        c->keycur[0xB] = false;
    }
    if (IsKeyDown(KEY_V)) {
        c->keycur[0xF] = true;
    } else if (IsKeyUp(KEY_V)) {
        c->keycur[0xF] = false;
    }
}

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

    c->memory[0x1FF] = 5;
    c->memory[0x1FE] = 1;
    printf("[*] init finished!\n");
}

void step(chip_8 *c) {
    uint8_t hi = c->memory[c->pc];
    uint8_t lo = c->memory[c->pc + 1];
    uint16_t full = (hi << 8) | lo;
    c->pc += 2;
    printf("%p\n", full);
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
    printf("[*] loaded ROM\n");
    fclose(fp);
}

int main() {
    printf("[*] warming up...\n");

    chip_8 c8;
    init(&c8);

    InitWindow(WIN_WIDTH, WIN_HEIGHT, "orca");
    const float fps = 60.0f;
    SetTargetFPS(fps);
    bool rom_loaded = false;
    while (!WindowShouldClose()) {
        if (IsFileDropped() && !rom_loaded) {
            FilePathList dropped_files = LoadDroppedFiles();
            if (dropped_files.count == 1) {
                rom_loaded = true;
                load(&c8, dropped_files.paths[0]);
                printf("%s", dropped_files.paths[0]);
            }
            UnloadDroppedFiles(dropped_files);
        }
        int font_size = 20;
        char text[] = "Drag and drop here";
        if (!rom_loaded) {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText(text, WIN_WIDTH / 2 - MeasureText(text, font_size) / 2, WIN_HEIGHT / 2 - font_size / 2, font_size,
                     WHITE);
            EndDrawing();
        }
        if (rom_loaded) {
            updateInput(&c8);
            if (c8.delay_timer > 0) c8.delay_timer--;
            for (uint8_t k = 0; k < 16; k++) {
                c8.keyold[k] = c8.keycur[k];
                c8.keycur[k] = IsKeyDown(k);
            }
            for (int a = 0; a <= 8; a++) {
                step(&c8);
            }
            draw_sprite_ray(&c8);
        }
    }
    return 0;
}
