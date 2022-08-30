#pragma once
#include <array>
#include <memory>

#include "config.hpp"
#include "fifo.hpp"


#define TEST 1


class BUS;

// gameboy graphics use a tiling system, instead of a frame buffer like modern systems.
// the gameboy has 8x8 pixel tiles in memory and calls their pointers to be rendered.
// it also uses a 2 bits per pixel architecture for storing a colour place holder, this allows for each pixel to have 1 out of 4 colours,
// which is picked from a palette.

// since each pixel is two bits, it takes 2 bytes of data store a single 8pixel line from a tile, and since there are 8 x 8 tiles, it takes 16 bytes for each tile.
// the first byte represents the 1 first bit of every pixel in the top line, the second byte represents


// emulation wise, the display is rendered like a crt display, I think the approach to take for this
// is to emulate the ppu into an array framebuffer, this buffer can be modified each cpu cycle,
// and then render the screen every frame by rendering the contents of the framebuffer, the
// framebuffer should hold pixel colour values.


//ARGB8888 pixel, byte order reversed to argb, due to endianess?
struct FramebufferPixel
{
	FramebufferPixel(Byte red, Byte green, Byte blue)
	{
		this->red = red;
		this->green = green;
		this->blue = blue;
	}

	FramebufferPixel()
	{
		this->red = 0x0;
		this->green = 0x0;
		this->blue = 0x0;
	}

	Byte blue;
	Byte green;
	Byte red;
	Byte alpha = 0xFF;
};

struct Tile
{
	Tile();
	Tile(BUS* bus, Word address);
	std::array<Byte, 16> bytes_per_tile = { 0, };
	void consolePrint();
	Byte getPixelColour(int x, int y);	
};

struct PPURegisters
{
	Byte* ly = nullptr;
	Byte* wx = nullptr;
	Byte* wy = nullptr;
	Byte* lyc = nullptr;
	Byte* scx = nullptr;
	Byte* scy = nullptr;
	Byte* stat = nullptr;
	Byte* lcdc = nullptr;
};


class PPU
{
public:

	PPU();

	enum eTileType { background, window, sprite };
	Byte scanline_x = 0;
	Tile tile;
	
	BUS* bus = nullptr;
	PPURegisters registers;
	int cycle_counter = 0;
	bool window_wy_triggered = false;


	void connectToBus(BUS* pBus);
	void updateGraphics(const int cycles);
	/// <summary>
	/// polls the state of FF40 register's 7th bit, if 1 then the LCD and PPU is enabled for processing/rendering else not.
	/// </summary>
	/// <returns>if it is enabled</returns>
	bool lcdEnabled();
	/// <summary>
	/// Converts tile id to address in video ram
	/// </summary>
	Word getTileAddressFromNumber(const Byte tile_number, const enum eTileType eTileType);
	void addToFramebuffer(const int x, const int y, const FIFOPixel fifo_pixel);
	FramebufferPixel dmgFramebufferPixelToRGB(const FIFOPixel fifo_pixel);
	Byte getPPUState();
	void newScanline();
	void updateState(Byte new_state);
	Byte getMemory(const Word address);
	void setMemory(const Word address, const Byte data);
	
private:

	void setRegisters();
	FIFO fifo_bg;
	FIFO fifo_sprite;


};

