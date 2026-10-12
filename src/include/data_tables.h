#ifndef DATA_TABLES_H
#define DATA_TABLES_H

#include "mosslight.h"

/* Plant definition: terrain, tags (2 bytes), growth periods */
typedef struct {
    uint8_t terrain;
    uint8_t tags;   /* bits 0-7: FLOWERS|NECTAR|SEEDS|FRUIT|GRASSLAND|WATER|WETLAND */
    uint8_t tags2;  /* bits 0-4: CANOPY|WOODLAND|SHELTER|DECAY|NIGHT_BLOOM */
    uint8_t growth_periods;
} PlantDef;

#define TF_FLOWERS     (1<<0)
#define TF_NECTAR      (1<<1)
#define TF_SEEDS       (1<<2)
#define TF_FRUIT       (1<<3)
#define TF_GRASSLAND   (1<<4)
#define TF_WATER       (1<<5)
#define TF_WETLAND     (1<<6)
#define TF2_CANOPY     (1<<0)
#define TF2_WOODLAND   (1<<1)
#define TF2_SHELTER    (1<<2)
#define TF2_DECAY      (1<<3)
#define TF2_NIGHT_BLOOM (1<<4)

static const PlantDef plant_defs[] = {
    /* 0: Coneflower */  {TERRAIN_FLOWER_BED, TF_FLOWERS|TF_NECTAR|TF_SEEDS, 0, 2},
    /* 1: Blk-Ey Susan */ {TERRAIN_FLOWER_BED, TF_FLOWERS|TF_NECTAR, 0, 2},
    /* 2: Bergamot */    {TERRAIN_FLOWER_BED, TF_FLOWERS|TF_NECTAR, 0, 2},
    /* 3: Goldenrod */   {TERRAIN_FLOWER_BED, TF_FLOWERS|TF_NECTAR|TF_SEEDS, 0, 2},
    /* 4: Aster */       {TERRAIN_FLOWER_BED, TF_FLOWERS|TF_NECTAR, 0, 2},
    /* 5: Milkweed */    {TERRAIN_FLOWER_BED, TF_FLOWERS|TF_NECTAR, 0, 2},
    /* 6: Violet */      {TERRAIN_WOODLAND, TF_FLOWERS, 0, 2},
    /* 7: Wood Phlox */  {TERRAIN_WOODLAND, TF_FLOWERS|TF_NECTAR, 0, 2},
    /* 8: Moonflower */  {TERRAIN_FLOWER_BED, TF_FLOWERS|TF_NECTAR, TF2_NIGHT_BLOOM, 2},
    /* 9: Blue Stem */   {TERRAIN_GRASS, TF_GRASSLAND|TF_SEEDS, TF2_SHELTER, 2},
    /* 10: Switchgrass */ {TERRAIN_GRASS, TF_GRASSLAND|TF_SEEDS, TF2_SHELTER, 2},
    /* 11: Wood Sedge */  {TERRAIN_WOODLAND, 0, TF2_WOODLAND|TF2_SHELTER, 2},
    /* 12: Serviceberry */ {TERRAIN_GRASS, TF_FRUIT, TF2_SHELTER, 4},
    /* 13: Elderberry */   {TERRAIN_GRASS, TF_FRUIT, TF2_SHELTER, 4},
    /* 14: Oak */          {TERRAIN_GRASS, 0, TF2_CANOPY|TF2_WOODLAND|TF2_SHELTER, 12},
    /* 15: Willow */       {TERRAIN_GRASS, TF_WETLAND, TF2_CANOPY|TF2_SHELTER, 12},
    /* 16: Cattail */      {TERRAIN_SHALLOW_WATER, TF_WETLAND, TF2_SHELTER, 2},
    /* 17: Pickerelweed */ {TERRAIN_SHALLOW_WATER, TF_WETLAND|TF_FLOWERS|TF_NECTAR, 0, 2},
    /* 18: Water Lily */   {TERRAIN_SHALLOW_WATER, TF_WATER|TF_FLOWERS, 0, 2},
    /* 19: Marsh Marigold */ {TERRAIN_SHALLOW_WATER, TF_WETLAND|TF_FLOWERS|TF_NECTAR, 0, 2},
    /* 20: Shelf Mushroom */ {TERRAIN_WOODLAND, 0, TF2_DECAY, 2},
    /* 21: Glowcap */       {TERRAIN_WOODLAND, 0, TF2_DECAY, 2},
};

/* Object definition */
typedef struct {
    uint8_t tags, tags2;
    bool decoration;
} ObjectDef;

static const ObjectDef object_defs[] = {
    /* 22: Fallen Log */  {0, TF2_SHELTER|TF2_DECAY|TF2_WOODLAND, false},
    /* 23: Old Stump */   {0, TF2_DECAY|TF2_SHELTER, false},
    /* 24: Rock */        {0, TF2_SHELTER, false},
    /* 25: Rock Pile */   {0, TF2_SHELTER, false},
    /* 26: Brush Pile */  {0, TF2_SHELTER|TF2_WOODLAND, false},
    /* 27: Bird Bath */   {TF_WATER, 0, false},
    /* 28: Bird House */  {0, TF2_SHELTER, false},
    /* 29: Bench */       {0, 0, true},
    /* 30: Lantern */     {0, 0, true},
    /* 31: Trellis */     {0, 0, true},
    /* 32: Footbridge */  {0, 0, true},
    /* 33: Step Stone */  {0, 0, true},
};

