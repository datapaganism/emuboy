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

	Fetcher fetcher;
	std::array<FIFOPixel, 16> queue;
	bool can_fetch = true;
	int fifo_cycle_counter = 0;
	PPU* ppu_parent = nullptr;
	Byte_s tail_pos = -1;

	void connectToPPU(PPU* pPPU);
	void push(FIFOPixel pixel);
	FIFOPixel pop();
	void updateFIFO(int cyclesUsed);
	void reset();
private:

};