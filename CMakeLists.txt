cmake_minimum_required(VERSION 3.24)

project(emuboy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_subdirectory(src)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_subdirectory(src/gui)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE core gui ${SDL2_LIBRARIES})
#set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE 1)

enable_testing()

add_subdirectory(tests)


add_executable(heap_vs_stack tests/heap_vs_stack.cpp)
target_link_libraries(heap_vs_stack PRIVATE core)


    




