/*
 * Maps - tile-based area loading and rendering
 * Areas are generated programmatically based on area type
 */
#include "mosslight.h"
#include "data_tables.h"

/* Simple sine approximation for trail winding */
static const int8_t sin_table[] = {0,1,2,3,3,3,2,1,0,-1,-2,-3,-3,-3,-2,-1};

/* Tile IDs for rendering */
#define T_BLANK     0
#define T_GRASS     1
#define T_GRASS2    2
#define T_SOIL      3
#define T_FLOWERBED 4
#define T_WOODLAND  5
#define T_SHALLOW   6
#define T_DEEP      7
#define T_STONE     8
#define T_WALL      9
#define T_ROOF     10
#define T_TRUNK    11
#define T_CANOPY_TL 12
#define T_CANOPY_TR 13
#define T_CANOPY_BL 14
#define T_CANOPY_BR 15
#define T_SPROUT   16
#define T_FLOWER_Y 17
#define T_FLOWER_M 18
#define T_RIPPLE   19

/* Area sizes (in tiles) */
static const uint8_t area_sizes[][2] = {
    {24, 24}, /* Main Garden */
    {20, 20}, /* Pond Edge */
    {20, 20}, /* Old Grove */
    {20, 20}, /* Moon Meadow */
    {24, 16}, /* Gloamwick */
    {24, 16}, /* Woodland Trail */
};

/* Draw map cache - tracks last camera tile position to avoid per-frame redraws */
static uint8_t last_cam_tx = 0xFF;
static uint8_t last_cam_ty = 0xFF;

bool is_garden_area(uint8_t area) {
    return area <= AREA_MOON_MEADOW;
}

static uint8_t terrain_tile(uint8_t t) {
    switch (t) {
        case TERRAIN_GRASS: return T_GRASS;
        case TERRAIN_SOIL: return T_SOIL;
        case TERRAIN_FLOWER_BED: return T_FLOWERBED;
        case TERRAIN_WOODLAND: return T_WOODLAND;
        case TERRAIN_SHALLOW_WATER: return T_SHALLOW;
        case TERRAIN_DEEP_WATER: return T_DEEP;
        case TERRAIN_STONE: return T_STONE;
        case TERRAIN_WALL: return T_WALL;
        case TERRAIN_ROOF: return T_ROOF;
        case TERRAIN_TREE_TRUNK: return T_TRUNK;
        default: return T_GRASS;
    }
}

/* Get plant render tile based on growth stage */
static uint8_t plant_tile(uint8_t plant_id, uint8_t growth) {
    if (growth == STAGE_NEWLY) return T_SPROUT;
    if (growth == STAGE_YOUNG) return T_FLOWER_Y;
    /* Mature - could vary by plant type but we keep it simple */
    return T_FLOWER_M;
}

