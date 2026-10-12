/*
 * Mosslight - Game Boy Color ROM
 * A relaxing wildlife garden game built with GBDK-2020
 */

#include "mosslight.h"
#include "data_tables.h"

void game_init(void) {
    cpu_fast();
    memset(&game, 0, sizeof(game));

    game.state = STATE_TITLE;
    game.time = TIME_MORNING;
    game.weather = WEATHER_CLEAR;
    game.day = 1;
    game.equipped_tool = TOOL_TROWEL;
    game.placing_item = 0xFF;
    game.unlocked[AREA_MAIN_GARDEN] = true;

    for (uint8_t i = 0; i < 8; i++)
        game.wildlife[i].active = false;

    input_update();
    load_tiles();
    load_sprites();
    audio_init();
}

void main(void) {
    game_init();

    while (1) {
        frame_count++;
        input_update();
        if (is_menu_cooling_down()) {
            /* still tick the cooldown */
        }

        switch (game.state) {
            case STATE_TITLE:
                {
                    static bool title_drawn = false;
                    if (!title_drawn) {
                        fill_bkg_rect(0, 0, 20, 18, 0);
                        /* Set palette attributes so title screen uses palette 3 (UI) */
                        VBK_REG = 1;
                        fill_bkg_rect(0, 0, 20, 18, 3);
                        VBK_REG = 0;
                        load_font();
                        draw_text_centered("MOSSLIGHT", 6);
                        draw_text_centered("A WILDLIFE GARDEN", 8);
                        draw_text_centered("PRESS START", 12);
                        move_win(7, 7);
                        HIDE_WIN;
                        title_drawn = true;
                    }
                    if (btn_pressed(BTN_START | BTN_A)) {
                        if (has_save()) {
                            load_game();
                        } else {
                            game.inventory[PLANT_CONEFLOWER] = 2;
                            game.inventory[PLANT_BLACK_EYED_SUSAN] = 2;
                            game.tutorial_step = 1;
                            player_init();
                            load_area(AREA_MAIN_GARDEN);
                        }
                        game.state = STATE_OVERWORLD;
                        play_sfx(SFX_CONFIRM);
                        set_menu_cooldown(15);
                    }
                }
                break;

            case STATE_OVERWORLD:
            case STATE_PLACEMENT:
                update_overworld();
                draw_overworld();
                break;

            case STATE_MENU:
                menu_update();
                menu_draw();
                break;

            case STATE_FIELD_GUIDE:
                fieldguide_update();
                fieldguide_draw();
                break;

            case STATE_SHOP:
                shop_update();
                shop_draw();
                break;

            case STATE_ENDING:
            case STATE_CREDITS:
                ending_update();
                ending_draw();
                break;

            default:
                game.state = STATE_OVERWORLD;
                break;
        }

        update_banner();
        music_tick();
        wait_vbl_done();
    }
}
