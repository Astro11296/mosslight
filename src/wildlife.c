/*
 * Wildlife - data-driven spawning and behavior
 */
#include "mosslight.h"
#include "data_tables.h"

extern void draw_banner(const char *t, uint8_t d);
extern void save_game(void);

/* Habitat cache */
static uint8_t habitat[TAG_COUNT];
static bool habitat_valid = false;

static void refresh_habitat(void) {
    if (!habitat_valid) {
        compute_habitat(habitat);
        habitat_valid = true;
    }
}

/* Count specific tiles/features for special requirements */
static uint16_t count_shallow_tiles(void) {
    uint16_t c = 0;
    for (uint8_t y = 0; y < map_h; y++)
        for (uint8_t x = 0; x < map_w; x++)
            if (current_map[y][x].terrain == TERRAIN_SHALLOW_WATER) c++;
    return c;
}

static uint16_t count_deep_tiles(void) {
    uint16_t c = 0;
    for (uint8_t y = 0; y < map_h; y++)
        for (uint8_t x = 0; x < map_w; x++)
            if (current_map[y][x].terrain == TERRAIN_DEEP_WATER) c++;
    return c;
}

static bool has_plant(uint8_t plant_id) {
    for (uint8_t y = 0; y < map_h; y++)
        for (uint8_t x = 0; x < map_w; x++)
            if (current_map[y][x].plant == plant_id &&
                current_map[y][x].growth >= STAGE_YOUNG)
                return true;
    return false;
}

static bool has_mature_plant(uint8_t plant_id) {
    for (uint8_t y = 0; y < map_h; y++)
        for (uint8_t x = 0; x < map_w; x++)
            if (current_map[y][x].plant == plant_id &&
                current_map[y][x].growth == STAGE_MATURE)
                return true;
    return false;
}

static bool has_object(uint8_t obj_id) {
    for (uint8_t y = 0; y < map_h; y++)
        for (uint8_t x = 0; x < map_w; x++)
            if (current_map[y][x].object == obj_id) return true;
    return false;
}

static uint8_t count_mature_plant(uint8_t plant_id) {
    uint8_t c = 0;
    for (uint8_t y = 0; y < map_h; y++)
        for (uint8_t x = 0; x < map_w; x++)
            if (current_map[y][x].plant == plant_id &&
                current_map[y][x].growth == STAGE_MATURE) c++;
    return c;
}

/* Check if a specific wildlife is eligible to spawn */
bool check_wildlife_eligible(uint8_t id) {
    if (id >= WL_COUNT) return false;
    const WildlifeDef *wd = &wildlife_defs[id];

    /* Check time */
    if (!(wd->time_mask & (1 << game.time))) return false;

    /* Check weather */
    if (!(wd->weather_mask & (1 << game.weather))) return false;

    refresh_habitat();

    /* Check each requirement */
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t tag = wd->req_tag[i];
        uint8_t need = wd->req_count[i];
        if (need == 0) continue;

        if (tag < TAG_COUNT) {
            /* Standard habitat tag */
            if (habitat[tag] < need) return false;
        } else {
            /* Special requirement */
            switch (tag) {
                case REQ_MILKWEED:
                    if (!has_plant(PLANT_MILKWEED)) return false;
                    break;
                case REQ_MATURE_TREE:
                    if (!has_mature_plant(PLANT_OAK) && !has_mature_plant(PLANT_WILLOW))
                        return false;
                    break;
                case REQ_SHALLOW_TILES:
                    if (count_shallow_tiles() < need) return false;
                    break;
                case REQ_DEEP_TILES:
                    if (count_deep_tiles() < need) return false;
                    break;
                case REQ_BIRD_HOUSE:
                    if (!has_object(OBJ_BIRD_HOUSE)) return false;
                    break;
                case REQ_FALLEN_LOG:
                    if (!has_object(OBJ_FALLEN_LOG)) return false;
                    break;
                case REQ_ROCK_PILE:
                    if (!has_object(OBJ_ROCK_PILE)) return false;
                    break;
                case REQ_STUMP:
                    if (!has_object(OBJ_OLD_STUMP)) return false;
                    break;
                case REQ_CATTAIL:
                    if (!has_plant(PLANT_CATTAIL)) return false;
                    break;
                case REQ_SERVICEBERRY:
                    if (!has_plant(PLANT_SERVICEBERRY)) return false;
                    break;
                case REQ_BERGAMOT:
                    if (!has_plant(PLANT_WILD_BERGAMOT)) return false;
                    break;
                case REQ_OAK:
                    if (!has_mature_plant(PLANT_OAK)) return false;
                    break;
                case REQ_ROCK:
                    if (!has_object(OBJ_ROCK) && !has_object(OBJ_ROCK_PILE))
                        return false;
                    break;
                default:
                    break;
            }
        }
    }

    /* Area-specific checks for rare wildlife */
    if (id == WL_OWL && !game.unlocked[AREA_OLD_GROVE]) return false;
    if (id == WL_HERON && !game.unlocked[AREA_POND_EDGE]) return false;

    return true;
}

