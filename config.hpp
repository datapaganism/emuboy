#pragma once
#include <cstdint>

const int DEFAULT_SCREEN_WIDTH = 640;
const int DEFAULT_SCREEN_HEIGHT = 480;
const bool SHOW_ON_START = false;


#define GB_CLOCKSPEED  4194304 // Hz
#define GB_CPU_MCYCLE_CLOCKSPEED 1048576
#define GBC_CLOCKSPEED 8388000 // Hz, Color model, maybe implement in the future.

#define VSYNC 59.73 // Hz, basically 60 but I want to be accurate

#define CYCLES_PER_FRAME GB_CPU_MCYCLE_CLOCKSPEED/VSYNC // used to figure out how many CPU operations can be done before we need update the screen a single frame

// the CPU uses cycles 70221 times per video frame
// a video frame takes 70221/4 cycles to render.


#define XRES 160
#define YRES 144

#define RES_SCALING 3 // defined integer scaling

#define EMULATOR_WINDOW_TITLE "emulator"

#define Byte std::uint8_t
#define Word std::uint16_t

#define Byte_s std::int8_t
#define Word_s std::int16_t

#define WORKRAMOFFSET 0xC000
#define IOOFFSET 0xFF00
#define OAMOFFSET 0xFE00
#define VIDEORAMOFFSET 0x8000
#define HIGHRAMOFFSET 0xFF80

#define IF_REGISTER 0xFF0F
#define IE_REGISTER 0xFFFF


// timer register addresses
#define DIV  0xFF04
#define TIMA 0xFF05
#define TMA  0xFF06
#define TAC  0xFF07

#define DIVinit GB_CLOCKSPEED / 16382
#define DIV_INC_RATE GB_CPU_MCYCLE_CLOCKSPEED / 16382

#define LCDC 0xFF40  //LCD Control R/W Register //ff40
#define STAT 0xFF41 //LCDC Status R/W Register //ff41


#define SCY 0xFF42  // position of topleft (y comp) pixel of the viewport within the bg map,  r/w 0xff42
#define SCX 0xFF43  // position of topleft (x comp) pixel of the viewport within the bg map,  r/w 0xff42

#define LY 0xFF44 //lcdc y coordinate read only //ff44
#define LYC 0xFF45 //ly compare r/w //

#define DMA 0xFF46 //dma transfer and start
#define BGP 0xFF47 //bg palette r/w

#define OBP0 0xFF48 //object palette 0 r/w
#define OBP1 0xFF49 //object palette 1 r/w

#define WY 0xFF4A // window y position r/w
#define WX 0xFF4B // window x position r/w

#define OAM 0xFE00 // OAM start address, to 0xFE9F

#define WY 0xFF4A // WY r/w
#define WX 0xFF4B // WX, true value += 7 r/w

// convert to gb register freq
// gb = 2048 - (131072 / hz) 

// convert to hz from gb freq register 
// hz = 131072 / (2048 - gb)


#define GB_PALLETE_OFF 0xFFFFFFFF
#define GB_PALLETE_00  0xFF 
#define GB_PALLETE_01  0xC0
#define GB_PALLETE_10  0x69
#define GB_PALLETE_11  0x00

#define GB_PALLETE_00_r 0x89 
#define GB_PALLETE_00_g 0xc0 
#define GB_PALLETE_00_b 0x77

#define GB_PALLETE_01_r 0x4d 
#define GB_PALLETE_01_g 0xa3 
#define GB_PALLETE_01_b 0x50 

#define GB_PALLETE_10_r 0x37
#define GB_PALLETE_10_g 0x76 
#define GB_PALLETE_10_b 0x4A 

#define GB_PALLETE_11_r 0x22 
#define GB_PALLETE_11_g 0x49 
#define GB_PALLETE_11_b 0x39 

#define GB_PALLETE_BG_r 0xCA 
#define GB_PALLETE_BG_g 0xDC 
#define GB_PALLETE_BG_b 0x9F 