cmake_minimum_required(VERSION 3.20)

# Create receiver executable
add_executable(receiver
  receiver/main.cpp
)

target_compile_features(receiver PUBLIC cxx_std_20)

# Prefer to link to packages only if they are available (CMake will error earlier if REQUIRED was used)
find_package(nlohmann_json QUIET CONFIG)
if(nlohmann_json_FOUND)
  target_link_libraries(receiver PRIVATE nlohmann_json::nlohmann_json)
endif()

find_package(spdlog QUIET CONFIG)
if(spdlog_FOUND)
  target_link_libraries(receiver PRIVATE spdlog::spdlog)
endif()

find_package(cpr QUIET CONFIG)
if(cpr_FOUND)
  target_link_libraries(receiver PRIVATE cpr::cpr)
endif()

find_package(ixwebsocket CONFIG REQUIRED)
target_link_libraries(receiver PRIVATE ixwebsocket::ixwebsocket)

target_include_directories(receiver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