/* Wildlife definition */
typedef struct {
    uint8_t req_tag[3];
    uint8_t req_count[3];
    uint8_t time_mask;
    uint8_t weather_mask;
    uint8_t resident_visits;
    uint8_t behavior;
    uint8_t sprite_pal;
    uint8_t rarity;
} WildlifeDef;

#define TM_MORNING (1<<TIME_MORNING)
#define TM_DAY     (1<<TIME_DAY)
#define TM_EVENING (1<<TIME_EVENING)
#define TM_NIGHT   (1<<TIME_NIGHT)
#define WM_CLEAR   (1<<WEATHER_CLEAR)
#define WM_CLOUDY  (1<<WEATHER_CLOUDY)
#define WM_RAIN    (1<<WEATHER_RAIN)
#define WM_ANY     0x07

/* Special requirement markers */
#define REQ_MILKWEED     0xF0
#define REQ_MATURE_TREE  0xF1
#define REQ_SHALLOW_TILES 0xF2
#define REQ_DEEP_TILES   0xF3
#define REQ_BIRD_HOUSE   0xF4
#define REQ_FALLEN_LOG   0xF5
#define REQ_ROCK_PILE    0xF6
#define REQ_STUMP        0xF7
#define REQ_CATTAIL      0xF8
#define REQ_SERVICEBERRY 0xF9
#define REQ_BERGAMOT     0xFA
#define REQ_OAK          0xFB
#define REQ_ROCK         0xFC

static const WildlifeDef wildlife_defs[] = {
    /* 0: Honey Bee */
    {{TAG_FLOWERS,0,0},{3,0,0}, TM_MORNING|TM_DAY, WM_CLEAR|WM_CLOUDY, 3, 0, 1, 0},
    /* 1: Bumblebee */
    {{TAG_NECTAR,0,0},{4,0,0}, TM_MORNING|TM_DAY, WM_ANY, 3, 1, 1, 0},
    /* 2: Monarch */
    {{REQ_MILKWEED,TAG_NECTAR,0},{1,3,0}, TM_DAY, WM_CLEAR, 3, 2, 1, 1},
    /* 3: Swallowtail */
    {{TAG_FLOWERS,0,0},{5,0,0}, TM_DAY, WM_CLEAR, 3, 3, 1, 1},
    /* 4: Luna Moth */
    {{TAG_NIGHT_BLOOM,REQ_MATURE_TREE,0},{3,1,0}, TM_NIGHT, WM_CLEAR|WM_CLOUDY, 4, 4, 3, 2},
    /* 5: Sphinx Moth */
    {{TAG_NIGHT_BLOOM,TAG_NECTAR,0},{2,4,0}, TM_EVENING|TM_NIGHT, WM_CLEAR, 3, 5, 3, 1},
    /* 6: Fireflies */
    {{TAG_GRASSLAND,TAG_SHELTER,0},{4,2,0}, TM_EVENING|TM_NIGHT, WM_CLOUDY|WM_RAIN, 3, 6, 3, 0},
    /* 7: Dragonfly */
    {{REQ_SHALLOW_TILES,TAG_WETLAND,0},{4,2,0}, TM_DAY, WM_CLEAR, 3, 7, 1, 1},
    /* 8: Lady Beetle */
    {{TAG_FLOWERS,0,0},{4,0,0}, TM_MORNING|TM_DAY, WM_ANY, 3, 0, 1, 0},
    /* 9: Snail */
    {{TAG_WETLAND,0,0},{2,0,0}, TM_MORNING|TM_EVENING, WM_RAIN, 3, 8, 1, 1},
    /* 10: Toad */
    {{TAG_SHELTER,TAG_WETLAND,0},{2,2,0}, TM_EVENING|TM_NIGHT, WM_RAIN, 3, 8, 2, 1},
    /* 11: Tree Frog */
    {{REQ_SHALLOW_TILES,TAG_WETLAND,0},{4,3,0}, TM_EVENING|TM_NIGHT, WM_RAIN|WM_CLOUDY, 3, 8, 2, 1},
    /* 12: Salamander */
    {{TAG_WOODLAND,TAG_DECAY,TAG_WETLAND},{4,2,1}, TM_EVENING|TM_NIGHT, WM_RAIN, 3, 8, 2, 2},
    /* 13: Garter Snake */
    {{TAG_GRASSLAND,TAG_SHELTER,0},{4,2,0}, TM_DAY, WM_CLEAR|WM_CLOUDY, 3, 9, 2, 1},
    /* 14: Cottontail */
    {{TAG_GRASSLAND,TAG_SHELTER,0},{6,2,0}, TM_MORNING|TM_EVENING, WM_ANY, 4, 9, 2, 0},
    /* 15: Chipmunk */
    {{TAG_SEEDS,TAG_SHELTER,0},{3,2,0}, TM_MORNING|TM_DAY, WM_CLEAR|WM_CLOUDY, 3, 9, 2, 0},
    /* 16: Squirrel */
    {{REQ_OAK,TAG_SEEDS,0},{1,3,0}, TM_DAY, WM_CLEAR|WM_CLOUDY, 3, 9, 2, 1},
    /* 17: Robin */
    {{TAG_FLOWERS,TAG_GRASSLAND,0},{3,1,0}, TM_MORNING|TM_DAY, WM_ANY, 3, 10, 2, 0},
    /* 18: Chickadee */
    {{TAG_CANOPY,TAG_SHELTER,0},{1,2,0}, TM_MORNING|TM_DAY, WM_ANY, 3, 10, 2, 0},
    /* 19: Cardinal */
    {{TAG_FRUIT,TAG_SHELTER,0},{3,2,0}, TM_MORNING|TM_DAY, WM_ANY, 3, 10, 2, 1},
    /* 20: Goldfinch */
    {{TAG_SEEDS,TAG_FLOWERS,0},{5,3,0}, TM_DAY, WM_CLEAR|WM_CLOUDY, 3, 10, 2, 1},
    /* 21: Hummingbird */
    {{TAG_NECTAR,0,0},{7,0,0}, TM_DAY, WM_CLEAR, 4, 11, 1, 2},
    /* 22: Woodpecker */
    {{TAG_CANOPY,TAG_DECAY,0},{2,1,0}, TM_MORNING|TM_DAY, WM_CLEAR|WM_CLOUDY, 3, 10, 2, 1},
    /* 23: Bat */
    {{TAG_NECTAR,TAG_SHELTER,0},{4,3,0}, TM_NIGHT, WM_CLEAR|WM_CLOUDY, 4, 11, 3, 1},
    /* 24: Owl */
    {{TAG_CANOPY,TAG_SHELTER,0},{3,3,0}, TM_NIGHT, WM_CLEAR|WM_CLOUDY, 4, 10, 3, 2},
    /* 25: Raccoon */
    {{TAG_WATER,TAG_FRUIT,TAG_SHELTER},{3,2,2}, TM_NIGHT, WM_ANY, 4, 9, 2, 1},
    /* 26: Painted Turtle */
    {{REQ_DEEP_TILES,REQ_SHALLOW_TILES,REQ_ROCK},{3,3,1}, TM_DAY, WM_CLEAR, 3, 8, 2, 1},
    /* 27: Great Blue Heron */
    {{TAG_WATER,TAG_WETLAND,0},{8,5,0}, TM_MORNING|TM_DAY, WM_CLEAR|WM_CLOUDY, 4, 9, 2, 2},
};

