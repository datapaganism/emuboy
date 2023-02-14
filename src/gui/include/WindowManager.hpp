#pragma once
#include <iostream>
#include <vector>

#include "SDL.h"
#include "Window.hpp"

class WindowManager
{
public:
    std::vector<std::unique_ptr<Window>> windows;
    WindowManager(const std::string rom_path, const std::string bios_path);
    ~WindowManager();
    
    void run();

    bool getInitSuccess();
private:
    bool init_success = false;
    bool fast_forward = false;
};