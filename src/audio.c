/*
 * Audio - GBC sound chip based music and SFX
 * Uses the 4 GBC sound channels:
 *   CH1: Square wave with sweep
 *   CH2: Square wave
 *   CH3: Custom wave (4-bit)
 *   CH4: Noise
 */
#include "mosslight.h"

/* GBC sound registers */
#define NR10 (*(volatile uint8_t*)0xFF10)
#define NR11 (*(volatile uint8_t*)0xFF11)
#define NR12 (*(volatile uint8_t*)0xFF12)
#define NR13 (*(volatile uint8_t*)0xFF13)
#define NR14 (*(volatile uint8_t*)0xFF14)
#define NR21 (*(volatile uint8_t*)0xFF16)
#define NR22 (*(volatile uint8_t*)0xFF17)
#define NR23 (*(volatile uint8_t*)0xFF18)
#define NR24 (*(volatile uint8_t*)0xFF19)
#define NR30 (*(volatile uint8_t*)0xFF1A)
#define NR31 (*(volatile uint8_t*)0xFF1B)
#define NR32 (*(volatile uint8_t*)0xFF1C)
#define NR33 (*(volatile uint8_t*)0xFF1D)
#define NR34 (*(volatile uint8_t*)0xFF1E)
#define NR41 (*(volatile uint8_t*)0xFF20)
#define NR42 (*(volatile uint8_t*)0xFF21)
#define NR43 (*(volatile uint8_t*)0xFF22)
#define NR44 (*(volatile uint8_t*)0xFF23)
#define NR50 (*(volatile uint8_t*)0xFF24)
#define NR51 (*(volatile uint8_t*)0xFF25)
#define NR52 (*(volatile uint8_t*)0xFF26)

/* Note frequencies for GBC (divided by 2 for register calc) */
/* GBC frequency = 131072 / (2048 - f) where f is 11-bit value */
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_REST 0

/* Convert Hz to GBC frequency register value */
static uint16_t freq_to_reg(uint16_t hz) {
    if (hz == 0) return 0;
    return 2048 - (131072 / hz);
}

static void play_tone(uint16_t hz, uint8_t duration, uint8_t channel) {
    if (hz == 0) return;

    uint16_t f = freq_to_reg(hz);

    switch (channel) {
        case 1: /* CH1 - square */
            NR10 = 0x00;
            NR11 = 0x80 | (duration & 0x3F);
            NR12 = 0xF0; /* volume envelope */
            NR13 = f & 0xFF;
            NR14 = 0x80 | ((f >> 8) & 0x07) | 0x40; /* restart + length */
            break;
        case 2: /* CH2 - square */
            NR21 = 0x80 | (duration & 0x3F);
            NR22 = 0xF0;
            NR23 = f & 0xFF;
            NR24 = 0x80 | ((f >> 8) & 0x07) | 0x40;
            break;
        case 4: /* CH4 - noise */
            NR41 = 0x3F;
            NR42 = 0xF0;
            NR43 = 0x20;
            NR44 = 0x80;
            break;
    }
}

void audio_init(void) {
    /* Enable sound */
    NR52 = 0x80;
    /* Set volume - full on both channels */
    NR50 = 0x77;
    /* Enable all channels to both outputs */
    NR51 = 0xFF;
}

void play_sfx(uint8_t sfx) {
    switch (sfx) {
        case SFX_BLIP:
            play_tone(NOTE_C5, 8, 2);
            break;
        case SFX_CONFIRM:
            play_tone(NOTE_E5, 8, 2);
            /* Delay then second note - simplified, just play one */
            break;
        case SFX_CANCEL:
            play_tone(NOTE_B4, 8, 2);
            break;
        case SFX_DISCOVER:
            play_tone(NOTE_G5, 16, 2);
            break;
        case SFX_PLANT:
            play_tone(NOTE_C4, 12, 1);
            break;
        case SFX_WATER:
            play_tone(NOTE_A4, 12, 1);
            break;
        case SFX_DIG:
            play_tone(NOTE_D4, 6, 1);
            break;
        case SFX_COIN:
            play_tone(NOTE_E5, 6, 2);
            break;
    }
}

