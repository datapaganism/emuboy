#include "GamePak.hpp"
#include <iostream>
#include <fstream>

GamePak::GamePak(const std::string filename) : GamePak::GamePak()
{
	this->filename = filename;
	std::ifstream file(this->filename, std::ios::binary | std::ios::ate);
	if(file.is_open())
	{
		std::ifstream::pos_type pos = file.tellg();
		this->rom.resize(pos); //reallocate memory space to fit entire rom
		file.seekg(0, std::ios::beg);
		file.read((char*)this->rom.data(), pos);		
		file.close();

		gamepak_loaded = true;
		cartridge_type = getCartridgeType();
		rom_size = getRomSize();
		number_of_rom_banks = 2 << rom_size;
		ram_size = getRamSize();
		allocateRam();
		cartridgeTypeInit();
		return;
	}
	file.close();
	fprintf(stderr, "GamePak not loaded");  exit(-1);
}

GamePak::GamePak()
{
}

Byte GamePak::getCartridgeType()
{
	return this->rom[0x147];
}

Byte GamePak::getRomSize()
{
	return this->rom[0x148];
}

Byte GamePak::getRamSize()
{
	return this->rom[0x149];
}



Byte GamePak::getMemory(const Word address)
{
	if (!gamepak_loaded)
		return 0xFF;

	if (address <= 0x3FFF) // if address is within rom bank 0
		return this->rom[address];

	if (address <= 0x7FFF) // otherwise return calculate the address by the rom bank used and return that data
	{
		int new_address = (address - 0x4000) + (current_rom_bank * 0x4000);
		return rom[new_address];

	}
	
	if (!ram_bank_enable)
		return 0;
	
	if (address <= 0xBFFF)
		return this->ram[(address - 0x2000) + (this->current_ram_bank * 0x2000)];
}

void GamePak::setMemory(const Word address, const Byte data)
{
	if (!gamepak_loaded)
		return;

	if (address <= 0x1FFF)
	{ // enable ram bank writing
		this->ramBankEnableHandler(address, data);
		return;
	}

	if (address <= 0x4000) // rom bank change
	{
		this->romBankChange(address, data);
		return;
	}

	if (address <= 0x6000) // rom / ram bank change
	{
		return;
	}
}

void GamePak::allocateRam()
{
	if (cartridge_type == 0x05 || cartridge_type == 0x06)
	{
		this->ram = std::make_unique<Byte[]>(256);
		return;
	}

	switch (ram_size)
	{
	case 0: break;
	case 1: break;
	case 2: this->ram = std::make_unique<Byte[]>(1 * 0x2000); break;
	case 3: this->ram = std::make_unique<Byte[]>(4 * 0x2000); break;
	case 4: this->ram = std::make_unique<Byte[]>(16 * 0x2000); break;
	case 5: this->ram = std::make_unique<Byte[]>(8 * 0x2000); break;
	default: fprintf(stderr, "Unreachable number of banks");  exit(-1);;
	}
}

void GamePak::ramBankEnableHandler(const Word address, const Byte data)
{
	switch (cartridge_type)
	{
	case 0x00: break; // ROM ONLY
	case 0x01: // MBC1
	case 0x02: // MBC1 + RAM
	case 0x03: // MBC1 + RAM + battery
		if ((data & 0xF) == 0xA)
		{
			this->ram_bank_enable = true;
			break;
		}
		if ((data & 0xF) == 0x0)
		{
			this->ram_bank_enable = false;
			break;
		}
		break;
	case 0x05: // MBC2
	case 0x06: // MBC2 + BATTERY
		if (!(address & (0b1 << 4)))
		{
			if ((data & 0xF) == 0xA)
			{
				this->ram_bank_enable = true;
				break;
			}
			if ((data & 0xF) == 0x0)
			{
				this->ram_bank_enable = false;
				break;
			}
		}
		break;
	case 0x08: break; // ROM + RAM
	case 0x09: break; // ROM + RAM + BATTERY
	case 0x0B: break; // MMM01
	case 0x0C: break; // MMM01 + RAM
	case 0x0D: break; // MMM01 + RAM + BATTERY
	case 0x0F: break; // MBC3
	case 0x10: break; // MBC3 + RAM
	case 0x11: break; 
	case 0x12: break;
	case 0x13: break;
	case 0x19: break;
	case 0x1A: break;
	case 0x1B: break;
	case 0x1C: break;
	case 0x1D: break;
	case 0x1E: break;
	case 0x20: break;
	case 0x22: break;
	case 0xFC: break;
	case 0xFD: break;
	case 0xFE: break;
	case 0xFF: break;
	default: fprintf(stderr, "Ram Bank number %i Enable not implemented", cartridge_type);  exit(-1);
	}
}

