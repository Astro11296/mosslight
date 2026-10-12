/*
 * Save system - uses GBC SRAM (battery-backed RAM)
 * MBC5 with 32KB RAM across 4 banks of 8KB
 * We use bank 0 for save data
 *
 * Reads/writes SRAM directly to avoid a large BSS buffer.
 */
#include "mosslight.h"
#include "data_tables.h"

#define SAVE_MAGIC 0x4D53  /* "MS" */
#define SAVE_VERSION 1

/* SRAM layout offsets */
#define SRM_MAGIC      0
#define SRM_VERSION    2
#define SRM_CHECKSUM   4
#define SRM_DAY        6
#define SRM_TIME       8
#define SRM_WEATHER    9
#define SRM_COINS     10
#define SRM_TOOL      12
#define SRM_TUT_STEP  13
#define SRM_TUT_DONE  14
#define SRM_ENDING    15
#define SRM_UNIQ_VIS  16
#define SRM_RESIDENTS 17
#define SRM_WL_DISC   18  /* WL_COUNT bytes */
#define SRM_WL_VIS    (SRM_WL_DISC + WL_COUNT)
#define SRM_PL_DISC   (SRM_WL_VIS + WL_COUNT)  /* PLANT_COUNT bytes */
#define SRM_UNLOCKED  (SRM_PL_DISC + PLANT_COUNT) /* AREA_COUNT bytes */
#define SRM_INVENTORY (SRM_UNLOCKED + AREA_COUNT)  /* ITEM_COUNT bytes */
#define SRM_P_AREA    (SRM_INVENTORY + ITEM_COUNT)
#define SRM_P_X       (SRM_P_AREA + 1)
#define SRM_P_Y       (SRM_P_X + 2)
#define SRM_P_DIR     (SRM_P_Y + 2)
#define SRM_AREA      (SRM_P_DIR + 1)
#define SRM_MAP_DATA  (SRM_AREA + 1)  /* map_h * map_w * 2 bytes */

#define SRAM_BASE ((volatile uint8_t*)0xA000)

static void sram_write(uint16_t off, uint8_t val) { SRAM_BASE[off] = val; }
static uint8_t sram_read(uint16_t off) { return SRAM_BASE[off]; }

static void sram_write16(uint16_t off, uint16_t val) {
    SRAM_BASE[off] = val & 0xFF;
    SRAM_BASE[off + 1] = val >> 8;
}
static uint16_t sram_read16(uint16_t off) {
    return SRAM_BASE[off] | (SRAM_BASE[off + 1] << 8);
}

static uint8_t pack_plant(uint8_t plant, uint8_t growth) {
    if (plant == 0xFF) return 0xFF;
    return (plant & 0x3F) | ((growth & 0x03) << 6);
}

static void unpack_plant(uint8_t packed, uint8_t *plant, uint8_t *growth) {
    if (packed == 0xFF) {
        *plant = 0xFF;
        *growth = STAGE_NONE;
    } else {
        *plant = packed & 0x3F;
        *growth = (packed >> 6) & 0x03;
    }
}

static uint16_t calc_sram_checksum(void) {
    uint16_t sum = 0;
    /* Checksum covers bytes 0-3 and 6 onward */
    for (uint16_t i = 0; i < 4; i++) sum += sram_read(i);
    /* Read up to end of header + map data (skip checksum at 4-5) */
    uint16_t end = SRM_MAP_DATA + (uint16_t)map_w * map_h * 2;
    if (end > 0x2000) end = 0x2000;
    for (uint16_t i = 6; i < end; i++) sum += sram_read(i);
    return sum;
}

