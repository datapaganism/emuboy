cmake_minimum_required(VERSION 3.24)

project(emuboy)

add_subdirectory(src)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_subdirectory(src/gui)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE core gui ${SDL2_LIBRARIES})
#set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE 1)


    




