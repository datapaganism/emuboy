#include "ppu.hpp"
#include "bus.hpp"

#include <bitset>
void PPU::connect_to_bus(BUS* pBus)
{
	this->bus = pBus;
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
