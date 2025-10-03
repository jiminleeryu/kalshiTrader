cmake_minimum_required(VERSION 3.20)
project(kalshiTrader VERSION 0.1.0 LANGUAGES CXX)

# Enable vcpkg toolchain manifest mode if using vcpkg
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "If you use vcpkg, set CMAKE_TOOLCHAIN_FILE to vcpkg toolchain file when configuring")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# If using vcpkg manifest mode, pass -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
if(DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Using CMake toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Use config packages provided by vcpkg (they become available when toolchain file is used)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
# uWebSockets removed; using ixwebsocket instead

# Sources
add_subdirectory(src)
