#include "WindowManager.hpp"
#include "config.hpp"
#include "Emulator.hpp"
#include "VRAMViewer.hpp"
#include "BGMapViewer.hpp"

WindowManager::WindowManager(const std::string rom_path, const std::string bios_path)
{
    this->mInitSuccess = true;


    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
        this->mInitSuccess = false;
    }
    else
    {

        //attempt making first window of type emulator
        this->windows.push_back(std::unique_ptr<Window>(std::make_unique<Emulator>(rom_path, bios_path,XRES, YRES, RES_SCALING, EMULATOR_WINDOW_TITLE, true)));
        if (!this->windows[0].get()->initSuccess())
        {
            std::cout << "Window 0 could not be created!\n";
            this->mInitSuccess = false;
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
    if (!this->mInitSuccess)
        std::cout << "Failed to initialize!\n";
    else
    {
        BUS* bus_ptr = static_cast<Emulator*>(this->windows[0].get())->getBusPtr();

        this->windows.push_back(std::unique_ptr<Window>(std::make_unique<VRAMViewer>(bus_ptr,(8 * 16), (8 * 24), 2, "VRAMViewer", false)));
        this->windows.push_back(std::unique_ptr<Window>(std::make_unique<BGMapViewer>(bus_ptr,(8 * 32), (8 * 32), 2, "BGMapViewer", false)));
        
        
        bool quit = false;

        
        unsigned int ticksNow = 0, ticksPrevious = 0;
        double tickDelta = 0;
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

                // switch windows
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
                    }
                }
            }
            ticksNow = SDL_GetTicks();
            tickDelta = ticksNow - ticksPrevious;

            if (tickDelta > 1000 / VSYNC)
            {
                ticksPrevious = ticksNow;

                // render each window
                for (auto& window : this->windows)
                {
                    window.get()->updateState();
                    window.get()->render();
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
}

bool WindowManager::initSuccess()
{
    return this->mInitSuccess;
}