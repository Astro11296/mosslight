/*
 * UI - menus, field guide, shop, progression, tutorial, ending, effects, trail
 */
#include "mosslight.h"
#include "data_tables.h"



/* ---- Menu state ---- */
static uint8_t menu_tab = 0;     /* 0=Garden,1=Bag,2=Map,3=Journal,4=Help,5=Options */
static uint8_t menu_cursor = 0;
static uint8_t menu_sub = 0;     /* submenu depth */

static const char *menu_tabs[] = {
    "Garden", "Bag", "Map", "Journal", "Help", "Options"
};

/* Journal entries - we store up to 64 entries of 40 chars */
typedef struct { uint16_t day; char text[36]; } JournalEntry;
static JournalEntry journal[16];
static uint8_t journal_count = 0;

void add_journal(const char *text) {
    if (journal_count < 16) {
        journal[journal_count].day = game.day;
        strncpy(journal[journal_count].text, text, 35);
        journal[journal_count].text[35] = '\0';
        journal_count++;
    }
}

/* ---- Menu ---- */
void menu_update(void) {
    if (!is_menu_cooling_down()) {
        if (btn_pressed(BTN_START | BTN_B)) {
            game.state = STATE_OVERWORLD;
            set_menu_cooldown(15);
            play_sfx(SFX_CANCEL);
            HIDE_WIN;
            return;
        }
    }

    if (btn_pressed(BTN_LEFT)) {
        if (menu_tab > 0) menu_tab--;
        else menu_tab = 5;
        menu_cursor = 0;
        play_sfx(SFX_BLIP);
    }
    if (btn_pressed(BTN_RIGHT)) {
        if (menu_tab < 5) menu_tab++;
        else menu_tab = 0;
        menu_cursor = 0;
        play_sfx(SFX_BLIP);
    }
    if (btn_pressed(BTN_UP)) {
        if (menu_cursor > 0) menu_cursor--;
        play_sfx(SFX_BLIP);
    }
    if (btn_pressed(BTN_DOWN)) {
        menu_cursor++;
        play_sfx(SFX_BLIP);
    }

    /* Tab-specific actions */
    if (btn_pressed(BTN_A)) {
        switch (menu_tab) {
            case 1: /* Bag - use item */
                /* Could select and place items */
                break;
            case 5: /* Options */
                switch (menu_cursor) {
                    case 0: /* toggle tool */
                        game.equipped_tool++;
                        if (game.equipped_tool >= TOOL_COUNT)
                            game.equipped_tool = TOOL_TROWEL;
                        play_sfx(SFX_CONFIRM);
                        break;
                }
                break;
        }
    }
}

