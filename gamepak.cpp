#include "gamepak.hpp"

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
	}
	file.close();
}

GAMEPAK::GAMEPAK()
{
}

