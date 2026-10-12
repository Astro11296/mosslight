#ifndef MOSSLIGHT_H
#define MOSSLIGHT_H

#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*===========================================================================
 * Mosslight - Game Boy Color ROM
 * A relaxing wildlife garden game built with GBDK-2020
 *=========================================================================*/

/* ---- Display ---- */
#define SCREEN_W 160
#define SCREEN_H 144
#define SCREEN_TW 20
#define SCREEN_TH 18

/* ---- Map ---- */
#define MAX_MAP_W 24
#define MAX_MAP_H 24

/* ---- Game states ---- */
typedef enum {
    STATE_TITLE,
    STATE_OVERWORLD,
    STATE_MENU,
    STATE_FIELD_GUIDE,
    STATE_SHOP,
    STATE_DIALOGUE,
    STATE_PLACEMENT,
    STATE_ENDING,
    STATE_CREDITS,
} GameState;

/* ---- Time ---- */
typedef enum { TIME_MORNING, TIME_DAY, TIME_EVENING, TIME_NIGHT } TimePeriod;
#define TIME_DURATION 360 /* 6 min per period at 60fps = 21600 frames */

/* ---- Weather ---- */
typedef enum { WEATHER_CLEAR, WEATHER_CLOUDY, WEATHER_RAIN } Weather;

/* ---- Terrain ---- */
typedef enum {
    TERRAIN_GRASS, TERRAIN_SOIL, TERRAIN_FLOWER_BED, TERRAIN_WOODLAND,
    TERRAIN_SHALLOW_WATER, TERRAIN_DEEP_WATER, TERRAIN_STONE,
    TERRAIN_WALL, TERRAIN_ROOF, TERRAIN_TREE_TRUNK, TERRAIN_COUNT
} TerrainType;

/* ---- Habitat tags ---- */
typedef enum {
    TAG_FLOWERS=0, TAG_NECTAR, TAG_SEEDS, TAG_FRUIT, TAG_GRASSLAND,
    TAG_WATER, TAG_WETLAND, TAG_CANOPY, TAG_WOODLAND, TAG_SHELTER,
    TAG_DECAY, TAG_NIGHT_BLOOM, TAG_COUNT
} HabitatTag;

/* ---- Growth ---- */
typedef enum { STAGE_NONE, STAGE_NEWLY, STAGE_YOUNG, STAGE_MATURE } GrowthStage;

/* ---- Tools ---- */
typedef enum { TOOL_TROWEL, TOOL_SPADE, TOOL_WATERING_CAN, TOOL_COUNT } ToolType;

/* ---- Areas ---- */
typedef enum {
    AREA_MAIN_GARDEN, AREA_POND_EDGE, AREA_OLD_GROVE, AREA_MOON_MEADOW,
    AREA_GLOAMWICK, AREA_TRAIL, AREA_COUNT
} AreaID;

/* ---- Discovery ---- */
typedef enum { DISC_UNKNOWN, DISC_VISITOR, DISC_RESIDENT } DiscoveryState;

/* ---- Direction ---- */
typedef enum { DIR_DOWN, DIR_UP, DIR_LEFT, DIR_RIGHT } Direction;

