#include "WindowManager.hpp"
#include "../../core/include/config.hpp"
#include "VRAMViewer.hpp"
#include "BGMapViewer.hpp"
#include "EmulatorWindow.hpp"
#include "joypad.hpp"



WindowManager::WindowManager(const std::string rom_path, const std::string bios_path)
{
    this->init_success = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
        this->init_success = false;
    }
    else
    {
        // Create an EmulatorWindow object (holds the emulator and it's window) and then push it back to the Window array
        windows.push_back(std::make_unique<EmulatorWindow>(rom_path, bios_path, XRES, YRES, RES_SCALING, EMULATOR_WINDOW_TITLE, true));

        if (!this->windows[0].get()->initSuccess())
        {
            std::cout << "Window 0 could not be created!\n";
            this->init_success = false;
        }
    }
}

WindowManager::~WindowManager()
{
    std::cout << "WindowManager destructor called\n";
    this->windows.clear();

    SDL_Quit();
}


void WindowManager::run()
{
    if (!this->init_success)
    {
        std::cout << "Failed to initialize!\n";
        return;
    }
    // Cast the Window pointer back to dervied class so we can access BUS class element.
    BUS* bus_ptr = (EmulatorWindow*)(windows[0].get());
        
    //Create and push back secondary windows, and pass the new pointer so the other windows can acess the emulator's BUS.
    windows.push_back(std::make_unique<VRAMViewer>(bus_ptr, (8 * 16), (8 * 24), 2, "VRAM Viewer", false));
    windows.push_back(std::make_unique<BGMapViewer>(bus_ptr, (8 * 32), (8 * 32), 2, "BG Map Viewer", false));

    bool quit = false;
    bool pause = false;

    uint64_t frame_start = 0;
    uint64_t frame_end = 0;
    double frame_time = 0;

    while (!quit)
    {
        frame_start = SDL_GetTicks64();

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }

            // handle each window's window event
            for (auto& window : this->windows)
                window.get()->handleWindowEvent(e);

            // handle each window's actual events (like keypresses etc)
            for (auto& window : this->windows)
                window.get()->handleEvent(e);

            // switch windows and perform other functions
            if (e.type == SDL_KEYDOWN)
            {
                // if it is num key 1 to X, X being the total number of windows
                if (e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym < SDLK_1 + windows.size())
                {
                    windows[e.key.keysym.sym - SDLK_1].get()->focus();
                }
                switch (e.key.keysym.sym)
                {
                case eJoypadButtonsDebug::pause:
                    pause = !pause;
                    break;
                case eJoypadButtonsDebug::fast_forward:
                    fast_forward = !fast_forward;
                    break;
                }
            }
            if (e.type == SDL_KEYUP)
            {
                
                switch (e.key.keysym.sym)
                {
                case SDLK_EQUALS:
                    for (auto& window : windows)
                        window.get()->updateScaling(true);
                    break;
                case SDLK_MINUS:
                    for (auto& window : windows)
                        window.get()->updateScaling(false);
                    break;
                }
            }
        }

        if (!pause)
        {
            for (auto& window : this->windows)
            {
                window.get()->updateState();
                window.get()->render();
            }
        }

        if (!this->windows[0].get()->isShown())
            quit = true;

        frame_end = SDL_GetTicks64() - frame_start;


        if (!fast_forward && frame_end < FRAMETIME)
        {
            SDL_Delay(FRAMETIME - frame_end);
        }
    }
}

bool WindowManager::getInitSuccess()
{
    return this->init_success;
}
