#pragma once

#include "SDL.h"
enum JoypadButtons
{
    dUp = SDLK_UP,
    dDown = SDLK_DOWN,
    dLeft = SDLK_LEFT,
    dRight = SDLK_RIGHT,

    bA = SDLK_a,
    bB = SDLK_s,

    bStart = SDLK_b,
    bSelect = SDLK_n,
    UNKNOWN = NULL,
};
static const JoypadButtons JoypadButtons_all[] = { dUp, dDown, dLeft, dRight, bA, bB, bStart, bSelect };

static enum JoypadButtons keyToEnum(int key)
{
    for (const auto keyEnum : JoypadButtons_all)
    {
        if (keyEnum == key)
            return keyEnum;
    }
    return UNKNOWN;
};

enum JoypadButtonsDebug
{
    pause = SDLK_p,
    showVRAM = SDLK_o,
};