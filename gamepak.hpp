#pragma once

#include <fstream>
#include <vector>
#include "config.h"

class GAMEPAK
{
public:
	GAMEPAK(const std::string filename);
	GAMEPAK();


	std::vector<Byte> rom;
private:
	
	std::string filename;

};

