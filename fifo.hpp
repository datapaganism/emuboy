#pragma once
#include <array>

#include "config.hpp"
#include "fetcher.hpp"
#include "FIFO_pixel.hpp"


class PPU;



class FIFO
{
public:
	FIFO();

	FETCHER fetcher;
	std::array<FIFO_pixel, 16> queue;
	void connect_to_ppu(PPU* pPPU);

	void push(FIFO_pixel pixel);

	FIFO_pixel pop();

	bool canFetch = true;

	int fifo_cycle_counter = 0;

	void update_fifo(int cyclesUsed);

	PPU* ppu_parent = nullptr;

	Byte_s tail_pos = -1;

	void reset();
private:

};