void load_area(uint8_t area) {
    map_w = area_sizes[area][0];
    map_h = area_sizes[area][1];
    if (map_w > MAX_MAP_W) map_w = MAX_MAP_W;
    if (map_h > MAX_MAP_H) map_h = MAX_MAP_H;

    /* Initialize all tiles */
    for (uint8_t y = 0; y < map_h; y++) {
        for (uint8_t x = 0; x < map_w; x++) {
            current_map[y][x].terrain = TERRAIN_GRASS;
            current_map[y][x].plant = 0xFF;
            current_map[y][x].growth = STAGE_NONE;
            current_map[y][x].object = 0xFF;
            current_map[y][x].watered = false;
            current_map[y][x].growth_timer = 0;
        }
    }

    /* Generate area-specific terrain */
    switch (area) {
        case AREA_MAIN_GARDEN:
            /* Cottage in top-left area */
            for (uint8_t x = 2; x < 7; x++) {
                for (uint8_t y = 2; y < 5; y++) {
                    current_map[y][x].terrain = TERRAIN_WALL;
                }
            }
            /* Roof */
            for (uint8_t x = 1; x < 8; x++) {
                current_map[1][x].terrain = TERRAIN_ROOF;
            }
            /* Path from cottage */
            for (uint8_t y = 5; y < 15; y++) {
                current_map[y][4].terrain = TERRAIN_STONE;
            }
            /* Some pre-existing flower beds */
            current_map[10][10].terrain = TERRAIN_FLOWER_BED;
            current_map[10][12].terrain = TERRAIN_FLOWER_BED;
            current_map[12][10].terrain = TERRAIN_FLOWER_BED;
            /* Some woodland in corner */
            for (uint8_t x = 18; x < 24; x++) {
                for (uint8_t y = 18; y < 24; y++) {
                    current_map[y][x].terrain = TERRAIN_WOODLAND;
                }
            }
            /* Stone border on right edge */
            for (uint8_t y = 0; y < map_h; y++) {
                current_map[y][map_w-1].terrain = TERRAIN_STONE;
            }
            break;

        case AREA_POND_EDGE:
            /* Central pond */
            for (uint8_t y = 8; y < 16; y++) {
                for (uint8_t x = 8; x < 16; x++) {
                    if (y >= 10 && y <= 14 && x >= 10 && x <= 14) {
                        current_map[y][x].terrain = TERRAIN_DEEP_WATER;
                    } else {
                        current_map[y][x].terrain = TERRAIN_SHALLOW_WATER;
                    }
                }
            }
            /* Grass around pond */
            /* Some woodland on edges */
            for (uint8_t x = 0; x < 5; x++) {
                for (uint8_t y = 0; y < 5; y++) {
                    current_map[y][x].terrain = TERRAIN_WOODLAND;
                }
            }
            break;

        case AREA_OLD_GROVE:
            /* Dense woodland */
            for (uint8_t y = 0; y < map_h; y++) {
                for (uint8_t x = 0; x < map_w; x++) {
                    current_map[y][x].terrain = TERRAIN_WOODLAND;
                }
            }
            /* Clearing in center */
            for (uint8_t y = 10; y < 16; y++) {
                for (uint8_t x = 10; x < 16; x++) {
                    current_map[y][x].terrain = TERRAIN_GRASS;
                }
            }
            /* Small pond */
            current_map[18][5].terrain = TERRAIN_SHALLOW_WATER;
            current_map[18][6].terrain = TERRAIN_SHALLOW_WATER;
            current_map[19][5].terrain = TERRAIN_SHALLOW_WATER;
            current_map[19][6].terrain = TERRAIN_DEEP_WATER;
            break;

        case AREA_MOON_MEADOW:
            /* Open grass with stone ruins */
            for (uint8_t y = 0; y < map_h; y++) {
                for (uint8_t x = 0; x < map_w; x++) {
                    current_map[y][x].terrain = TERRAIN_GRASS;
                }
            }
            /* Ruined stone border */
            for (uint8_t y = 5; y < 20; y += 3) {
                current_map[y][5].terrain = TERRAIN_STONE;
                current_map[y][18].terrain = TERRAIN_STONE;
            }
            /* Woodland patches */
            for (uint8_t x = 20; x < 24; x++) {
                for (uint8_t y = 2; y < 8; y++) {
                    current_map[y][x].terrain = TERRAIN_WOODLAND;
                }
            }
            break;

        case AREA_GLOAMWICK:
            /* Town with buildings and paths */
            for (uint8_t y = 0; y < map_h; y++) {
                for (uint8_t x = 0; x < map_w; x++) {
                    current_map[y][x].terrain = TERRAIN_GRASS;
                }
            }
            /* Main path */
            for (uint8_t x = 0; x < map_w; x++) {
                current_map[8][x].terrain = TERRAIN_STONE;
            }
            /* Shop building (left) */
            for (uint8_t x = 4; x < 9; x++) {
                for (uint8_t y = 4; y < 7; y++) {
                    current_map[y][x].terrain = TERRAIN_WALL;
                }
            }
            for (uint8_t x = 3; x < 10; x++) {
                current_map[3][x].terrain = TERRAIN_ROOF;
            }
            /* Natural history room (right) */
            for (uint8_t x = 20; x < 25; x++) {
                for (uint8_t y = 4; y < 7; y++) {
                    current_map[y][x].terrain = TERRAIN_WALL;
                }
            }
            for (uint8_t x = 19; x < 26; x++) {
                current_map[3][x].terrain = TERRAIN_ROOF;
            }
            break;

        case AREA_TRAIL:
            /* Winding trail through woods */
            for (uint8_t y = 0; y < map_h; y++) {
                for (uint8_t x = 0; x < map_w; x++) {
                    current_map[y][x].terrain = TERRAIN_WOODLAND;
                }
            }
            /* Trail path winding */
            for (uint8_t x = 0; x < map_w; x++) {
                uint8_t py = 8 + sin_table[x & 15];
                if (py >= map_h) py = map_h - 1;
                current_map[py][x].terrain = TERRAIN_STONE;
                if (py + 1 < map_h) current_map[py+1][x].terrain = TERRAIN_STONE;
            }
            break;
    }

    game.player.area = area;

    /* Invalidate draw_map cache so it redraws after area change */
    last_cam_tx = 0xFF;
    last_cam_ty = 0xFF;
}

