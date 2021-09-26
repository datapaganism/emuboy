#pragma once

#include <cstdint>

#define DEBUG 0
#define TURBO 0


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

#define Byte_s int8_t
#define Word_s int16_t

#define WORKRAMOFFSET 0xc000
#define IOOFFSET 0xFF00

#define IF_REGISTER 0xFF0F
#define IE_REGISTER 0xFFFF


// timer register addresses
#define DIV  0xFF04
#define TIMA 0xFF05
#define TMA  0xFF06
#define TAC  0xFF07

#define DIVinit GB_CLOCKSPEED / 16382

#define LCDC 0xFF40  //LCD Control R/W Register //ff40
#define STAT 0xFF41 //LCDC Status R/W Register //ff41

#define SCY 0xFF42  //scroll y r/w //ff42
#define SCX 0xFF43 //scroll x r/w //ff43

#define LY 0xFF44 //lcdc y coordinate read only //ff44
#define LYC 0xFF45 //ly compare r/w //

#define DMA 0xFF46 //dma transfer and start
#define BGP 0xFF47 //bg palette r/w

#define OBP0 0xFF48 //object palette 0 r/w
#define OBP1 0xFF49 //object palette 1 r/w

#define WY 0xFF4A // window y position r/w
#define WX 0xFF4B // window x position r/w



// convert to gb register freq
// gb = 2048 - (131072 / hz) 

// convert to hz from gb freq register 
// hz = 131072 / (2048 - gb)


#define GB_PALLETE_OFF 0xFFFFFFFF
#define GB_PALLETE_00  0xE0F8D0FF 
#define GB_PALLETE_01  0x88c070FF 
#define GB_PALLETE_10  0x346856FF
#define GB_PALLETE_11  0x081820FF