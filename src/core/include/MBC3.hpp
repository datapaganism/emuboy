#pragma once
#include "config.hpp"
#include "MBC.hpp"

class MBC3 : public MBC
{
public:
	
	MBC3::MBC3() : MBC() {};

	void ramBankEnableHandler(Word address, Byte data) override;

	void ramBankChange(Word address, Byte data) override;
	void romBankChange(Word address, Byte data) override;


	void setMemory(const Word address, const Byte data) override;
	Byte getMemory(const Word address) override;

};
