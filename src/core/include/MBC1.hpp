#pragma once
#include "config.hpp"
#include "MBC.hpp"

class MBC1 : public MBC
{
public:

	MBC1() : MBC() {};

	void ramBankEnableHandler(Word address, Byte data) override;

	void ramBankChange(Word address, Byte data) override;
	void romBankChange(Word address, Byte data) override;

	void bankingModeSelect(Word address, Byte data) override;

	Byte getMemory(const Word address) override;
	void setMemory(const Word address, const Byte data) override;



};
