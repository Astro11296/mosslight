/*
 * Garden - tools, terrain conversion, planting, growth, habitat computation
 */
#include "mosslight.h"
#include "data_tables.h"

extern void draw_banner(const char *t, uint8_t d);
extern void save_game(void);

/* Check if tile has plant or object blocking tool use */
static bool tile_occupied(uint8_t tx, uint8_t ty) {
    if (tx >= map_w || ty >= map_h) return true;
    return current_map[ty][tx].plant != 0xFF ||
           current_map[ty][tx].object != 0xFF;
}

bool can_plant(uint8_t item, uint8_t tx, uint8_t ty) {
    if (tx >= map_w || ty >= map_h) return false;
    if (item >= PLANT_COUNT) return false;
    if (tile_occupied(tx, ty)) return false;

    uint8_t required_terrain = plant_defs[item].terrain;
    uint8_t actual = current_map[ty][tx].terrain;

    /* Serviceberry and Elderberry can go on grass or woodland */
    if (item == PLANT_SERVICEBERRY || item == PLANT_ELDERBERRY) {
        return actual == TERRAIN_GRASS || actual == TERRAIN_WOODLAND;
    }
    /* Oak can go on grass or woodland */
    if (item == PLANT_OAK) {
        return actual == TERRAIN_GRASS || actual == TERRAIN_WOODLAND;
    }
    /* Water Lily can go on shallow or deep water */
    if (item == PLANT_WATER_LILY) {
        return actual == TERRAIN_SHALLOW_WATER || actual == TERRAIN_DEEP_WATER;
    }

    return actual == required_terrain;
}

bool plant_seed(uint8_t item, uint8_t tx, uint8_t ty) {
    if (!can_plant(item, tx, ty)) return false;
    if (game.inventory[item] == 0) return false;

    game.inventory[item]--;
    current_map[ty][tx].plant = item;
    current_map[ty][tx].growth = STAGE_NEWLY;
    current_map[ty][tx].growth_timer = 0;
    current_map[ty][tx].watered = false;

    /* Mark plant as discovered */
    if (!game.plant_disc[item]) {
        game.plant_disc[item] = true;
        game.coins += 3;
        draw_banner("New Plant Found!", 120);
    }

    play_sfx(SFX_PLANT);
    save_game();
    return true;
}

bool use_tool(uint8_t tool, uint8_t tx, uint8_t ty) {
    if (tx >= map_w || ty >= map_h) return false;
    if (!is_garden_area(game.player.area)) return false;

    uint8_t terrain = current_map[ty][tx].terrain;

    switch (tool) {
        case TOOL_TROWEL:
            if (tile_occupied(tx, ty)) return false;
            switch (terrain) {
                case TERRAIN_GRASS:
                    current_map[ty][tx].terrain = TERRAIN_SOIL;
                    play_sfx(SFX_DIG);
                    return true;
                case TERRAIN_SOIL:
                    current_map[ty][tx].terrain = TERRAIN_FLOWER_BED;
                    play_sfx(SFX_DIG);
                    return true;
                case TERRAIN_FLOWER_BED:
                    current_map[ty][tx].terrain = TERRAIN_GRASS;
                    play_sfx(SFX_DIG);
                    return true;
                default:
                    return false;
            }

        case TOOL_SPADE:
            if (tile_occupied(tx, ty)) return false;
            switch (terrain) {
                case TERRAIN_GRASS:
                case TERRAIN_SOIL:
                    current_map[ty][tx].terrain = TERRAIN_SHALLOW_WATER;
                    play_sfx(SFX_DIG);
                    return true;
                case TERRAIN_SHALLOW_WATER:
                    current_map[ty][tx].terrain = TERRAIN_DEEP_WATER;
                    play_sfx(SFX_WATER);
                    return true;
                case TERRAIN_DEEP_WATER:
                    current_map[ty][tx].terrain = TERRAIN_SHALLOW_WATER;
                    play_sfx(SFX_WATER);
                    return true;
                default:
                    return false;
            }

        case TOOL_WATERING_CAN:
            if (current_map[ty][tx].plant != 0xFF &&
                current_map[ty][tx].growth == STAGE_NEWLY &&
                !current_map[ty][tx].watered) {
                current_map[ty][tx].watered = true;
                play_sfx(SFX_WATER);
                return true;
            }
            return false;

        default:
            return false;
    }
}

