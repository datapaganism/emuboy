#include "fifo.hpp"
#include "ppu.hpp"
#include "bus.hpp"


FIFO::FIFO()
{
	fetcher.connectToFIFO(this);
}

void FIFO::reset()
{
	Stack<>::reset();
	fifo_cycle_counter = 0;
	fetcher.reset();
}


void FIFO::connectToPPU(PPU* pPPU)
{
	ppu = pPPU;
}