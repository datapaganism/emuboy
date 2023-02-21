#pragma once
#include "config.hpp"
#include "MBC.hpp"

class MBC2 : public MBC
{
public:
	
	MBC2::MBC2() : MBC() {};

	void ramBankEnableHandler(Word address, Byte data) override;

	void romBankChange(Word address, Byte data) override;

	void setMemory(const Word address, const Byte data) override;
	Byte getMemory(const Word address) override;

};