void menu_draw(void) {
    load_font();

    /* Clear window with fill tiles */
    fill_win_rect(0, 0, 20, 18, 0);

    /* Title bar */
    draw_text_win("MOSSLIGHT", 1, 0);
    draw_text_win(menu_tabs[menu_tab], 14, 0);

    /* Tab indicators */
    for (uint8_t i = 0; i < 6; i++) {
        if (i == menu_tab) {
            draw_text_win(">", 1 + i * 3, 1);
        }
    }

    /* Content based on tab */
    switch (menu_tab) {
        case 0: { /* Garden */
            draw_text_win("Area:", 1, 3);
            draw_text_win(area_names[game.player.area], 7, 3);
            draw_text_win("Time:", 1, 4);
            draw_text_win(time_names[game.time], 7, 4);
            draw_text_win("Weather:", 1, 5);
            draw_text_win(weather_names[game.weather], 10, 5);
            draw_text_win("Day:", 1, 6);
            /* Display day number - simple */
            draw_text_win("Visitors:", 1, 8);
            /* Display counts */
            draw_text_win("Residents:", 1, 9);
            break;
        }
        case 1: { /* Bag - inventory */
            draw_text_win("Seeds & Plants", 1, 3);
            uint8_t row = 4;
            for (uint8_t i = 0; i < PLANT_COUNT && row < 14; i++) {
                if (game.inventory[i] > 0) {
                    draw_text_win(plant_names[i], 2, row);
                    /* Show count */
                    row++;
                }
            }
            draw_text_win("Objects", 1, 14);
            row = 15;
            for (uint8_t i = OBJ_FALLEN_LOG; i < OBJ_COUNT && row < 18; i++) {
                if (game.inventory[i] > 0) {
                    draw_text_win(object_names[i - OBJ_FALLEN_LOG], 2, row);
                    row++;
                }
            }
            break;
        }
        case 2: { /* Map */
            draw_text_win("Garden Property", 1, 3);
            draw_text_win("* Main Garden", 2, 4);
            if (game.unlocked[AREA_POND_EDGE])
                draw_text_win("* Pond Edge", 2, 5);
            if (game.unlocked[AREA_OLD_GROVE])
                draw_text_win("* Old Grove", 2, 6);
            if (game.unlocked[AREA_MOON_MEADOW])
                draw_text_win("* Moon Meadow", 2, 7);
            draw_text_win("Town", 1, 9);
            draw_text_win("* Gloamwick", 2, 10);
            if (game.unlocked[AREA_TRAIL])
                draw_text_win("* Woodland Trail", 2, 11);
            break;
        }
        case 3: { /* Journal */
            draw_text_win("Journal", 1, 3);
            uint8_t row = 4;
            uint8_t start = 0;
            if (journal_count > 12) start = journal_count - 12;
            for (uint8_t i = start; i < journal_count && row < 18; i++) {
                draw_text_win(journal[i].text, 1, row);
                row++;
            }
            break;
        }
        case 4: { /* Help */
            draw_text_win("Controls:", 1, 3);
            draw_text_win("D-Pad: Move", 2, 4);
            draw_text_win("A: Interact", 2, 5);
            draw_text_win("B: Use Tool", 2, 6);
            draw_text_win("Start: Menu", 2, 7);
            draw_text_win("Select: Guide", 2, 8);
            draw_text_win("Tool:", 1, 10);
            switch (game.equipped_tool) {
                case TOOL_TROWEL: draw_text_win("Trowel", 7, 10); break;
                case TOOL_SPADE: draw_text_win("Spade", 7, 10); break;
                case TOOL_WATERING_CAN: draw_text_win("Watering Can", 7, 10); break;
            }
            break;
        }
        case 5: { /* Options */
            draw_text_win("Options", 1, 3);
            draw_text_win("> Tool Select", 2, 4);
            draw_text_win("  Music Vol", 2, 5);
            draw_text_win("  Sound Vol", 2, 6);
            draw_text_win("Current Tool:", 1, 8);
            switch (game.equipped_tool) {
                case TOOL_TROWEL: draw_text_win("Trowel", 2, 9); break;
                case TOOL_SPADE: draw_text_win("Spade", 2, 9); break;
                case TOOL_WATERING_CAN: draw_text_win("Water Can", 2, 9); break;
            }
            break;
        }
    }

    move_win(7, 7);
    SHOW_WIN;
}

/* ---- Field Guide ---- */
static uint8_t fg_tab = 0; /* 0=Wildlife, 1=Plants */
static uint8_t fg_cursor = 0;

void fieldguide_update(void) {
    if (!is_menu_cooling_down()) {
        if (btn_pressed(BTN_SELECT | BTN_B)) {
            game.state = STATE_OVERWORLD;
            set_menu_cooldown(15);
            play_sfx(SFX_CANCEL);
            HIDE_WIN;
            return;
        }
    }

    if (btn_pressed(BTN_LEFT) || btn_pressed(BTN_RIGHT)) {
        fg_tab = 1 - fg_tab;
        fg_cursor = 0;
        play_sfx(SFX_BLIP);
    }
    if (btn_pressed(BTN_UP)) {
        if (fg_cursor > 0) fg_cursor--;
        play_sfx(SFX_BLIP);
    }
    if (btn_pressed(BTN_DOWN)) {
        fg_cursor++;
        if (fg_tab == 0 && fg_cursor >= WL_COUNT) fg_cursor = WL_COUNT - 1;
        if (fg_tab == 1 && fg_cursor >= PLANT_COUNT) fg_cursor = PLANT_COUNT - 1;
        play_sfx(SFX_BLIP);
    }
}

