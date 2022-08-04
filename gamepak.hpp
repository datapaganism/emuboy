#pragma once
#include <vector>
#include <string>

#include "config.hpp"

class GamePak
{
public:
	GamePak(const std::string filename);
	GamePak();


	std::vector<Byte> rom;
	bool gamepak_loaded = false;
private:
	
	std::string filename;


};

