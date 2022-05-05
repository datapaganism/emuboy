# CMakeList.txt : CMake project for CMakeExample, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("emuboy")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Add source to this project's executable.
add_executable (emuboy "main.cpp" "Window.cpp" "Window.hpp" "WindowManager.hpp" "WindowManager.cpp" "config.hpp" "bus.hpp" "bus.cpp" "cpu.hpp" "cpu.cpp" "fetch_decode_execute.cpp" "dma_controller.hpp" "dma_controller.cpp" "fetcher.hpp" "fetcher.cpp" "fifo.hpp" "fifo.cpp" "FIFO_pixel.hpp" "gamepak.hpp" "gamepak.cpp" "joypad_mapping.hpp" "ppu.hpp" "ppu.cpp" "VRAMViewer.cpp" "VRAMViewer.hpp" "BGMapViewer.cpp" "BGMapViewer.hpp" "cpu_instructions.cpp")
target_link_libraries(emuboy ${SDL2_LIBRARIES})
# TODO: Add tests and install targets if needed.
