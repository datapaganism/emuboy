#include "gamepak.hpp"
#include <iostream>

GAMEPAK::GAMEPAK(const std::string filename) : GAMEPAK::GAMEPAK()
{
	this->filename = filename;
	std::ifstream file(this->filename, std::ios::binary | std::ios::ate);
	if(file.is_open())
	{
		std::ifstream::pos_type pos = file.tellg();

		//reallocate memory space to fit entire rom
		this->rom.resize(pos);

		file.seekg(0, std::ios::beg);
		file.read((char*)this->rom.data(), pos);		

		this->gamepakLoaded = true;

		file.close();
		return;
	}
	std::cout << "GAMEPAK NOT LOADED\n";
	file.close();

	this->allocate_ram();
}

GAMEPAK::GAMEPAK()
{
}

Byte GAMEPAK::get_cartridge_type()
{
	return this->rom[0x147];
}

Byte GAMEPAK::get_rom_size()
{
	return this->rom[0x148];
}

Byte GAMEPAK::get_ram_size()
{
	return this->rom[0x149];
}

Byte GAMEPAK::get_memory(const Word address)
{
	if (address <= 0x3FFF) // if address is within rom bank 0
		return this->rom[address];

	if (address <= 0x7FFF) // otherwise return calculate the address by the rom bank used and return that data
		return this->rom[(address - 0x4000) + (this->current_rom_bank * 0x4000)];

	if (address <= 0xBFFF)
		return this->rom[(address - 0x2000) + (this->current_ram_bank * 0x2000)];
}

void GAMEPAK::set_memory(const Word address, const Byte data)
{
	if (address < 0x2000) // enable ram bank writing
		this->ram_bank_enable_handler(address, data); return;

	if (address <= 0x4000) // rom bank change
		this->rom_bank_change(address, data); return;

	if (address <= 0x6000) // rom / ram bank change
		return;
}

void GAMEPAK::allocate_ram()
{
	if (this->get_cartridge_type() == 0x05 || this->get_cartridge_type() == 0x06)
	{
		this->ram = std::make_unique<Byte[]>(256);
		return;
	}

	Byte numOfBanks = this->get_ram_size();
	switch (numOfBanks)
	{
	case 0: break;
	case 1: break;
	case 2: this->ram = std::make_unique<Byte[]>(1 * 0x2000); break;
	case 3: this->ram = std::make_unique<Byte[]>(4 * 0x2000); break;
	case 4: this->ram = std::make_unique<Byte[]>(16 * 0x2000); break;
	case 5: this->ram = std::make_unique<Byte[]>(8 * 0x2000); break;
	default: throw "unreachable number of banks";
	}
}

void GAMEPAK::ram_bank_enable_handler(const Word address, const Byte data)
{
	switch (this->get_cartridge_type())
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
		if (!address & (0b1 << 4))
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
	}
}

void GAMEPAK::ram_bank_change(const Word address, const Byte data)
{
	switch (this->get_cartridge_type())
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
	}
}

void GAMEPAK::rom_bank_change(const Word address, const Byte data)
{
	switch (this->get_cartridge_type())
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
	}
}