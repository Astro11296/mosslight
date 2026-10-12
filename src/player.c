/*
 * Player - movement, animation, interaction, tool use
 */
#include "mosslight.h"
#include "data_tables.h"

extern uint16_t cam_x_get(void);
extern uint16_t cam_y_get(void);

/* Pixel speed */
#define PLAYER_SPEED 1

void player_init(void) {
    game.player.x = 4 * 8;  /* tile 4, pixel position */
    game.player.y = 6 * 8;
    game.player.direction = DIR_DOWN;
    game.player.frame = 0;
    game.player.frame_timer = 0;
    game.player.moving = false;
    game.player.target_x = game.player.x;
    game.player.target_y = game.player.y;
}

void player_update(void) {
    Player *p = &game.player;

    /* If currently moving to target, continue */
    if (p->moving) {
        if (p->x < p->target_x) p->x += PLAYER_SPEED;
        if (p->x > p->target_x) p->x -= PLAYER_SPEED;
        if (p->y < p->target_y) p->y += PLAYER_SPEED;
        if (p->y > p->target_y) p->y -= PLAYER_SPEED;

        if (p->x == p->target_x && p->y == p->target_y) {
            p->moving = false;
            p->frame = 0;
        }

        /* Animation */
        p->frame_timer++;
        if (p->frame_timer >= 8) {
            p->frame_timer = 0;
            p->frame = (p->frame + 1) & 1;
        }
        return;
    }

    /* Check for movement input */
    int8_t dir_x = 0, dir_y = 0;
    if (btn_held(BTN_UP))    { dir_y = -1; p->direction = DIR_UP; }
    if (btn_held(BTN_DOWN))  { dir_y =  1; p->direction = DIR_DOWN; }
    if (btn_held(BTN_LEFT))  { dir_x = -1; p->direction = DIR_LEFT; }
    if (btn_held(BTN_RIGHT)) { dir_x =  1; p->direction = DIR_RIGHT; }

    if (dir_x != 0 || dir_y != 0) {
        /* Try to move in tile increments */
        uint16_t nx = p->x + dir_x * 8;
        uint16_t ny = p->y + dir_y * 8;
        uint8_t tx = nx / 8;
        uint8_t ty = ny / 8;

        if (can_walk(tx, ty)) {
            p->target_x = nx;
            p->target_y = ny;
            p->moving = true;
        }
    }

    /* A button - interact */
    if (btn_pressed(BTN_A)) {
        player_try_interact();
    }

    /* B button - use tool (outdoors only) */
    if (btn_pressed(BTN_B)) {
        if (game.state == STATE_OVERWORLD && is_garden_area(game.player.area)) {
            player_try_use_tool();
        }
    }
}

void player_draw(void) {
    Player *p = &game.player;
    uint16_t px = p->x - cam_x_get();
    uint16_t py = p->y - cam_y_get();

    /* Player is 16x16 = 4 8x8 sprites */
    uint8_t base = get_player_sprite_base(p->direction);

    /* Sprite 0: top-left */
    set_sprite_tile(0, base);
    set_sprite_prop(0, 0); /* palette 0 */
    move_sprite(0, px, py);

    /* Sprite 1: top-right */
    set_sprite_tile(1, base + 1);
    set_sprite_prop(1, 0);
    move_sprite(1, px + 8, py);

    /* Sprite 2: bottom-left */
    set_sprite_tile(2, base + 2);
    set_sprite_prop(2, 0);
    move_sprite(2, px, py + 8);

    /* Sprite 3: bottom-right */
    set_sprite_tile(3, base + 3);
    set_sprite_prop(3, 0);
    move_sprite(3, px + 8, py + 8);
}

/* Get tile in front of player */
static void get_front_tile(uint8_t *tx, uint8_t *ty) {
    *tx = game.player.x / 8;
    *ty = game.player.y / 8;
    switch (game.player.direction) {
        case DIR_UP:    if (*ty > 0) (*ty)--; break;
        case DIR_DOWN:  (*ty)++; break;
        case DIR_LEFT:  if (*tx > 0) (*tx)--; break;
        case DIR_RIGHT: (*tx)++; break;
    }
}

void player_try_interact(void) {
    uint8_t tx, ty;
    get_front_tile(&tx, &ty);

    if (tx >= map_w || ty >= map_h) return;

    MapTile *tile = &current_map[ty][tx];

    /* Check for cottage bed (rest) */
    if (tile->terrain == TERRAIN_WALL && game.player.area == AREA_MAIN_GARDEN) {
        /* Near cottage - rest */
        draw_dialogue("Cottage", "Rest until morning?\nA: Yes  B: No");
        /* Simplified: just advance time */
        return;
    }

    /* Check for shop */
    if (game.player.area == AREA_GLOAMWICK) {
        if (tx >= 4 && tx <= 8 && ty >= 4 && ty <= 6) {
            game.prev_state = game.state;
            game.state = STATE_SHOP;
            set_menu_cooldown(15);
            play_sfx(SFX_BLIP);
            return;
        }
    }

    /* Check for plants - inspect */
    if (tile->plant != 0xFF && tile->plant < PLANT_COUNT) {
        draw_dialogue(plant_names[tile->plant], "A healthy plant.");
        return;
    }

    /* Check for objects - inspect or pick up */
    if (tile->object != 0xFF) {
        draw_dialogue(object_names[tile->object - OBJ_FALLEN_LOG], "An object in the garden.");
        return;
    }

    /* Default: examine terrain */
    switch (tile->terrain) {
        case TERRAIN_GRASS: draw_dialogue("", "Tall grass sways gently."); break;
        case TERRAIN_SOIL: draw_dialogue("", "Bare earth, ready for planting."); break;
        case TERRAIN_FLOWER_BED: draw_dialogue("", "A prepared flower bed."); break;
        case TERRAIN_WOODLAND: draw_dialogue("", "Woodland floor, rich with leaf litter."); break;
        case TERRAIN_SHALLOW_WATER: draw_dialogue("", "Shallow water ripples softly."); break;
        case TERRAIN_DEEP_WATER: draw_dialogue("", "Deep, still water."); break;
        case TERRAIN_STONE: draw_dialogue("", "An old stone path."); break;
        default: break;
    }
}

void player_try_use_tool(void) {
    uint8_t tx, ty;
    /* Use tool on tile player is standing on or facing */
    get_front_tile(&tx, &ty);
    /* Actually use tile player is on for trowel/spade */
    tx = game.player.x / 8;
    ty = game.player.y / 8;

    if (tx >= map_w || ty >= map_h) return;
    if (!is_garden_area(game.player.area)) return;

    if (use_tool(game.equipped_tool, tx, ty)) {
        save_game();
    }
}
