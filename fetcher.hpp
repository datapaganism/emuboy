#pragma once
#include <array>

#include "config.hpp"
#include "FIFO_pixel.hpp"

class FIFO;


class Fetcher
{
public:
	Fetcher();
	
	Word address_to_read = 0;
	Word tile_map_address = 0;
	Word tile_address = 0;
	Byte tile_number = 0;
	Byte data0 = 0;
	Byte data1 = 0;
	Byte fetcher_x = 0; // ranges between 0 - 31
	Byte fetcher_y = 0; // ranges between 0 - 255
	Byte fetcher_scanline_x = 0; // increments by 1 per pixel fetch, 0 to 159 range, independent of x's pushed 
	Byte state = 0;
	std::array<FIFOPixel, 8> temp_buffer;
	int cycle_counter = 0;
	FIFO* fifo_parent = nullptr;
	
	void connectToFIFO(FIFO* fifo_ptr);
	Word SCRegistersToTopLeftBGMapAddress();
	Byte getTileNumber(Word address);
	void updateFetcher(const int cycles);
	void reset();
	void incAddress();

private:
	
};