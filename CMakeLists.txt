# CMakeList.txt : CMake project for CMakeExample, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("emuboy")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

#enable_cxx_compiler_flag_if_supported("-Wall")
#enable_cxx_compiler_flag_if_supported("-Wextra")
#enable_cxx_compiler_flag_if_supported("-pedantic")


# Add source to this project's executable.
add_executable (emuboy "main.cpp" "Window.cpp" "Window.hpp" "WindowManager.hpp" "WindowManager.cpp" "config.hpp" "bus.hpp" "bus.cpp" "cpu.hpp" "cpu.cpp" "dma_controller.hpp" "dma_controller.cpp" "fetcher.hpp" "fetcher.cpp" "fifo.hpp" "fifo.cpp" "FIFO_pixel.hpp" "gamepak.hpp" "gamepak.cpp" "joypad_mapping.hpp" "ppu.hpp" "ppu.cpp" "VRAMViewer.cpp" "VRAMViewer.hpp" "BGMapViewer.cpp" "BGMapViewer.hpp" "EmulatorWindow.cpp" "EmulatorWindow.hpp")
target_link_libraries(emuboy ${SDL2_LIBRARIES})

add_executable (emuboycpu "cputest.cpp" "config.hpp" "bus.hpp" "bus.cpp" "cpu.hpp" "cpu.cpp" "dma_controller.hpp" "dma_controller.cpp" "fetcher.hpp" "fetcher.cpp" "fifo.hpp" "fifo.cpp" "FIFO_pixel.hpp" "gamepak.hpp" "gamepak.cpp" "joypad_mapping.hpp" "ppu.hpp" "ppu.cpp")
target_link_libraries(emuboycpu ${SDL2_LIBRARIES})

add_executable (fifotest "fifotest.cpp" "config.hpp" "bus.hpp" "bus.cpp" "cpu.hpp" "cpu.cpp" "dma_controller.hpp" "dma_controller.cpp" "fetcher.hpp" "fetcher.cpp" "fifo.hpp" "fifo.cpp" "FIFO_pixel.hpp" "gamepak.hpp" "gamepak.cpp" "joypad_mapping.hpp" "ppu.hpp" "ppu.cpp")
target_link_libraries(fifotest ${SDL2_LIBRARIES})
# TODO: Add tests and install targets if needed.