/* Music - very simple loop patterns */
static uint8_t current_track = 0xFF;
static uint16_t music_note_idx = 0;
static uint16_t music_timer = 0;

/* Music note sequences (simplified) */
static const uint16_t morning_notes[] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_E5, NOTE_C5, NOTE_G4, NOTE_E4, NOTE_G4,
    NOTE_A4, NOTE_C5, NOTE_E5, NOTE_C5, NOTE_A4, NOTE_E4, NOTE_C4, NOTE_REST
};
static const uint16_t day_notes[] = {
    NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C5, NOTE_B4,
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_E5, NOTE_C5, NOTE_G4, NOTE_E4, NOTE_REST
};
static const uint16_t evening_notes[] = {
    NOTE_E5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_B4,
    NOTE_C5, NOTE_D5, NOTE_E5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_REST
};
static const uint16_t night_notes[] = {
    NOTE_A4, NOTE_C5, NOTE_E5, NOTE_C5, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_G4,
    NOTE_A4, NOTE_E4, NOTE_C4, NOTE_E4, NOTE_A4, NOTE_REST, NOTE_REST, NOTE_REST
};
static const uint16_t rain_notes[] = {
    NOTE_E4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_F4,
    NOTE_E4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4, NOTE_REST
};
static const uint16_t gloamwick_notes[] = {
    NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_A4,
    NOTE_B4, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_REST
};

#define NOTE_COUNT 16
#define NOTE_DURATION 20 /* frames per note */

void play_music(uint8_t track) {
    if (current_track == track) return;
    current_track = track;
    music_note_idx = 0;
    music_timer = 0;
}

void stop_music(void) {
    current_track = 0xFF;
    /* Stop all channels */
    NR12 = 0x00;
    NR22 = 0x00;
    NR32 = 0x00;
    NR42 = 0x00;
}

/* Called every frame to advance music */
void music_tick(void) {
    if (current_track == 0xFF) return;

    music_timer++;
    if (music_timer < NOTE_DURATION) return;
    music_timer = 0;

    const uint16_t *notes = NULL;
    switch (current_track) {
        case MUSIC_MORNING:  notes = morning_notes; break;
        case MUSIC_DAY:      notes = day_notes; break;
        case MUSIC_EVENING:  notes = evening_notes; break;
        case MUSIC_NIGHT:    notes = night_notes; break;
        case MUSIC_RAIN:     notes = rain_notes; break;
        case MUSIC_GLOAMWICK: notes = gloamwick_notes; break;
        default: return;
    }

    uint16_t note = notes[music_note_idx % NOTE_COUNT];
    if (note != NOTE_REST) {
        play_tone(note, NOTE_DURATION, 2);
    }

    music_note_idx++;
}

/* Ambient sounds - simple noise generation */
static uint8_t ambient_type = 0xFF;

void start_ambient(uint8_t type) {
    ambient_type = type;

    switch (type) {
        case 0: /* birds */
            /* Periodic chirps via CH1 */
            break;
        case 1: /* crickets */
            /* High freq pulses on CH4 */
            NR41 = 0x01;
            NR42 = 0x60;
            NR43 = 0x80;
            NR44 = 0x80;
            break;
        case 2: /* rain */
            /* Noise on CH4 */
            NR41 = 0x3F;
            NR42 = 0x40;
            NR43 = 0x40;
            NR44 = 0x80;
            break;
        case 3: /* water */
            /* Low rumble on CH3 */
            break;
        case 4: /* frogs */
            play_tone(150, 30, 1);
            break;
    }
}

void stop_ambient(void) {
    ambient_type = 0xFF;
    NR42 = 0x00;
}

void set_music_volume(uint8_t v) {
    /* Map 0-100 to 0-7 */
    uint8_t vol = (v * 7) / 100;
    NR50 = (vol << 4) | vol;
}
