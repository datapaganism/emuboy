project(core)

file(GLOB_RECURSE  SRC_FILES *.cpp)

add_library(${PROJECT_NAME}
	${SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