/* ---- Items (plants + objects) ---- */
typedef enum {
    /* Flowers 0-8 */
    PLANT_CONEFLOWER=0, PLANT_BLACK_EYED_SUSAN, PLANT_WILD_BERGAMOT,
    PLANT_GOLDENROD, PLANT_ASTER, PLANT_MILKWEED, PLANT_WOODLAND_VIOLET,
    PLANT_WOODLAND_PHLOX, PLANT_MOONFLOWER,
    /* Grasses 9-11 */
    PLANT_LITTLE_BLUESTEM, PLANT_SWITCHGRASS, PLANT_WOODLAND_SEDGE,
    /* Shrubs 12-13 */
    PLANT_SERVICEBERRY, PLANT_ELDERBERRY,
    /* Trees 14-15 */
    PLANT_OAK, PLANT_WILLOW,
    /* Aquatic 16-19 */
    PLANT_CATTAIL, PLANT_PICKERELWEED, PLANT_WATER_LILY, PLANT_MARSH_MARIGOLD,
    /* Fungi 20-21 */
    PLANT_SHELF_MUSHROOM, PLANT_GLOWCAP,
    PLANT_COUNT,
    /* Objects 22-33 */
    OBJ_FALLEN_LOG=22, OBJ_OLD_STUMP, OBJ_ROCK, OBJ_ROCK_PILE,
    OBJ_BRUSH_PILE, OBJ_BIRD_BATH, OBJ_BIRD_HOUSE, OBJ_BENCH,
    OBJ_LANTERN, OBJ_TRELLIS, OBJ_FOOTBRIDGE, OBJ_STEPPING_STONE,
    OBJ_COUNT,
    ITEM_COUNT = OBJ_COUNT
} ItemID;

/* ---- Wildlife ---- */
typedef enum {
    WL_HONEY_BEE=0, WL_BUMBLEBEE, WL_MONARCH, WL_SWALLOWTAIL,
    WL_LUNA_MOTH, WL_SPHINX_MOTH, WL_FIREFLIES, WL_DRAGONFLY,
    WL_LADY_BEETLE, WL_SNAIL, WL_TOAD, WL_TREE_FROG, WL_SALAMANDER,
    WL_GARTER_SNAKE, WL_COTTONTAIL, WL_CHIPMUNK, WL_SQUIRREL,
    WL_ROBIN, WL_CHICKADEE, WL_CARDINAL, WL_GOLDFINCH, WL_HUMMINGBIRD,
    WL_WOODPECKER, WL_BAT, WL_OWL, WL_RACCOON, WL_PAINTED_TURTLE,
    WL_HERON, WL_COUNT
} WildlifeID;

/* ---- Map tile ---- */
typedef struct {
    uint8_t terrain;
    uint8_t plant;    /* 0xFF = empty */
    uint8_t growth;
    uint8_t object;   /* 0xFF = empty */
    bool watered;
    uint8_t growth_timer;
} MapTile;

/* ---- Player ---- */
typedef struct {
    uint16_t x, y;
    uint8_t area;
    uint8_t direction;
    uint8_t frame;
    uint8_t frame_timer;
    bool moving;
    uint16_t target_x, target_y;
} Player;

/* ---- Wildlife entity ---- */
typedef struct {
    uint8_t id;
    int16_t x, y, tx, ty;
    uint8_t state, timer, frame, frame_timer;
    bool active;
    uint8_t dir;
} WildlifeEntity;

/* ---- Game data ---- */
typedef struct {
    GameState state, prev_state;
    TimePeriod time;
    Weather weather;
    uint16_t day;
    uint16_t time_timer;
    uint16_t coins;
    Player player;
    uint8_t inventory[ITEM_COUNT];
    uint8_t equipped_tool;
    uint8_t wildlife_disc[WL_COUNT];
    uint8_t wildlife_visits[WL_COUNT];
    bool plant_disc[PLANT_COUNT];
    uint8_t unique_visitors;
    uint8_t residents;
    bool unlocked[AREA_COUNT];
    uint8_t tutorial_step;
    bool tutorial_done;
    bool ending_triggered;
    bool rain_watered_today;
    uint8_t placing_item;
    WildlifeEntity wildlife[8];
    uint16_t spawn_timer;
    bool trail_generated;
} GameData;

/* ---- Globals ---- */
extern GameData game;
extern MapTile current_map[MAX_MAP_H][MAX_MAP_W];
extern uint8_t map_w, map_h;
extern uint16_t frame_count;

/* ---- Input ---- */
void input_update(void);
bool btn_pressed(uint8_t b);
bool btn_held(uint8_t b);
#define BTN_RIGHT 0x01
#define BTN_LEFT 0x02
#define BTN_UP 0x04
#define BTN_DOWN 0x08
#define BTN_A 0x10
#define BTN_B 0x20
#define BTN_START 0x80
#define BTN_SELECT 0x40

