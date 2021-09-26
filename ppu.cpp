#include "ppu.hpp"
#include "bus.hpp"

#include <bitset>
void PPU::connect_to_bus(BUS* pBus)
{
	this->bus = pBus;
}

void TILE::consolePrint()
{    
	for (int y = 0; y < 8; y++)
	{
	    for (int x = 0; x < 8; x++)
	    {
	        this->getPixelColour(x, y);
	        std::cout << " ";
	    }
	    std::cout << "\n";
	}
}

void TILE::getPixelColour(int x, int y)
{
	int offset = (0b1 << (7 - x));

	bool bit0 = this->bytes_per_tile[2 * y] & offset;
	bool bit1 = this->bytes_per_tile[(2 * y) + 1] & offset;

	Byte result = (((Byte)bit0 << 1) | (Byte)bit1);
	std::cout << std::bitset<2>{result};

	//return result;
}

TILE::TILE(BUS* bus, Word address) : TILE()
{
	for (int i = 0; i < 16; i++)
	{
		this->bytes_per_tile[0] = bus->get_memory(address + i);
	}	
}

TILE::TILE()
{
	this->bytes_per_tile.fill(0x00);
}

PPU::PPU()
{
	for (auto& pixel : this->framebuffer)
	{
		pixel = FRAMEBUFFER_PIXEL(0xFF, 0xFF, 0xFF);
	}

	
	//int x = 3, y = 2;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);
	//x = 4, y = 2;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 6, y = 2;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);
	//x = 7, y = 2;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 2, y = 3;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 5, y = 3;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 8, y = 3;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 2, y = 4;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 8, y = 4;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 3, y = 5;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 7, y = 5;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 4, y = 6;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 6, y = 6;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 5, y = 7;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);
};



void FIFO::push(FIFO_pixel pixel)
{
	if (tail_pos <= 16)
	{
		this->queue[tail_pos++] = pixel;
	}
}

FIFO_pixel FIFO::pop()
{
	if (tail_pos > 0)
	{
		FIFO_pixel temp = this->queue[0];
		for (int i = 0; i < this->tail_pos - 1; i++)
		{
			this->queue[i] = this->queue[i + 1];
		}
		memset(&this->queue[this->tail_pos - 1], 0, sizeof(FIFO_pixel));
		this->tail_pos--;

		return temp;
	}
}

FIFO::FIFO()
{
}