/* Camera position getters - declared early for use in draw_map */
uint16_t cam_x_get(void);
uint16_t cam_y_get(void);

/* Check if a tile is walkable */
bool can_walk(uint8_t tx, uint8_t ty) {
    if (tx >= map_w || ty >= map_h) return false;
    uint8_t t = current_map[ty][tx].terrain;
    /* Walls, roofs, deep water are not walkable */
    if (t == TERRAIN_WALL || t == TERRAIN_ROOF) return false;
    if (t == TERRAIN_DEEP_WATER) return false;
    /* Trees/trunks block */
    if (t == TERRAIN_TREE_TRUNK) return false;
    /* Objects block (except decorations you can walk over like stepping stones) */
    if (current_map[ty][tx].object != 0xFF) {
        uint8_t obj = current_map[ty][tx].object;
        if (obj != OBJ_STEPPING_STONE) return false;
    }
    return true;
}

/* Render the visible portion of the map to the BG */
void draw_map(void) {
    uint8_t start_tx = cam_x_get() / 8;
    uint8_t start_ty = cam_y_get() / 8;

    /* Only redraw if camera moved to a new tile position */
    if (start_tx == last_cam_tx && start_ty == last_cam_ty) return;
    last_cam_tx = start_tx;
    last_cam_ty = start_ty;

    for (uint8_t dy = 0; dy < SCREEN_TH; dy++) {
        for (uint8_t dx = 0; dx < SCREEN_TW; dx++) {
            uint8_t tx = start_tx + dx;
            uint8_t ty = start_ty + dy;
            uint8_t tile = T_GRASS;

            if (tx < map_w && ty < map_h) {
                MapTile *mt = &current_map[ty][tx];
                tile = terrain_tile(mt->terrain);

                /* Water animation */
                if (mt->terrain == TERRAIN_SHALLOW_WATER || mt->terrain == TERRAIN_DEEP_WATER) {
                    if ((frame_count >> 4) & 1) tile = T_RIPPLE;
                }

                /* Plant rendering */
                if (mt->plant != 0xFF && mt->growth > STAGE_NONE) {
                    tile = plant_tile(mt->plant, mt->growth);
                }

                /* Tree rendering (2x2 canopy) */
                if (mt->plant == PLANT_OAK || mt->plant == PLANT_WILLOW) {
                    if (mt->growth == STAGE_MATURE) {
                        /* We render trees as canopy tiles */
                        tile = T_CANOPY_TL + (dx & 1) + ((dy & 1) * 2);
                    } else {
                        tile = plant_tile(mt->plant, mt->growth);
                    }
                }

                /* Object rendering - for now use stone as placeholder */
                if (mt->object != 0xFF) {
                    /* Objects render as special tiles, using stone for now */
                    tile = T_STONE;
                }
            }

            set_bkg_tile_xy(dx, dy, tile);
        }
    }

    /* Set CGB BG palette attributes for visible tiles */
    VBK_REG = 1;
    for (uint8_t dy = 0; dy < SCREEN_TH; dy++) {
        for (uint8_t dx = 0; dx < SCREEN_TW; dx++) {
            uint8_t tx = start_tx + dx;
            uint8_t ty = start_ty + dy;
            uint8_t pal = 0; /* default grass palette */

            if (tx < map_w && ty < map_h) {
                MapTile *mt = &current_map[ty][tx];
                uint8_t t = mt->terrain;
                if (t == TERRAIN_SOIL || t == TERRAIN_FLOWER_BED)
                    pal = 1;
                else if (t == TERRAIN_SHALLOW_WATER || t == TERRAIN_DEEP_WATER)
                    pal = 2;
                else if (t == TERRAIN_WALL || t == TERRAIN_ROOF || t == TERRAIN_STONE)
                    pal = 3;
                else
                    pal = 0;
            }
            set_bkg_tile_xy(dx, dy, pal);
        }
    }
    VBK_REG = 0;
}

/* Camera position getters */
extern uint16_t cam_x, cam_y;
uint16_t cam_x_get(void) { return cam_x; }
uint16_t cam_y_get(void) { return cam_y; }
