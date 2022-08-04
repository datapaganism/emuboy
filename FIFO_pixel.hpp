#pragma once
#include "config.hpp"

struct FIFOPixel
{
	Byte colour : 2;
	Byte palette : 3;
	bool sprite_priority;
	bool bg_priority;

	FIFOPixel()
	{
		this->bg_priority = 0;
		this->sprite_priority = 0;
		this->colour = 0;
		this->palette = 0;
	};

	FIFOPixel(Byte colour, Byte palette, bool sprite_priority, bool bg_priority) : FIFOPixel()
	{
		this->colour = colour;
		this->palette = palette;
		this->sprite_priority = sprite_priority;
		this->bg_priority = bg_priority;
	}
};
