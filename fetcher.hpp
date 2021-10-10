#pragma once
#include "config.h"
#include <array>
#include "FIFO_pixel.hpp"

class FIFO;


class FETCHER
{
public:
	FETCHER();
	
	Word address_to_read = 0;
	Word tile_map_address = 0;
	Word tile_address = 0;
	Byte tile_number = 0;
	Byte data0;
	Byte data1;

	Byte fetcher_x = 0;
	Byte fetcher_y = 0;
	
	std::array<FIFO_pixel, 8> temp_buffer;

	Byte state = 0;
	int cycle_counter = 0;

	
	void connect_to_fifo(FIFO* pFIFO);
	
	//void get_tile();

	Word sc_registers_to_top_left_bg_map_address();

	//Byte get_tile_number();
	Byte get_tile_number(Word address);

	/*Byte scanline_x_pos = 0;
	Byte fetcher_x = 0;
	Byte fetcher_y = 0;*/
	/*get_tile_data_low();
	get_tile_data_high();
	push();*/

	void update_fetcher(const int cycles);

	FIFO* fifo_parent = nullptr;

	void reset();

	void latch();
	void inc_address();
private:
	
};