/* Shop entries */
typedef struct {
    uint8_t item_id;
    uint8_t price;
    bool late_unlock;
} ShopEntry;

static const ShopEntry shop_items[] = {
    {PLANT_CONEFLOWER, 4, false}, {PLANT_BLACK_EYED_SUSAN, 4, false},
    {PLANT_WILD_BERGAMOT, 5, false}, {PLANT_LITTLE_BLUESTEM, 4, false},
    {PLANT_MILKWEED, 6, false}, {OBJ_BIRD_BATH, 8, false},
    {OBJ_FALLEN_LOG, 6, false}, {OBJ_ROCK, 3, false},
    {OBJ_BENCH, 10, false}, {OBJ_LANTERN, 8, false},
    {PLANT_GOLDENROD, 5, true}, {PLANT_ASTER, 5, true},
    {PLANT_SWITCHGRASS, 5, true}, {PLANT_SERVICEBERRY, 8, true},
    {PLANT_ELDERBERRY, 8, true}, {PLANT_CATTAIL, 6, true},
    {PLANT_PICKERELWEED, 6, true}, {PLANT_WATER_LILY, 7, true},
    {OBJ_BIRD_HOUSE, 10, true}, {OBJ_ROCK_PILE, 6, true},
    {OBJ_BRUSH_PILE, 6, true}, {OBJ_TRELLIS, 8, true},
    {OBJ_FOOTBRIDGE, 12, true},
};
#define SHOP_COUNT (sizeof(shop_items)/sizeof(ShopEntry))

/* Progression thresholds */
#define UNLOCK_TRAIL_VISITORS    3
#define UNLOCK_POND_VISITORS     6
#define UNLOCK_POND_RESIDENTS    2
#define UNLOCK_GROVE_VISITORS   12
#define UNLOCK_GROVE_RESIDENTS   5
#define UNLOCK_MEADOW_VISITORS  20
#define UNLOCK_MEADOW_RESIDENTS  9
#define ENDING_VISITORS         24
#define ENDING_RESIDENTS        12

#endif /* DATA_TABLES_H */