/* Count active wildlife */
static uint8_t count_active_wildlife(void) {
    uint8_t c = 0;
    for (uint8_t i = 0; i < 8; i++)
        if (game.wildlife[i].active) c++;
    return c;
}

/* Find a free wildlife slot */
static int8_t find_free_slot(void) {
    for (uint8_t i = 0; i < 8; i++)
        if (!game.wildlife[i].active) return i;
    return -1;
}

/* Spawn a wildlife at a map edge */
static void spawn_wildlife(uint8_t id) {
    int8_t slot = find_free_slot();
    if (slot < 0) return;

    /* Determine max active based on progression */
    uint8_t max_active = 3;
    if (game.unique_visitors >= 6) max_active = 5;
    if (game.unique_visitors >= 12) max_active = 7;
    if (count_active_wildlife() >= max_active) return;

    WildlifeEntity *w = &game.wildlife[slot];
    w->id = id;
    w->active = true;
    w->state = 1; /* moving */
    w->timer = 0;
    w->frame = 0;
    w->frame_timer = 0;
    w->dir = DIR_DOWN;

    /* Enter from a random edge, away from player */
    uint8_t edge = (uint8_t)(frame_count + slot) & 3;
    switch (edge) {
        case 0: /* top */
            w->x = (uint8_t)(frame_count % map_w) * 8;
            w->y = 0;
            break;
        case 1: /* bottom */
            w->x = (uint8_t)(frame_count % map_w) * 8;
            w->y = (map_h - 1) * 8;
            break;
        case 2: /* left */
            w->x = 0;
            w->y = (uint8_t)(frame_count % map_h) * 8;
            break;
        default: /* right */
            w->x = (map_w - 1) * 8;
            w->y = (uint8_t)(frame_count % map_h) * 8;
            break;
    }

    w->tx = w->x;
    w->ty = w->y;

    /* Register as visitor if first time */
    if (game.wildlife_disc[id] == DISC_UNKNOWN) {
        game.wildlife_disc[id] = DISC_VISITOR;
        game.wildlife_visits[id]++;
        game.unique_visitors++;
        game.coins += 10;
        draw_banner(wildlife_names[id], 120);
        save_game();
    } else {
        game.wildlife_visits[id]++;
    }

    /* Check residency */
    try_make_resident(id);
}

