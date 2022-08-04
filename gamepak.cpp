#include "gamepak.hpp"
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

		this->gamepak_loaded = true;

		file.close();
		return;
	}
	std::cout << "GamePak NOT LOADED\n";
	file.close();
}

GamePak::GamePak()
{
}