void save_game(void) {
    ENABLE_RAM;

    sram_write16(SRM_MAGIC, SAVE_MAGIC);
    sram_write16(SRM_VERSION, SAVE_VERSION);
    sram_write16(SRM_DAY, game.day);
    sram_write(SRM_TIME, (uint8_t)game.time);
    sram_write(SRM_WEATHER, (uint8_t)game.weather);
    sram_write16(SRM_COINS, game.coins);
    sram_write(SRM_TOOL, game.equipped_tool);
    sram_write(SRM_TUT_STEP, game.tutorial_step);
    sram_write(SRM_TUT_DONE, game.tutorial_done);
    sram_write(SRM_ENDING, game.ending_triggered);
    sram_write(SRM_UNIQ_VIS, game.unique_visitors);
    sram_write(SRM_RESIDENTS, game.residents);

    uint16_t off = SRM_WL_DISC;
    for (uint8_t i = 0; i < WL_COUNT; i++) sram_write(off++, game.wildlife_disc[i]);
    off = SRM_WL_VIS;
    for (uint8_t i = 0; i < WL_COUNT; i++) sram_write(off++, game.wildlife_visits[i]);
    off = SRM_PL_DISC;
    for (uint8_t i = 0; i < PLANT_COUNT; i++) sram_write(off++, game.plant_disc[i]);
    off = SRM_UNLOCKED;
    for (uint8_t i = 0; i < AREA_COUNT; i++) sram_write(off++, game.unlocked[i]);
    off = SRM_INVENTORY;
    for (uint8_t i = 0; i < ITEM_COUNT; i++) sram_write(off++, game.inventory[i]);

    sram_write(SRM_P_AREA, game.player.area);
    sram_write16(SRM_P_X, game.player.x);
    sram_write16(SRM_P_Y, game.player.y);
    sram_write(SRM_P_DIR, game.player.direction);
    sram_write(SRM_AREA, game.player.area);

    uint16_t moff = SRM_MAP_DATA;
    for (uint8_t y = 0; y < map_h; y++) {
        for (uint8_t x = 0; x < map_w; x++) {
            sram_write(moff++, current_map[y][x].terrain);
            sram_write(moff++, pack_plant(current_map[y][x].plant,
                                          current_map[y][x].growth));
        }
    }

    sram_write16(SRM_CHECKSUM, calc_sram_checksum());

    DISABLE_RAM;
}

bool load_game(void) {
    ENABLE_RAM;

    if (sram_read16(SRM_MAGIC) != SAVE_MAGIC) { DISABLE_RAM; return false; }
    if (sram_read16(SRM_VERSION) != SAVE_VERSION) { DISABLE_RAM; return false; }
    uint16_t cs = calc_sram_checksum();
    if (cs != sram_read16(SRM_CHECKSUM)) { DISABLE_RAM; return false; }

    game.day = sram_read16(SRM_DAY);
    game.time = (TimePeriod)sram_read(SRM_TIME);
    game.weather = (Weather)sram_read(SRM_WEATHER);
    game.coins = sram_read16(SRM_COINS);
    game.equipped_tool = sram_read(SRM_TOOL);
    game.tutorial_step = sram_read(SRM_TUT_STEP);
    game.tutorial_done = sram_read(SRM_TUT_DONE);
    game.ending_triggered = sram_read(SRM_ENDING);
    game.unique_visitors = sram_read(SRM_UNIQ_VIS);
    game.residents = sram_read(SRM_RESIDENTS);

    uint16_t off = SRM_WL_DISC;
    for (uint8_t i = 0; i < WL_COUNT; i++) game.wildlife_disc[i] = sram_read(off++);
    off = SRM_WL_VIS;
    for (uint8_t i = 0; i < WL_COUNT; i++) game.wildlife_visits[i] = sram_read(off++);
    off = SRM_PL_DISC;
    for (uint8_t i = 0; i < PLANT_COUNT; i++) game.plant_disc[i] = sram_read(off++);
    off = SRM_UNLOCKED;
    for (uint8_t i = 0; i < AREA_COUNT; i++) game.unlocked[i] = sram_read(off++);
    off = SRM_INVENTORY;
    for (uint8_t i = 0; i < ITEM_COUNT; i++) game.inventory[i] = sram_read(off++);

    uint8_t area = sram_read(SRM_AREA);
    load_area(area);

    uint16_t moff = SRM_MAP_DATA;
    for (uint8_t y = 0; y < map_h; y++) {
        for (uint8_t x = 0; x < map_w; x++) {
            current_map[y][x].terrain = sram_read(moff++);
            uint8_t plant, growth;
            unpack_plant(sram_read(moff++), &plant, &growth);
            current_map[y][x].plant = plant;
            current_map[y][x].growth = growth;
            current_map[y][x].object = 0xFF;
            current_map[y][x].watered = true;
            current_map[y][x].growth_timer = 0;
        }
    }

    game.player.x = sram_read16(SRM_P_X);
    game.player.y = sram_read16(SRM_P_Y);
    game.player.area = sram_read(SRM_P_AREA);
    game.player.direction = sram_read(SRM_P_DIR);
    game.player.moving = false;

    DISABLE_RAM;
    return true;
}

bool has_save(void) {
    ENABLE_RAM;
    uint16_t magic = sram_read16(SRM_MAGIC);
    DISABLE_RAM;
    return magic == SAVE_MAGIC;
}
