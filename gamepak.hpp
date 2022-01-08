#pragma once
#include <fstream>
#include <vector>

#include "config.hpp"

class GAMEPAK
{
public:
	GAMEPAK(const std::string filename);
	GAMEPAK();


	std::vector<Byte> rom;
	bool gamepakLoaded = false;
private:
	
	std::string filename;


};

