#pragma bank 2

#include "mosslight.h"
#include "data_tables.h"

BANKREF(bank2)

/* Plant names - compact for GBC */
const char* plant_names[] = {
    "Coneflower",
    "Blk-Ey Susan",
    "Bergamot",
    "Goldenrod",
    "Aster",
    "Milkweed",
    "Violet",
    "Wood Phlox",
    "Moonflower",
    "Blue Stem",
    "Switchgrass",
    "Wood Sedge",
    "Serviceberry",
    "Elderberry",
    "Oak",
    "Willow",
    "Cattail",
    "Pickerelweed",
    "Water Lily",
    "Marsh Marigold",
    "Shelf Mush",
    "Glowcap",
};

/* Wildlife names */
const char* wildlife_names[] = {
    "Honey Bee",
    "Bumblebee",
    "Monarch",
    "Swallowtail",
    "Luna Moth",
    "Sphinx Moth",
    "Firefly",
    "Dragonfly",
    "Lady Beetle",
    "Snail",
    "Toad",
    "Tree Frog",
    "Salamander",
    "Garter Snake",
    "Cottontail",
    "Chipmunk",
    "Squirrel",
    "Robin",
    "Chickadee",
    "Cardinal",
    "Goldfinch",
    "Hummingbird",
    "Woodpecker",
    "Bat",
    "Owl",
    "Raccoon",
    "Turtle",
    "Heron",
};

/* Object names */
const char* object_names[] = {
    "Fallen Log",
    "Old Stump",
    "Rock",
    "Rock Pile",
    "Brush Pile",
    "Bird Bath",
    "Bird House",
    "Bench",
    "Lantern",
    "Trellis",
    "Footbridge",
    "Step Stone",
};

/* Area names */
const char* area_names[] = {
    "Main Garden",
    "Pond Edge",
    "Old Grove",
    "Moon Meadow",
    "Gloamwick",
    "Woodland Trail",
};

/* Time period names */
const char* time_names[] = {
    "Morning",
    "Day",
    "Evening",
    "Night",
};

/* Weather names */
const char* weather_names[] = {
    "Clear",
    "Cloudy",
    "Rain",
};

/* NPC dialogue */
const char* mara_greeting = "Welcome to Mara's nursery!";

const char* rowan_lines[] = {
    "Dragonflies need more\nthan water. Give the\npond plants to hunt\naround.",
    "Some moths only appear\nafter the flowers open\nat night.",
    "Old wood looks untidy\nto us. To half the\ngarden, it looks like\nhome.",
    "A garden with only\nflowers is a banquet.\nAdd shelter and water\nand it becomes a home.",
    "Patience. The garden\nwas never meant to be\nfinished in a day.",
    "Even a single log can\nchange who comes to\nvisit.",
};

#define ROWAN_LINE_COUNT (sizeof(rowan_lines)/sizeof(char*))

/* Caretaker note */
const char* caretaker_note = "The garden never needed\ntaming. It only needed\nsomeone to notice what\nbelonged here.";

const char* caretaker_note_ending = "The garden never needed\ntaming. It only needed\nsomeone to notice what\nbelonged here.\n\nYou noticed.";

