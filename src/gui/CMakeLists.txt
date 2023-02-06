project(gui)

file(GLOB_RECURSE  SRC_FILES *.cpp)

add_library(${PROJECT_NAME}
	${SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})


target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
