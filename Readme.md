Emuboy - Nintendo Game Boy emulator
using C++ and SDL2

SDL directory must be added to

CMakePresets.json
eg.

"cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "SDL2_DIR": "C:/vclib/SDL2-2.26.5"
      }

for building on visual studio 2022