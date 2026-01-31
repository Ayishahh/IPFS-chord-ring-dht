# ============================================================================
# IPFS Ring DHT Simulator - CMakeLists.txt
# ============================================================================
# Build with CMake:
#   mkdir build && cd build
#   cmake -G "MinGW Makefiles" ..
#   cmake --build .
# ============================================================================

cmake_minimum_required(VERSION 3.15)
project(IPFS_Ring_DHT_Simulator VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Source files
set(SOURCES
    src/main.cpp
)

# Header files (for IDE integration)
set(HEADERS
    src/BTree.h
    src/CircularLL.h
    src/DoublyLL.h
    src/IPFS.h
    src/Menu.h
    src/Queue.h
    src/SHA1.h
)

# Create executable
add_executable(ipfs_dht ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(ipfs_dht PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Set output directory
set_target_properties(ipfs_dht PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
)

# Create data directory for sample files
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/data/sample_files)

# Installation (optional)
install(TARGETS ipfs_dht DESTINATION bin)
install(DIRECTORY data/ DESTINATION share/ipfs_dht/data)

# Print configuration
message(STATUS "")
message(STATUS "IPFS Ring DHT Simulator Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Output Directory: ${CMAKE_SOURCE_DIR}/bin")
message(STATUS "")
