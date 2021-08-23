#pragma once

#include "config.h"
#include <array>
class BUS;

// gameboy graphics use a tiling system, instead of a frame buffer like modern systems.
// the gameboy has 8x8 pixel tiles in memory and calls their pointers to be rendered.
// it also uses a 2 bits per pixel architecture for storing a colour place holder, this allows for each pixel to have 1 out of 4 colours,
// which is picked from a palette.

// since each pixel is two bits, it takes 2 bytes of data store a single 8pixel line from a tile, and since there are 8 x 8 tiles, it takes 16 bytes for each tile.
// the first byte represents the 1 first bit of every pixel in the top line, the second byte represents

struct TILE
{
	std::array<Byte, 16> bytes_per_tile;
	Byte arrayy[16];

	void getPixelColour(int x, int y);

};

class PPU
{
public:
	void init();
	void connect_to_bus(BUS* pBus);

	TILE tile;



private:
	BUS* bus = nullptr;
};