/* ---- Banner/dialogue ---- */
void draw_banner(const char *t, uint8_t d);
void update_banner(void);
bool is_banner_active(void);
const char* get_banner_text(void);
void draw_dialogue(const char *s, const char *t);
bool is_dialogue_active(void);
void close_dialogue(void);
const char* get_dlg_speaker(void);
const char* get_dlg_text(void);

/* ---- Cooldown ---- */
void set_menu_cooldown(uint8_t f);
bool is_menu_cooling_down(void);

/* ---- Engine ---- */
void update_overworld(void);
void draw_overworld(void);
void update_time(void);
void update_weather(void);

/* ---- Tiles ---- */
void load_tiles(void);

/* ---- Sprites ---- */
void load_sprites(void);
uint8_t get_wildlife_sprite(uint8_t id);
uint8_t get_player_sprite_base(uint8_t dir);

/* ---- Maps ---- */
void load_area(uint8_t area);
void draw_map(void);
bool can_walk(uint8_t tx, uint8_t ty);
bool is_garden_area(uint8_t area);

/* ---- Player ---- */
void player_init(void);
void player_update(void);
void player_draw(void);
void player_try_interact(void);
void player_try_use_tool(void);

/* ---- Garden ---- */
bool can_plant(uint8_t item, uint8_t tx, uint8_t ty);
bool plant_seed(uint8_t item, uint8_t tx, uint8_t ty);
bool use_tool(uint8_t tool, uint8_t tx, uint8_t ty);
bool can_place_object(uint8_t item, uint8_t tx, uint8_t ty);
bool place_object(uint8_t item, uint8_t tx, uint8_t ty);
void update_plant_growth(void);
void compute_habitat(uint8_t *tags);

/* ---- Wildlife ---- */
void wildlife_update(void);
void wildlife_draw(void);
void check_spawn(void);
bool check_wildlife_eligible(uint8_t id);
void try_make_resident(uint8_t id);

/* ---- Camera ---- */
uint16_t cam_x_get(void);
uint16_t cam_y_get(void);

/* ---- UI ---- */
void menu_update(void);
void menu_draw(void);
void fieldguide_update(void);
void fieldguide_draw(void);
void shop_update(void);
void shop_draw(void);

/* ---- Save ---- */
void save_game(void);
bool load_game(void);
bool has_save(void);

/* ---- Audio ---- */
void audio_init(void);
void play_music(uint8_t track);
void stop_music(void);
void play_sfx(uint8_t sfx);
void music_tick(void);
void start_ambient(uint8_t type);
void stop_ambient(void);
#define MUSIC_MORNING 0
#define MUSIC_DAY 1
#define MUSIC_GLOAMWICK 2
#define MUSIC_RAIN 3
#define MUSIC_EVENING 4
#define MUSIC_NIGHT 5
#define SFX_BLIP 0
#define SFX_CONFIRM 1
#define SFX_CANCEL 2
#define SFX_DISCOVER 3
#define SFX_PLANT 4
#define SFX_WATER 5
#define SFX_DIG 6
#define SFX_COIN 7

/* ---- Text ---- */
void load_font(void);
void draw_text(const char *s, uint8_t x, uint8_t y);
void draw_text_centered(const char *s, uint8_t y);
void draw_text_win(const char *s, uint8_t x, uint8_t y);

/* ---- Effects ---- */
void update_weather_effects(void);
void draw_weather_effects(void);

/* ---- Progression ---- */
void check_progression(void);

/* ---- Tutorial ---- */
void tutorial_update(void);

/* ---- Ending ---- */
void check_ending(void);
void ending_update(void);
void ending_draw(void);

/* ---- Trail ---- */
void generate_trail_items(void);

/* ---- Data ---- */
extern const char* plant_names[];
extern const char* wildlife_names[];
extern const char* object_names[];
extern const char* area_names[];
extern const char* time_names[];
extern const char* weather_names[];

#endif /* MOSSLIGHT_H */
