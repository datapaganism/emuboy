#pragma once
#include "SDL.h"

enum eJoypadButtons
{
    d_up = SDLK_UP,
    d_down = SDLK_DOWN,
    d_left = SDLK_LEFT,
    d_right = SDLK_RIGHT,

    b_a = SDLK_a,
    b_b = SDLK_s,

    b_start = SDLK_b,
    b_select = SDLK_n,
    UNKNOWN = NULL,
};

enum eJoypadButtonsDebug
{
    pause = SDLK_p,
    showVRAM = SDLK_o,
};


static const eJoypadButtons eJoypadButtons_all[] = { d_up, d_down, d_left, d_right, b_a, b_b, b_start, b_select };

static enum eJoypadButtons keyToEnum(int key)
{
    for (const auto key_enum : eJoypadButtons_all)
        if (key_enum == key)
            return key_enum;

    return UNKNOWN;
};

inline int button_to_bit(const enum eJoypadButtons button)
{
    int bit = 0;
    switch (button)
    {
    case d_right: {bit = 0b00000001; break; }
    case d_left: {bit = 0b00000010; break; }
    case d_up: {bit = 0b00000100; break; }
    case d_down: {bit = 0b00001000; break; }
    case b_a: {bit = 0b00010000; break; }
    case b_b: {bit = 0b00100000; break; }
    case b_select: {bit = 0b01000000; break; }
    case b_start: {bit = 0b10000000; break; }
    default: fprintf(stderr, "Unreachable button press");  exit(-1); break;
    }

    return bit;
}
