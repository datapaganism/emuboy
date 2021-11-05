#pragma once

#include "config.h"
#include "fifo.hpp"
#include <array>
#include <memory>
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


//RGB24 pixel
struct FRAMEBUFFER_PIXEL
{
	FRAMEBUFFER_PIXEL(Byte red, Byte green, Byte blue)
	{
		this->red = red;
		this->green = green;
		this->blue = blue;
	}

	FRAMEBUFFER_PIXEL()
	{
		this->red = 0x0;
		this->green = 0x0;
		this->blue = 0x0;
	}

	Byte red;
	Byte green;
	Byte blue;

	
};

struct TILE
{
	std::array<Byte, 16> bytes_per_tile = { 0, };

	void consolePrint();
	void getPixelColour(int x, int y);
	TILE(BUS *bus, Word address);

	TILE();

};



/// <summary>
/// im am such a fool, how do we know where we draw? well a framebuffer would be nice but there isnt enough ram for that,
/// what is the work around? a tile map and tiles.
/// you store the tiles in a region of memory and then a tile map which is like a frame buffer but has tile references.
/// </summary>



class PPU
{
public:

	Byte scanline_x = 0;

	enum tile_type { background, window, sprite };
	PPU();
	void init();
	void connect_to_bus(BUS* pBus);

	std::unique_ptr<FRAMEBUFFER_PIXEL[]> framebuffer = std::make_unique<FRAMEBUFFER_PIXEL[]>(XRES * YRES);

	
	TILE tile;

	FIFO fifo_bg;
	FIFO fifo_sprite;

	void update_graphics(const int cycles);
	
	/// <summary>
	/// polls the state of FF40 register's 7th bit, if 1 then the LCD and PPU is enabled for processing/rendering else not.
	/// </summary>
	/// <returns>if it is enabled</returns>
	bool lcd_enabled();

	/// <summary>
	/// Converts tile id to address in video ram
	/// </summary>
	Word get_tile_address_from_number(const Byte tile_number, const enum tile_type tile_type);

	
	BUS* bus = nullptr;

	/// <summary>
	/// 
	/// </summary>
	int cycle_counter = 0;

	void add_to_framebuffer(const int x, const int y, const FIFO_pixel fifo_pixel);
	FRAMEBUFFER_PIXEL dmg_framebuffer_pixel_to_rgb(const FIFO_pixel fifo_pixel);

	/// <summary>
	/// convert scx and scy position to the top left tile's (of the viewport) address, ignores shifting
	/// </summary>
	/// <returns></returns>



	void new_scanline();

	void update_state(Byte new_state);
private:

	


};