void GamePak::ramBankChange(const Word address, const Byte data)
{
	switch (cartridge_type)
	{
	case 0x00: break; // ROM ONLY
	case 0x01: // MBC1
	case 0x02: // MBC1 + RAM
	case 0x03: // MBC1 + RAM + battery
	case 0x05: // MBC2
	case 0x06: // MBC2 + BATTERY
	case 0x08: break; // ROM + RAM
	case 0x09: break; // ROM + RAM + BATTERY
	case 0x0B: break; // MMM01
	case 0x0C: break; // MMM01 + RAM
	case 0x0D: break; // MMM01 + RAM + BATTERY
	case 0x0F: break; // MBC3
	case 0x10: break; // MBC3 + RAM
	case 0x11: break;
	case 0x12: break;
	case 0x13: break;
	case 0x19: break;
	case 0x1A: break;
	case 0x1B: break;
	case 0x1C: break;
	case 0x1D: break;
	case 0x1E: break;
	case 0x20: break;
	case 0x22: break;
	case 0xFC: break;
	case 0xFD: break;
	case 0xFE: break;
	case 0xFF: break;
	default: fprintf(stderr, "Ram Bank number %i Change not implemented", cartridge_type);  exit(-1);
	}
}

void GamePak::romBankChange(const Word address, const Byte data)
{
	switch (cartridge_type)
	{
	case 0x00: break; // ROM ONLY
	case 0x01:
		{
			Byte rom_bank_to_select = data & 0x1F;
			
			if (rom_bank_to_select == 0)
			{
				current_rom_bank = 1;
				return;
			}
			if (rom_bank_to_select > number_of_rom_banks)
			{
				current_rom_bank = (rom_bank_to_select & number_of_rom_banks);
				if (current_rom_bank == 8)
					NO_OP;
				return;
			}
			current_rom_bank = rom_bank_to_select;
			return;
		} // MBC1
	case 0x02: // MBC1 + RAM
	case 0x03: // MBC1 + RAM + battery
	case 0x05: // MBC2
	case 0x06: // MBC2 + BATTERY
	case 0x08: break; // ROM + RAM
	case 0x09: break; // ROM + RAM + BATTERY
	case 0x0B: break; // MMM01
	case 0x0C: break; // MMM01 + RAM
	case 0x0D: break; // MMM01 + RAM + BATTERY
	case 0x0F: break; // MBC3
	case 0x10: break; // MBC3 + RAM
	case 0x11: break;
	case 0x12: break;
	case 0x13: break;
	case 0x19: break;
	case 0x1A: break;
	case 0x1B: break;
	case 0x1C: break;
	case 0x1D: break;
	case 0x1E: break;
	case 0x20: break;
	case 0x22: break;
	case 0xFC: break;
	case 0xFD: break;
	case 0xFE: break;
	case 0xFF: break;
	default: fprintf(stderr, "Rom Bank number %i Change not implemented", cartridge_type);  exit(-1);
	}
}

void GamePak::cartridgeTypeInit()
{
	switch (cartridge_type)
	{
	case 0x00: current_rom_bank = 1; break; // ROM ONLY
	case 0x01: current_rom_bank = 1;// MBC1
	case 0x02: // MBC1 + RAM
	case 0x03:  break;// MBC1 + RAM + battery
	case 0x05: // MBC2
	case 0x06:  break;// MBC2 + BATTERY
	case 0x08: break; // ROM + RAM
	case 0x09: break; // ROM + RAM + BATTERY
	case 0x0B: break; // MMM01
	case 0x0C: break; // MMM01 + RAM
	case 0x0D: break; // MMM01 + RAM + BATTERY
	case 0x0F: break; // MBC3
	case 0x10: break; // MBC3 + RAM
	case 0x11: break;
	case 0x12: break;
	case 0x13: break;
	case 0x19: break;
	case 0x1A: break;
	case 0x1B: break;
	case 0x1C: break;
	case 0x1D: break;
	case 0x1E: break;
	case 0x20: break;
	case 0x22: break;
	case 0xFC: break;
	case 0xFD: break;
	case 0xFE: break;
	case 0xFF: break;
	default: fprintf(stderr, "Cartridge type %i INIT not implemented", cartridge_type);  exit(-1);
	}
}