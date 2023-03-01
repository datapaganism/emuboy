#pragma once
#include "config.hpp"
#include "MBC.hpp"
#include <iostream>

class MBC1 : public MBC
{
public:

	MBC1() : MBC() {};
	
	void ramBankEnable(Word address, Byte data) override;

	void ramBankChange(Word address, Byte data) override;
	void romBankChange(Word address, Byte data) override;

	void bankingModeSelect(Word address, Byte data);

	Byte getMemory(const Word address) override;
	void setMemory(const Word address, const Byte data) override;

	bool banking_mode = false;

};
