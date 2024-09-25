cmake_minimum_required(VERSION 3.15.3)
project(tests CXX C)

set(TESTS_DIR ${CMAKE_CURRENT_LIST_DIR})
cmake_path(GET TESTS_DIR PARENT_PATH ROOT_DIR)

# Check if the COVERAGE flag is set
option(COVERAGE "Enable coverage flags" OFF)
option(TEST_NUM "Enable test num" OFF)
if(COVERAGE EQUAL 1)
    message(STATUS "Code coverage enabled!")
    add_compile_options(-g -O0 --coverage)
    add_link_options(--coverage)
endif()

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
include_directories(PkgConfig::FFTW)

function(gen_test app_name test_file)
    # Create the executable target
    add_executable(${app_name} ${test_file})
    if (${app_name} STREQUAL "test_fft")
        target_include_directories(${app_name} PUBLIC ${ROOT_DIR}/Src/common)
        target_include_directories(${app_name} PUBLIC ${ROOT_DIR}/Src/platform/ubuntu/math)
        # target_sources(${app_name} PUBLIC )
        # ${ROOT_DIR}/Src/common/FFT.cpp ${ROOT_DIR}/Src/platform/stm32/math/arm_rfft_q15.c ${ROOT_DIR}/Src/platform/stm32/math/arm_mult_q15.c)
    endif()

    # Link libraries to the target
    target_link_libraries(${app_name} PkgConfig::FFTW)
    target_link_libraries(${app_name} GTest::gtest GTest::gtest_main)
endfunction()

# Define the path to the test files directory
set(UNIT_TESTS_DIR ${ROOT_DIR}/Tests)

# Generate test executables
gen_test(test_fft     ${UNIT_TESTS_DIR}/common/test_fft.cpp)
gen_test(test_algorithms ${UNIT_TESTS_DIR}/common/test_algorithms.cpp)