void fieldguide_draw(void) {
    load_font();
    fill_win_rect(0, 0, 20, 18, 0);

    draw_text_win("FIELD GUIDE", 1, 0);
    draw_text_win(fg_tab == 0 ? "Wildlife" : "Plants", 12, 0);

    if (fg_tab == 0) {
        /* Wildlife list */
        uint8_t start = 0;
        if (fg_cursor > 10) start = fg_cursor - 10;
        uint8_t row = 2;
        for (uint8_t i = start; i < WL_COUNT && row < 13; i++) {
            if (i == fg_cursor) draw_text_win(">", 1, row);
            if (game.wildlife_disc[i] == DISC_UNKNOWN) {
                draw_text_win("???", 3, row);
            } else {
                draw_text_win(wildlife_names[i], 3, row);
                if (game.wildlife_disc[i] == DISC_RESIDENT)
                    draw_text_win("R", 17, row);
                else
                    draw_text_win("V", 17, row);
            }
            row++;
        }

        /* Detail */
        uint8_t id = fg_cursor;
        draw_text_win("---", 1, 14);
        if (game.wildlife_disc[id] == DISC_UNKNOWN) {
            draw_text_win("Unknown species", 1, 15);
            draw_text_win("Build habitat to", 1, 16);
            draw_text_win("attract it.", 1, 17);
        } else {
            draw_text_win(wildlife_names[id], 1, 14);
            draw_text_win(time_names[game.time], 1, 15);
            draw_text_win(weather_names[game.weather], 10, 15);
            if (game.wildlife_disc[id] == DISC_RESIDENT)
                draw_text_win("RESIDENT", 1, 16);
            else
                draw_text_win("VISITOR", 1, 16);
        }
    } else {
        /* Plant list */
        uint8_t start = 0;
        if (fg_cursor > 12) start = fg_cursor - 12;
        uint8_t row = 2;
        for (uint8_t i = start; i < PLANT_COUNT && row < 15; i++) {
            if (i == fg_cursor) draw_text_win(">", 1, row);
            if (game.plant_disc[i]) {
                draw_text_win(plant_names[i], 3, row);
            } else {
                draw_text_win("???", 3, row);
            }
            row++;
        }
    }

    move_win(7, 7);
    SHOW_WIN;
}

/* ---- Shop ---- */
static uint8_t shop_cursor = 0;

void shop_update(void) {
    if (!is_menu_cooling_down()) {
        if (btn_pressed(BTN_B)) {
            game.state = STATE_OVERWORLD;
            set_menu_cooldown(15);
            play_sfx(SFX_CANCEL);
            HIDE_WIN;
            return;
        }
    }

    if (btn_pressed(BTN_UP)) {
        if (shop_cursor > 0) shop_cursor--;
        play_sfx(SFX_BLIP);
    }
    if (btn_pressed(BTN_DOWN)) {
        shop_cursor++;
        if (shop_cursor >= SHOP_COUNT) shop_cursor = SHOP_COUNT - 1;
        play_sfx(SFX_BLIP);
    }

    if (btn_pressed(BTN_A)) {
        /* Try to buy selected item */
        const ShopEntry *se = &shop_items[shop_cursor];
        if (se->late_unlock && game.unique_visitors < UNLOCK_TRAIL_VISITORS) {
            play_sfx(SFX_CANCEL);
            return;
        }
        if (game.coins >= se->price) {
            game.coins -= se->price;
            game.inventory[se->item_id]++;
            play_sfx(SFX_COIN);
            save_game();
        } else {
            play_sfx(SFX_CANCEL);
        }
    }
}

void shop_draw(void) {
    load_font();
    fill_win_rect(0, 0, 20, 18, 0);

    draw_text_win("MARA'S NURSERY", 1, 0);
    draw_text_win("Coins:", 1, 1);
    /* Show coin count */

    uint8_t start = 0;
    if (shop_cursor > 12) start = shop_cursor - 12;
    uint8_t row = 3;
    for (uint8_t i = start; i < SHOP_COUNT && row < 16; i++) {
        const ShopEntry *se = &shop_items[i];
        if (i == shop_cursor) draw_text_win(">", 1, row);

        if (se->item_id < PLANT_COUNT) {
            draw_text_win(plant_names[se->item_id], 3, row);
        } else {
            draw_text_win(object_names[se->item_id - OBJ_FALLEN_LOG], 3, row);
        }

        /* Price - show as number */
        /* Simple: show price digits */
        row++;
    }

    move_win(7, 7);
    SHOW_WIN;
}