/* Make resident if visit count met */
void try_make_resident(uint8_t id) {
    if (id >= WL_COUNT) return;
    if (game.wildlife_disc[id] != DISC_VISITOR) return;

    const WildlifeDef *wd = &wildlife_defs[id];
    if (game.wildlife_visits[id] >= wd->resident_visits) {
        /* Check special residency conditions */
        bool special_ok = true;

        /* Honey Bee: 6 flowers */
        if (id == WL_HONEY_BEE) {
            refresh_habitat();
            if (habitat[TAG_FLOWERS] < 6) special_ok = false;
        }
        /* Bumblebee: Bergamot or Coneflower present */
        if (id == WL_BUMBLEBEE) {
            if (!has_plant(PLANT_WILD_BERGAMOT) && !has_plant(PLANT_CONEFLOWER))
                special_ok = false;
        }
        /* Monarch: 2 mature Milkweed */
        if (id == WL_MONARCH) {
            if (count_mature_plant(PLANT_MILKWEED) < 2) special_ok = false;
        }
        /* Swallowtail: 8 flowers */
        if (id == WL_SWALLOWTAIL) {
            refresh_habitat();
            if (habitat[TAG_FLOWERS] < 8) special_ok = false;
        }
        /* Luna Moth: 5 night_bloom + mature Oak or Willow */
        if (id == WL_LUNA_MOTH) {
            refresh_habitat();
            if (habitat[TAG_NIGHT_BLOOM] < 5) special_ok = false;
            if (!has_mature_plant(PLANT_OAK) && !has_mature_plant(PLANT_WILLOW))
                special_ok = false;
        }
        /* Dragonfly: 6 water tiles */
        if (id == WL_DRAGONFLY) {
            if (count_shallow_tiles() + count_deep_tiles() < 6) special_ok = false;
        }
        /* Chickadee: Bird House */
        if (id == WL_CHICKADEE) {
            if (!has_object(OBJ_BIRD_HOUSE)) special_ok = false;
        }
        /* Cardinal: Serviceberry or Elderberry */
        if (id == WL_CARDINAL) {
            if (!has_plant(PLANT_SERVICEBERRY) && !has_plant(PLANT_ELDERBERRY))
                special_ok = false;
        }
        /* Hummingbird: Wild Bergamot */
        if (id == WL_HUMMINGBIRD) {
            if (!has_plant(PLANT_WILD_BERGAMOT)) special_ok = false;
        }
        /* Woodpecker: mature Oak + Stump */
        if (id == WL_WOODPECKER) {
            if (!has_mature_plant(PLANT_OAK) || !has_object(OBJ_OLD_STUMP))
                special_ok = false;
        }
        /* Salamander: Fallen Log + mature tree */
        if (id == WL_SALAMANDER) {
            if (!has_object(OBJ_FALLEN_LOG)) special_ok = false;
            if (!has_mature_plant(PLANT_OAK) && !has_mature_plant(PLANT_WILLOW))
                special_ok = false;
        }
        /* Toad: Fallen Log or Rock Pile */
        if (id == WL_TOAD) {
            if (!has_object(OBJ_FALLEN_LOG) && !has_object(OBJ_ROCK_PILE))
                special_ok = false;
        }
        /* Chipmunk: Fallen Log or Stump */
        if (id == WL_CHIPMUNK) {
            if (!has_object(OBJ_FALLEN_LOG) && !has_object(OBJ_OLD_STUMP))
                special_ok = false;
        }
        /* Garter Snake: Rock Pile */
        if (id == WL_GARTER_SNAKE) {
            if (!has_object(OBJ_ROCK_PILE)) special_ok = false;
        }
        /* Tree Frog: Cattail */
        if (id == WL_TREE_FROG) {
            if (!has_plant(PLANT_CATTAIL)) special_ok = false;
        }

        if (special_ok) {
            game.wildlife_disc[id] = DISC_RESIDENT;
            game.residents++;
            game.coins += 8;
            draw_banner("New Resident!", 120);
            save_game();
        }
    }
}

/* Try to spawn a new wildlife */
void check_spawn(void) {
    /* Invalidate habitat cache so it refreshes */
    habitat_valid = false;

    /* Build list of eligible species */
    uint8_t eligible[WL_COUNT];
    uint8_t eligible_count = 0;
    uint8_t weights[WL_COUNT];

    for (uint8_t i = 0; i < WL_COUNT; i++) {
        if (check_wildlife_eligible(i)) {
            eligible[eligible_count] = i;
            /* Residents get higher weight, rare species stay low */
            weights[eligible_count] = 10;
            if (game.wildlife_disc[i] == DISC_RESIDENT) weights[eligible_count] = 20;
            if (wildlife_defs[i].rarity == 1) weights[eligible_count] /= 2;
            if (wildlife_defs[i].rarity == 2) weights[eligible_count] /= 4;
            eligible_count++;
        }
    }

    if (eligible_count == 0) return;

    /* Weighted random selection */
    uint16_t total_weight = 0;
    for (uint8_t i = 0; i < eligible_count; i++)
        total_weight += weights[i];

    if (total_weight == 0) return;

    uint16_t r = (uint16_t)(frame_count * 31 + game.day * 17) % total_weight;
    uint16_t acc = 0;
    for (uint8_t i = 0; i < eligible_count; i++) {
        acc += weights[i];
        if (r < acc) {
            spawn_wildlife(eligible[i]);
            break;
        }
    }
}