/* Wildlife habitat hints (shown before discovery) */
const char* wildlife_hints[] = {
    "Small buzzer that loves\nopen flowers.",
    "A fuzzy, slow flier\nthat visits many blooms.",
    "An orange wanderer that\nneeds a special plant.",
    "A large butterfly that\nloops over abundant\nflowers.",
    "Something rare and\ngreen glows near night\nblooms and old trees.",
    "A fast hovering visitor\nat dusk near night\nblooms.",
    "Tiny lights drift over\ngrass and water at\ndusk.",
    "A darting hunter above\nplanted water.",
    "A small spotted crawler\non familiar flowers.",
    "A slow soft body emerges\nonly in rain near water.",
    "A bumpy hopper that\nhides under logs near\nwet ground.",
    "A small singer by the\nwater in the rain.",
    "A secretive crawler found\nnear old logs and wet\nplaces.",
    "A quick slender reptile\nthat slips through grass\nto hide under rocks.",
    "A long-eared hopper that\ngrazes in open grass.",
    "A small striped runner\nthat dashes between logs.",
    "A bushy-tailed climber\nthat favors old oak trees.",
    "A familiar morning singer\nthat hops and pecks.",
    "A small social bird that\nflits between branches\nand shrubs.",
    "A bright red visitor that\nperches in fruit shrubs.",
    "A yellow seed-eater that\nfavors golden blooms.",
    "A tiny buzzing jewel that\ncan hover in place.",
    "A determined drummer on\nold wood and mature\ntrunks.",
    "Fast silhouettes that\nwheel overhead at night.",
    "A quiet, watchful presence\nin the high branches at\nnight.",
    "A masked wanderer that\nexplores near water after\ndark.",
    "A patient swimmer that\nbasks on waterside rocks.",
    "Something tall and still\nwalks shallow water in\nthe morning.",
};

/* Wildlife naturalist notes (shown after discovery) */
const char* wildlife_notes[] = {
    "Bees are essential\npollinators, drawn to\nmany flower types.",
    "Bumblebees fly in cooler\nweather than most bees.",
    "Monarchs rely on milkweed\nfor their caterpillars.",
    "Swallowtails are among\nour largest butterflies.",
    "Luna Moths are short-lived\nand do not eat as adults.",
    "Sphinx Moths hover like\nhummingbirds at dusk.",
    "Fireflies use light to\nfind mates in the grass.",
    "Dragonflies patrol ponds\nfor smaller insects.",
    "Lady Beetles eat aphids\nand garden pests.",
    "Snails thrive in moisture\nand retreat in dry weather.",
    "Toads return to the same\nshelter each season.",
    "Tree Frogs sing after rain\nin warm evenings.",
    "Salamanders breathe through\ntheir moist skin.",
    "Garter Snakes are harmless\nand eat slugs and insects.",
    "Rabbits graze gently and\nrarely damage established\nplants.",
    "Chipmunks store seeds in\nsmall burrows.",
    "Squirrels plant forgotten\nacorns that become trees.",
    "Robins are often the first\nbirds heard each morning.",
    "Chickadees are curious and\nbold for their size.",
    "Cardinals keep the same\nmate and territory yearly.",
    "Goldfinches delay nesting\nuntil seeds are abundant.",
    "Hummingbirds beat their\nwings over 50 times per\nsecond.",
    "Woodpeckers communicate by\ndrumming on hollow wood.",
    "Bats navigate by sound and\neat many insects nightly.",
    "Owls hunt silently and\nturn their heads far.",
    "Raccoons are curious and\ndexterous but never\ndestructive here.",
    "Turtles sun themselves on\nrocks to regulate warmth.",
    "Herons stand motionless\nfor long minutes while\nfishing.",
};

/* Help text */
const char* help_controls = "D-Pad: Move/Navigate\nA: Interact/Confirm/Place\nB: Tool/Cancel\nStart: Menu\nSelect: Field Guide";

const char* help_gardening = "Use Trowel to make soil\nand flower beds.\nUse Spade to make water.\nWatering Can settles new\nplants. Rain does too.";

const char* help_wildlife = "Build habitat and animals\nwill come. Each species\nhas preferences for time,\nweather, and plants.";

const char* help_time = "Each period lasts about\n6 minutes. Rest at the\ncottage bed to skip to\nmorning. Time pauses in\nmenus.";

const char* help_town = "Gloamwick has a nursery\nrun by Mara and a natural\nhistory room with Rowan.";

const char* help_trail = "The Woodland Trail yields\nseeds, stones, and rare\nfinds each day. Walk it\ncasually to see what's\nnew.";
