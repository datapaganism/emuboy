#pragma once
#include "config.hpp"
#include "MBC.hpp"

class MBC5 : public MBC
{
public:
	
	MBC5::MBC5() : MBC() {};

	void ramBankEnable(Word address, Byte data) override;

	void ramBankChange(Word address, Byte data) override;
	void romBankChange(Word address, Byte data) override;
	void romBankChange9thBit(Word address, Byte data);

	void setMemory(const Word address, const Byte data) override;
	Byte getMemory(const Word address) override;
};