bool can_place_object(uint8_t item, uint8_t tx, uint8_t ty) {
    if (tx >= map_w || ty >= map_h) return false;
    if (item < OBJ_FALLEN_LOG || item >= OBJ_COUNT) return false;
    if (tile_occupied(tx, ty)) return false;

    uint8_t terrain = current_map[ty][tx].terrain;
    /* Can't place on deep water, walls, or roofs */
    if (terrain == TERRAIN_DEEP_WATER || terrain == TERRAIN_WALL ||
        terrain == TERRAIN_ROOF) return false;

    /* Footbridge goes on shallow water */
    if (item == OBJ_FOOTBRIDGE) {
        return terrain == TERRAIN_SHALLOW_WATER;
    }

    return true;
}

bool place_object(uint8_t item, uint8_t tx, uint8_t ty) {
    if (!can_place_object(item, tx, ty)) return false;
    if (game.inventory[item] == 0) return false;

    game.inventory[item]--;
    current_map[ty][tx].object = item;
    play_sfx(SFX_CONFIRM);
    save_game();
    return true;
}

/* Update plant growth - called each frame but only advances on time period change */
static uint8_t last_period = 0xFF;

void update_plant_growth(void) {
    /* Check if time period changed */
    if (last_period == 0xFF) {
        last_period = game.time;
        return;
    }
    if (last_period == game.time) return;

    last_period = game.time;

    /* Advance growth for all plants in current area */
    for (uint8_t y = 0; y < map_h; y++) {
        for (uint8_t x = 0; x < map_w; x++) {
            if (current_map[y][x].plant != 0xFF &&
                current_map[y][x].growth < STAGE_MATURE) {

                /* Rain auto-waters */
                if (game.weather == WEATHER_RAIN && !current_map[y][x].watered) {
                    current_map[y][x].watered = true;
                }

                /* Only grow if watered */
                if (current_map[y][x].watered) {
                    current_map[y][x].growth_timer++;
                    uint8_t plant = current_map[y][x].plant;
                    if (plant < PLANT_COUNT) {
                        uint8_t threshold = plant_defs[plant].growth_periods;
                        if (current_map[y][x].growth_timer >= threshold) {
                            current_map[y][x].growth++;
                            current_map[y][x].growth_timer = 0;
                        }
                    }
                }
            }
        }
    }
}

/* Compute habitat tags for the current area */
void compute_habitat(uint8_t *tags) {
    memset(tags, 0, TAG_COUNT);

    for (uint8_t y = 0; y < map_h; y++) {
        for (uint8_t x = 0; x < map_w; x++) {
            MapTile *t = &current_map[y][x];

            /* Only mature plants contribute */
            if (t->plant != 0xFF && t->growth == STAGE_MATURE && t->plant < PLANT_COUNT) {
                const PlantDef *pd = &plant_defs[t->plant];
                /* Add tags from plant */
                for (uint8_t i = 0; i < 8; i++) {
                    if (pd->tags & (1 << i)) tags[i]++;
                }
                for (uint8_t i = 0; i < 5; i++) {
                    if (pd->tags2 & (1 << i)) tags[i + 8]++;
                }
            }

            /* Objects contribute tags */
            if (t->object != 0xFF && t->object >= OBJ_FALLEN_LOG) {
                uint8_t oi = t->object - OBJ_FALLEN_LOG;
                if (oi < (sizeof(object_defs)/sizeof(ObjectDef))) {
                    const ObjectDef *od = &object_defs[oi];
                    for (uint8_t i = 0; i < 8; i++) {
                        if (od->tags & (1 << i)) tags[i]++;
                    }
                    for (uint8_t i = 0; i < 5; i++) {
                        if (od->tags2 & (1 << i)) tags[i + 8]++;
                    }
                }
            }

            /* Terrain contributes water tags */
            if (t->terrain == TERRAIN_SHALLOW_WATER) {
                tags[TAG_WATER]++;
                tags[TAG_WETLAND]++;
            }
            if (t->terrain == TERRAIN_DEEP_WATER) {
                tags[TAG_WATER]++;
            }
        }
    }
}
