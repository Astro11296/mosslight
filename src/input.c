/*
 * Input system - handles GBC joypad input
 * Maps to: D-Pad, A, B, Start, Select
 */
#include "mosslight.h"

static uint8_t prev_keys = 0;
static uint8_t curr_keys = 0;

void input_update(void) {
    prev_keys = curr_keys;
    curr_keys = joypad();
}

bool btn_pressed(uint8_t button) {
    return (curr_keys & button) && !(prev_keys & button);
}

bool btn_released(uint8_t button) {
    return !(curr_keys & button) && (prev_keys & button);
}

bool btn_held(uint8_t button) {
    return (curr_keys & button);
}
