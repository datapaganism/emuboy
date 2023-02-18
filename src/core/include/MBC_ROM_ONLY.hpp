#pragma once
#include "config.hpp"
#include "MBC.hpp"

class MBC_ROM_ONLY : public MBC
{
public:
	
	MBC_ROM_ONLY() : MBC() {};

	Byte getMemory(const Word address) override;
};
