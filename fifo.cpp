#include "fifo.hpp"
#include "ppu.hpp"
#include "bus.hpp"







void FIFO::push(FIFO_pixel pixel)
{
	if (tail_pos <= 15)
	{
		this->queue[++tail_pos] = pixel;
	}
}

//void FIFO::push(const Word address)
//{
//	//need to access bus, write fetcher
//	Byte tile_number = this->;
//}


FIFO_pixel FIFO::pop()
{
	if (tail_pos > -1)
	{
		FIFO_pixel temp = this->queue[0];
		for (int i = 0; i < this->tail_pos; i++)
		{
			this->queue[i] = this->queue[i + 1];
		}
		memset(&this->queue[this->tail_pos], 0, sizeof(FIFO_pixel));
		this->tail_pos--;

		return temp;
	}
}

void FIFO::update_fifo(int cyclesUsed)
{
	this->fifo_cycle_counter += cyclesUsed;

	while (this->fifo_cycle_counter >= 4)
	{
		this->fifo_cycle_counter -= 4;

		if (this->tail_pos > -1)
		{
			BUS* pBus = this->ppu_parent->bus;
			Byte ly = pBus->get_memory(LY);
			
			//debug
			//ly = this->ppu_parent->debug_ly;

			if ( this->ppu_parent->scanline_x <= 160)
			{
				this->ppu_parent->add_to_framebuffer(this->ppu_parent->scanline_x, ly, this->pop());
				this->ppu_parent->scanline_x++;
			}

			//if (this->ppu_parent->scanline_x > 160)
			//{
			//	this->ppu_parent->scanline_x = 0;
			//	pBus->set_memory(LY, ly+1);

			//	//debug
			//	this->ppu_parent->debug_ly++;

			//}
		}
	}
}

void FIFO::reset()
{
	this->tail_pos = -1;
	this->queue.fill(FIFO_pixel(NULL, NULL, NULL, NULL));
	this->fifo_cycle_counter = 0;
	this->fetcher.reset();
}

FIFO::FIFO()
{
	this->fetcher.connect_to_fifo(this);
}

void FIFO::connect_to_ppu(PPU* pPPU)
{
	this->ppu_parent = pPPU;
}