/* Update all active wildlife */
void wildlife_update(void) {
    for (uint8_t i = 0; i < 8; i++) {
        WildlifeEntity *w = &game.wildlife[i];
        if (!w->active) continue;

        w->timer++;
        w->frame_timer++;
        if (w->frame_timer >= 15) {
            w->frame_timer = 0;
            w->frame = (w->frame + 1) & 1;
        }

        switch (w->state) {
            case 0: /* idle */
                if (w->timer > 60 + (frame_count + i * 7) % 60) {
                    w->state = 1;
                    w->timer = 0;
                    /* Pick new target */
                    w->tx = w->x + ((int8_t)((frame_count + i * 13) % 64) - 32);
                    w->ty = w->y + ((int8_t)((frame_count + i * 17) % 64) - 32);
                    if (w->tx < 0) w->tx = 0;
                    if (w->ty < 0) w->ty = 0;
                    if (w->tx > map_w * 8) w->tx = map_w * 8;
                    if (w->ty > map_h * 8) w->ty = map_h * 8;
                }
                break;

            case 1: /* moving */
                if (w->x < w->tx) w->x++;
                if (w->x > w->tx) w->x--;
                if (w->y < w->ty) w->y++;
                if (w->y > w->ty) w->y--;

                if (w->x == w->tx && w->y == w->ty) {
                    w->state = 0;
                    w->timer = 0;
                }

                /* Auto-exit after a while */
                if (w->timer > 600 + i * 50) {
                    w->state = 3;
                    w->timer = 0;
                }
                break;

            case 3: /* exit */
                /* Move toward nearest edge */
                if (w->x < map_w * 4) w->x -= 2;
                else w->x += 2;
                if (w->y < map_h * 4) w->y -= 2;
                else w->y += 2;

                if (w->x < -16 || w->x > map_w * 8 + 16 ||
                    w->y < -16 || w->y > map_h * 8 + 16) {
                    w->active = false;
                }
                break;
        }
    }
}

/* Draw all active wildlife as sprites */
void wildlife_draw(void) {
    extern uint16_t cam_x_get(void);
    extern uint16_t cam_y_get(void);
    uint16_t cx = cam_x_get();
    uint16_t cy = cam_y_get();

    /* Sprites 4-11 for wildlife (8 sprites, player uses 0-3) */
    for (uint8_t i = 0; i < 8; i++) {
        WildlifeEntity *w = &game.wildlife[i];
        uint8_t sprite_idx = 4 + i;

        if (!w->active) {
            /* Hide sprite */
            move_sprite(sprite_idx, 0, 0 + SCREEN_H);
            continue;
        }

        int16_t sx = w->x - cx;
        int16_t sy = w->y - cy;

        /* Cull if off-screen */
        if (sx < -8 || sx > SCREEN_W || sy < -8 || sy > SCREEN_H) {
            move_sprite(sprite_idx, 0, 0 + SCREEN_H);
            continue;
        }

        uint8_t tile = get_wildlife_sprite(w->id);
        set_sprite_tile(sprite_idx, tile);

        /* Choose palette based on wildlife type */
        uint8_t pal = wildlife_defs[w->id].sprite_pal & 3;
        set_sprite_prop(sprite_idx, pal);

        /* Fireflies blink at night */
        if (w->id == WL_FIREFLIES) {
            if ((frame_count >> 3) & 1) {
                move_sprite(sprite_idx, 0, 0 + SCREEN_H);
                continue;
            }
        }

        move_sprite(sprite_idx, sx, sy);
    }
}
