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
static const eJoypadButtons eJoypadButtons_all[] = { d_up, d_down, d_left, d_right, b_a, b_b, b_start, b_select };

static enum eJoypadButtons keyToEnum(int key)
{
    for (const auto key_enum : eJoypadButtons_all)
        if (key_enum == key)
            return key_enum;

    return UNKNOWN;
};

enum eJoypadButtonsDebug
{
    pause = SDLK_p,
    showVRAM = SDLK_o,
};