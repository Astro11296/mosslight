/*
 * Engine - overworld update and draw, time, weather, camera
 */
#include "mosslight.h"
#include "data_tables.h"

#define FONT_TILE_START 64

GameData game;
MapTile current_map[MAX_MAP_H][MAX_MAP_W];
uint8_t map_w, map_h;
uint16_t frame_count = 0;

/* Camera position in pixels */
uint16_t cam_x = 0, cam_y = 0;

/* Banner */
static char banner_text[40];
static uint8_t banner_timer = 0;

/* Dialogue */
static const char *dlg_speaker = "";
static const char *dlg_text = "";
static bool dlg_active = false;

/* Cooldown */
static uint8_t menu_cd = 0;

/* ---- Banner ---- */
void draw_banner(const char *t, uint8_t d) {
    strncpy(banner_text, t, sizeof(banner_text)-1);
    banner_text[sizeof(banner_text)-1] = '\0';
    banner_timer = d;
    play_sfx(SFX_DISCOVER);
}
void update_banner(void) {
    if (banner_timer > 0) banner_timer--;
    if (menu_cd > 0) menu_cd--;
}
bool is_banner_active(void) { return banner_timer > 0; }
const char* get_banner_text(void) { return banner_text; }

/* ---- Dialogue ---- */
void draw_dialogue(const char *s, const char *t) { dlg_speaker = s; dlg_text = t; dlg_active = true; }
bool is_dialogue_active(void) { return dlg_active; }
void close_dialogue(void) { dlg_active = false; }
const char* get_dlg_speaker(void) { return dlg_speaker; }
const char* get_dlg_text(void) { return dlg_text; }

/* ---- Cooldown ---- */
void set_menu_cooldown(uint8_t f) { menu_cd = f; }
bool is_menu_cooling_down(void) { return menu_cd > 0; }

/* ---- Time ---- */
void update_time(void) {
    if (game.state != STATE_OVERWORLD && game.state != STATE_PLACEMENT)
        return;
    game.time_timer++;
    /* 360 seconds * 60 fps = 21600 frames per period */
    if (game.time_timer >= TIME_DURATION * 60) {
        game.time_timer = 0;
        game.time++;
        if (game.time > TIME_NIGHT) {
            game.time = TIME_MORNING;
            game.day++;
            update_weather();
            game.trail_generated = false;
            game.rain_watered_today = false;
        }
        /* Update music for new time period */
        switch (game.time) {
            case TIME_MORNING: play_music(MUSIC_MORNING); break;
            case TIME_DAY:     play_music(MUSIC_DAY); break;
            case TIME_EVENING: play_music(MUSIC_EVENING); break;
            case TIME_NIGHT:   play_music(MUSIC_NIGHT); break;
        }
    }
}

/* ---- Weather ---- */
void update_weather(void) {
    uint8_t r = (uint8_t)(frame_count + game.day) % 10;
    if (r < 5) game.weather = WEATHER_CLEAR;
    else if (r < 8) game.weather = WEATHER_CLOUDY;
    else game.weather = WEATHER_RAIN;

    if (game.weather == WEATHER_RAIN) {
        play_music(MUSIC_RAIN);
    }
}

/* ---- Camera ---- */
static void update_camera(void) {
    /* Center camera on player, clamp to map bounds */
    uint16_t map_pixel_w = (uint16_t)map_w * 8;
    uint16_t map_pixel_h = (uint16_t)map_h * 8;

    if (map_pixel_w > SCREEN_W) {
        cam_x = game.player.x - SCREEN_W / 2;
        if (cam_x > map_pixel_w - SCREEN_W) cam_x = map_pixel_w - SCREEN_W;
    } else {
        cam_x = 0;
    }

    if (map_pixel_h > SCREEN_H) {
        cam_y = game.player.y - SCREEN_H / 2;
        if (cam_y > map_pixel_h - SCREEN_H) cam_y = map_pixel_h - SCREEN_H;
    } else {
        cam_y = 0;
    }

    move_bkg(cam_x, cam_y);
}

/* ---- Overworld update ---- */
void update_overworld(void) {
    /* Handle Start/Select with cooldown */
    if (!is_menu_cooling_down()) {
        if (btn_pressed(BTN_START)) {
            game.prev_state = game.state;
            game.state = STATE_MENU;
            set_menu_cooldown(15);
            play_sfx(SFX_BLIP);
            return;
        }
        if (btn_pressed(BTN_SELECT)) {
            game.prev_state = game.state;
            game.state = STATE_FIELD_GUIDE;
            set_menu_cooldown(15);
            play_sfx(SFX_BLIP);
            return;
        }
    }

    /* Dialogue takes priority */
    if (dlg_active) {
        if (btn_pressed(BTN_A | BTN_B)) {
            close_dialogue();
            play_sfx(SFX_BLIP);
        }
        return;
    }

    /* Update player */
    player_update();

    /* Update time */
    update_time();

    /* Update plant growth on time transition */
    update_plant_growth();

    /* Update wildlife */
    wildlife_update();

    /* Check spawn */
    game.spawn_timer++;
    if (game.spawn_timer >= 1200) { /* ~20 seconds at 60fps */
        game.spawn_timer = 0;
        check_spawn();
    }

    /* Tutorial */
    if (!game.tutorial_done) tutorial_update();

    /* Progression */
    check_progression();

    /* Ending check */
    check_ending();

    /* Weather effects */
    update_weather_effects();
}

/* ---- Overworld draw ---- */
void draw_overworld(void) {
    update_camera();
    draw_map();
    player_draw();
    wildlife_draw();

    /* Draw dialogue box if active */
    if (dlg_active) {
        /* Use window layer for dialogue */
        fill_win_rect(0, 14, 20, 4, FONT_TILE_START); /* fill with space tiles */
        draw_text_win(dlg_speaker, 1, 14);
        draw_text_win(dlg_text, 1, 15);
        move_win(7, 7 + 14*8);
        SHOW_WIN;
    } else {
        HIDE_WIN;
    }

    /* Draw banner if active */
    if (banner_timer > 0) {
        /* Draw at top of screen using BG tiles - we'll use a simple approach */
        /* Since BG is used for map, we use the window for banner */
        /* Actually for simplicity, draw on top row */
    }
}