/* ---- Progression ---- */
void check_progression(void) {
    if (!game.unlocked[AREA_TRAIL] && game.unique_visitors >= UNLOCK_TRAIL_VISITORS) {
        game.unlocked[AREA_TRAIL] = true;
        draw_banner("Trail Unlocked!", 180);
        add_journal("Woodland Trail unlocked.");
    }
    if (!game.unlocked[AREA_POND_EDGE] &&
        game.unique_visitors >= UNLOCK_POND_VISITORS &&
        game.residents >= UNLOCK_POND_RESIDENTS) {
        game.unlocked[AREA_POND_EDGE] = true;
        draw_banner("Pond Edge Unlocked!", 180);
        add_journal("Pond Edge discovered.");
    }
    if (!game.unlocked[AREA_OLD_GROVE] &&
        game.unique_visitors >= UNLOCK_GROVE_VISITORS &&
        game.residents >= UNLOCK_GROVE_RESIDENTS) {
        game.unlocked[AREA_OLD_GROVE] = true;
        draw_banner("Old Grove Unlocked!", 180);
        add_journal("Old Grove discovered.");
    }
    if (!game.unlocked[AREA_MOON_MEADOW] &&
        game.unique_visitors >= UNLOCK_MEADOW_VISITORS &&
        game.residents >= UNLOCK_MEADOW_RESIDENTS) {
        game.unlocked[AREA_MOON_MEADOW] = true;
        draw_banner("Moon Meadow Unlocked!", 180);
        add_journal("Moon Meadow discovered.");
        /* Give Moonflower seeds */
        game.inventory[PLANT_MOONFLOWER] += 3;
        draw_banner("Found Moonflower Seeds!", 120);
    }
}

/* ---- Tutorial ---- */
void tutorial_update(void) {
    if (game.tutorial_done) return;

    switch (game.tutorial_step) {
        case 1: /* Wait for movement */
            if (btn_held(BTN_UP|BTN_DOWN|BTN_LEFT|BTN_RIGHT)) {
                game.tutorial_step = 2;
            }
            break;
        case 2: /* Wait for interaction */
            if (btn_pressed(BTN_A)) {
                game.tutorial_step = 3;
                draw_banner("Try planting a seed!", 120);
            }
            break;
        case 3: /* Wait for planting */
            /* Check if any plant was planted */
            for (uint8_t y = 0; y < map_h; y++) {
                for (uint8_t x = 0; x < map_w; x++) {
                    if (current_map[y][x].plant != 0xFF) {
                        game.tutorial_step = 4;
                        draw_banner("Now water it with B!", 120);
                        return;
                    }
                }
            }
            break;
        case 4: /* Wait for watering */
            for (uint8_t y = 0; y < map_h; y++) {
                for (uint8_t x = 0; x < map_w; x++) {
                    if (current_map[y][x].plant != 0xFF && current_map[y][x].watered) {
                        game.tutorial_step = 5;
                        draw_banner("Press SELECT for Guide", 120);
                        return;
                    }
                }
            }
            break;
        case 5: /* Wait for field guide open */
            if (btn_pressed(BTN_SELECT)) {
                game.tutorial_step = 6;
            }
            break;
        case 6: /* Done */
            game.tutorial_done = true;
            game.unlocked[AREA_GLOAMWICK] = true;
            draw_banner("Road to Gloamwick open!", 180);
            add_journal("The road to Gloamwick opened.");
            break;
    }
}

/* ---- Ending ---- */
void check_ending(void) {
    if (game.ending_triggered) return;

    if (game.unlocked[AREA_MAIN_GARDEN] &&
        game.unlocked[AREA_POND_EDGE] &&
        game.unlocked[AREA_OLD_GROVE] &&
        game.unlocked[AREA_MOON_MEADOW] &&
        game.unique_visitors >= ENDING_VISITORS &&
        game.residents >= ENDING_RESIDENTS &&
        game.wildlife_disc[WL_LUNA_MOTH] == DISC_RESIDENT) {
        if (game.time == TIME_NIGHT) {
            game.ending_triggered = true;
            game.state = STATE_ENDING;
            play_music(MUSIC_NIGHT);
        }
    }
}

