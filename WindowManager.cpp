#include "WindowManager.hpp"
#include "config.hpp"
#include "VRAMViewer.hpp"
#include "BGMapViewer.hpp"
#include "EmulatorWindow.hpp"

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
        // Create an EmulatorWindow object (holds the emulator and it's window) and then push it back to the Window array, however,
        // the window array is of a base type, due to polymorphism, the objects it holds are treated as a base type instead of derived ones.
        this->windows.push_back(std::make_unique<EmulatorWindow>(rom_path, bios_path, XRES, YRES, RES_SCALING, EMULATOR_WINDOW_TITLE, true));

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
        // Cast the base pointer back to dervied class so we can access BUS class element.
    EmulatorWindow* emulator_window_ptr = static_cast<EmulatorWindow*>(this->windows[0].get());
        
    //Create and push back secondary windows, and pass the new pointer so the other windows can acess the emulator's BUS.
    this->windows.push_back(std::make_unique<VRAMViewer>(emulator_window_ptr, (8 * 16), (8 * 24), 2, "VRAM Viewer", false));
    this->windows.push_back(std::make_unique<BGMapViewer>(emulator_window_ptr, (8 * 32), (8 * 32), 2, "BG Map Viewer", false));

    bool quit = false;
    bool pause = false;

    unsigned int ticks_now = 0, ticks_previous = 0;
    double tick_delta = 0;
    while (!quit)
    {
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
                switch (e.key.keysym.sym)
                {
                case SDLK_1:
                    this->windows[0].get()->focus();
                    break;

                case SDLK_2:
                    this->windows[1].get()->focus();
                    break;

                case SDLK_3:
                    this->windows[2].get()->focus();
                    break;

                case eJoypadButtonsDebug::pause:
                    pause = !pause;
                    break;
                }
            }
        }

        // tick at custom frequency

        ticks_now = SDL_GetTicks();
        tick_delta = ticks_now - ticks_previous;

        if (tick_delta > 1000 / VSYNC)
        {
            ticks_previous = ticks_now;

                // render each window
            if (!pause)
            {
                for (auto& window : this->windows)
                {
                    window.get()->updateState();
                    window.get()->render();
                }
            }
        }

        //bool allWindowsClosed = true;

        //for (auto& window : this->windows)
        //{
        //    if (window.get()->isShown())
        //        allWindowsClosed = false; break;
        //}

        ////if all windows closed
        //if (allWindowsClosed)
        //    quit = true;

        // if main window closed
        if (!this->windows[0].get()->isShown())
            quit = true;
    }
    
}

bool WindowManager::getInitSuccess()
{
    return this->init_success;
}
