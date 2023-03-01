#pragma once
#include "config.hpp"
#include "MBC.hpp"

class NO_MBC : public MBC
{
public:
	
	NO_MBC() : MBC() {};

	Byte getMemory(const Word address) override;
	void setMemory(const Word address, const Byte data) override;
};