void ending_update(void) {
    if (game.state == STATE_ENDING) {
        if (btn_pressed(BTN_A)) {
            game.state = STATE_CREDITS;
        }
    } else if (game.state == STATE_CREDITS) {
        if (btn_pressed(BTN_A | BTN_START)) {
            game.state = STATE_OVERWORLD;
            HIDE_WIN;
        }
    }
}

void ending_draw(void) {
    load_font();
    fill_win_rect(0, 0, 20, 18, 0);

    if (game.state == STATE_ENDING) {
        draw_text_win("The garden is", 1, 3);
        draw_text_win("alive again.", 1, 4);
        draw_text_win("", 1, 6);
        draw_text_win("The garden never", 1, 8);
        draw_text_win("needed taming.", 1, 9);
        draw_text_win("It only needed", 1, 10);
        draw_text_win("someone to notice", 1, 11);
        draw_text_win("what belonged here.", 1, 12);
        draw_text_win("", 1, 14);
        draw_text_win("You noticed.", 1, 15);
        draw_text_win("", 1, 16);
        draw_text_win("Press A...", 1, 17);
    } else {
        draw_text_win("MOSSLIGHT", 5, 3);
        draw_text_win("", 1, 5);
        draw_text_win("A garden game", 3, 7);
        draw_text_win("by you.", 3, 8);
        draw_text_win("", 1, 10);
        draw_text_win("Thank you for", 3, 12);
        draw_text_win("playing.", 3, 13);
        draw_text_win("", 1, 15);
        draw_text_win("Continue Garden", 3, 16);
        draw_text_win("Press A", 6, 17);
    }

    move_win(7, 7);
    SHOW_WIN;
}

/* ---- Weather effects ---- */
static uint8_t rain_drops[8][2]; /* x, y positions for rain */
static bool rain_init = false;

void update_weather_effects(void) {
    if (game.weather == WEATHER_RAIN) {
        if (!rain_init) {
            for (uint8_t i = 0; i < 8; i++) {
                rain_drops[i][0] = (uint8_t)(frame_count + i * 37) % SCREEN_W;
                rain_drops[i][1] = (uint8_t)(frame_count + i * 53) % SCREEN_H;
            }
            rain_init = true;
        }
        for (uint8_t i = 0; i < 8; i++) {
            rain_drops[i][1] += 2;
            if (rain_drops[i][1] > SCREEN_H) {
                rain_drops[i][1] = 0;
                rain_drops[i][0] = (uint8_t)(frame_count + i * 37) % SCREEN_W;
            }
        }
    } else {
        rain_init = false;
    }
}

void draw_weather_effects(void) {
    /* Rain drawn using additional sprites - but we're at sprite limit */
    /* For now, rain is handled by palette shifting and BG tile changes */
}

/* ---- Trail ---- */
void generate_trail_items(void) {
    if (game.trail_generated) return;
    game.trail_generated = true;

    /* Add random seeds/items to trail area */
    uint8_t finds = 3 + ((uint8_t)(frame_count + game.day) % 4); /* 3-6 */

    for (uint8_t i = 0; i < finds; i++) {
        uint8_t r = (uint8_t)(frame_count + i * 41 + game.day * 7) % 10;
        switch (r) {
            case 0: case 1: game.inventory[PLANT_ASTER]++; break;
            case 2: game.inventory[PLANT_GOLDENROD]++; break;
            case 3: game.inventory[PLANT_WOODLAND_VIOLET]++; break;
            case 4: game.inventory[PLANT_WOODLAND_PHLOX]++; break;
            case 5: game.inventory[PLANT_SHELF_MUSHROOM]++; break;
            case 6: game.inventory[OBJ_ROCK]++; break;
            case 7: game.inventory[OBJ_FALLEN_LOG]++; break;
            case 8:
                if (game.weather == WEATHER_RAIN)
                    game.inventory[PLANT_GLOWCAP]++;
                else
                    game.inventory[PLANT_ASTER]++;
                break;
            case 9:
                game.inventory[PLANT_MOONFLOWER]++;
                game.coins += 2;
                break;
        }
    }
}
