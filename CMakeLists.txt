cmake_minimum_required(VERSION 3.10)
project(UE_Simulator_5G VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/pdcp_layer.cpp
    src/rlc_layer.cpp
    src/mac_layer.cpp
    src/ue_simulator.cpp
)

# Library sources (without main)
set(LIB_SOURCES
    src/pdcp_layer.cpp
    src/rlc_layer.cpp
    src/mac_layer.cpp
    src/ue_simulator.cpp
)

# Main executable
add_executable(ue_simulator ${SOURCES})

# Threading library
find_package(Threads REQUIRED)
target_link_libraries(ue_simulator PRIVATE Threads::Threads)

# Google Test
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    # Download and configure Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/release-1.12.1.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    enable_testing()
    
    # Test executable
    add_executable(ue_simulator_tests
        tests/test_pdcp_layer.cpp
        tests/test_rlc_layer.cpp
        tests/test_mac_layer.cpp
        tests/test_integration.cpp
        ${LIB_SOURCES}
    )
    
    target_link_libraries(ue_simulator_tests
        PRIVATE
        GTest::gtest_main
        Threads::Threads
    )
    
    include(GoogleTest)
    gtest_discover_tests(ue_simulator_tests)
    
    message(STATUS "Unit tests enabled")
endif()

# Build information
message(STATUS "===========================================")
message(STATUS "UE Simulator 5G/6G - 3GPP Compliant")
message(STATUS "===========================================")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "===========================================")
