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


// emulation wise, the display is rendered like a crt display, I think the approach to take for this
// is to emulate the ppu into an array framebuffer, this buffer can be modified each cpu cycle,
// and then render the screen every frame by rendering the contents of the framebuffer, the
// framebuffer should hold pixel colour values.




/*
 possible rendering solution


		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		for (int x = 0; x < gb_width; x++)
		{
			for (int y = 0; y < gb_HEIGHT; y++)
			{
				if (display == on)
				{
					colour[3] = this->framebuffer.get_colour(x,y);
					SDL_SetRenderDrawColor(renderer.renderer, colour[0],colour[1],colour[2], 255);
					SDL_Rect r;
					r.x = x * WINDOW_MULTIPLIER;
					r.y = y * WINDOW_MULTIPLIER;
					r.w = WINDOW_MULTIPLIER;
					r.h = WINDOW_MULTIPLIER;
					SDL_RenderFillRect(renderer, &r);
				}
			}
		}

		SDL_RenderPresent(renderer);


*/

struct FRAMEBUFFER_PIXEL
{
	FRAMEBUFFER_PIXEL(Byte red, Byte blue, Byte green) : FRAMEBUFFER_PIXEL()
	{
		this->red = red;
		this->blue = blue;
		this->green = green;
	}

	FRAMEBUFFER_PIXEL()
	{
		this->red = 0x0;
		this->blue = 0x0;
		this->green = 0x0;
	}

	Byte red;
	Byte blue;
	Byte green;
};

struct TILE
{
	std::array<Byte, 16> bytes_per_tile = { 0, };

	void consolePrint();
	void getPixelColour(int x, int y);
	TILE(BUS *bus, Word address);

	TILE();

};

struct FIFO_pixel
{
	Byte colour : 2;
	Byte palette : 3;
	bool sprite_priority;
	bool bg_priority;

	FIFO_pixel()
	{
		this->bg_priority = 0;
		this->sprite_priority = 0;
		this->colour = 0;
		this->palette = 0;
	};

	FIFO_pixel(Byte colour, Byte palette, bool sprite_priority, bool bg_priority) : FIFO_pixel()
	{
		this->colour = colour;
		this->palette = palette;
		this->sprite_priority = sprite_priority;
		this->bg_priority = bg_priority;
	}
};

class FIFO
{
public:
	FIFO();
	
	std::array<FIFO_pixel, 16> queue;

	void push(FIFO_pixel pixel);

	FIFO_pixel pop();

private:
	Byte tail_pos = 0;

};

class PPU
{
public:
	PPU();
	void init();
	void connect_to_bus(BUS* pBus);

	FRAMEBUFFER_PIXEL framebuffer[XRES * YRES];
	
	TILE tile;

	FIFO fifo_bg;
	FIFO fifo_sprite;



private:
	BUS* bus = nullptr;
};

