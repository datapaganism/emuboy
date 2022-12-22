#include "fifo.hpp"
#include "ppu.hpp"
#include "bus.hpp"


FIFO::FIFO()
{
	fetcher.connectToFIFO(this);
}

void FIFO::reset()
{
	FIFOStack<>::reset();
	fetcher.reset();
}


void FIFO::connectToPPU(PPU* pPPU)
{
	ppu = pPPU;
}