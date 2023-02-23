include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(cpu_test cpu_test.cpp "timer_test.cpp")

target_link_libraries(cpu_test
 PRIVATE
  GTest::GTest
  core)

add_test(cpu_gtests cpu_test)


add_subdirectory(SM83_tests)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


# Add tests
function(set_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} gtest gtest_main core)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(${TESTNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

endfunction()

set_test(cpu_sm83_tests cpu_sm83_test.cpp)