#pragma once
#include <array>

#include "config.hpp"
#include "FIFO_pixel.hpp"

class FIFO;
class PPU;

enum eFetcherState
{
	get_tile = 0,
	get_tile_data_low,
	get_tile_data_high,
	sleep,
	push
};

class Fetcher
{
public:
	Fetcher();
	
	Word tile_map_address = 0;
	Word tile_address = 0;
	Byte tile_number = 0;
	Byte data0 = 0;
	Byte data1 = 0;
	Byte fetcher_x_tile = 0; // ranges between 0 - 31
	Byte fetcher_y_line = 0; // ranges between 0 - 255
	Byte fetcher_scanline_x = 0; // increments by 1 per pixel fetch, 0 to 159 range, independent of x's pushed 
	Byte state = 0;
	std::array<FIFOPixel, 8> pixels;
	int cycle_counter = 0;
	FIFO* fifo = nullptr;
	PPU* ppu = nullptr;
	bool rendering_sprite = false;
	bool fifo_needs_more_bgwin_pixels = false;
	
	void connectToFIFO(FIFO* fifo_ptr);
	void connectToPPU(PPU* ppu_ptr);
	Word scRegistersToTopLeftBGMapAddress();
	Byte getTileNumber(Word address);
	void updateFetcher(const int cycles);
	void reset();
	void incAddress();
	bool isWindowActive();

private:
	
};