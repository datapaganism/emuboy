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

	
	/*
	void readSave();
	void writeSave();

	Byte getCartridgeType();
	Byte getRomSize();
	Byte getRamSize();

	Byte getMemory(const Word address);
	void setMemory(const Word address, const Byte data);

	void allocateRam();
	void ramBankEnableHandler(const Word address, const Byte data);

	void ramBankChange(const Word address, const Byte data);
	void romBankChange(const Word address, const Byte data);
	*/
	//void cartridgeTypeInit();



};
