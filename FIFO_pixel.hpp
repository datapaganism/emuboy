#pragma once
#include "config.hpp"

struct FIFO_pixel
{
	Byte colour : 2;
	Byte palette : 3;
	bool sprite_priority;
	bool bg_priority;

	FIFO_pixel()
	{
		this->bg_priority = 0;
		this->sprite_priority = 0;
		this->colour = 0;
		this->palette = 0;
	};

	FIFO_pixel(Byte colour, Byte palette, bool sprite_priority, bool bg_priority) : FIFO_pixel()
	{
		this->colour = colour;
		this->palette = palette;
		this->sprite_priority = sprite_priority;
		this->bg_priority = bg_priority;
	}
};
