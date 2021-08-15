#pragma once

#include <cstdint>

#define GB_CLOCKSPEED  4194304 // Hz
#define GBC_CLOCKSPEED 8388000 // Hz, Color model, maybe implement in the future.

#define VSYNC 59.73 // Hz, basically 60 but I want to be accurate

#define CYCLES_PER_FRAME GB_CLOCKSPEED/VSYNC // used to figure out how many CPU operations can be done before we need update the screen a single frame

#define XRES 160
#define YRES 144

#define RES_SCALING 5 // defined integer scaling

#define EMULATOR_WINDOW_TITLE "emulator"

#define Byte uint8_t
#define Word uint16_t

#define Byte_s sint8_t
#define Word_s sint16_t

#define WORKRAMOFFSET 0xc000

// convert to gb register freq
// gb = 2048 - (131072 / hz) 

// convert to hz from gb freq register 
// hz = 131072 / (2048 - gb)


