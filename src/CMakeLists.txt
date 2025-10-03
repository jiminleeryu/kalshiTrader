add_library(kalshi_trader_lib
  receiver/client.cpp
)

target_include_directories(kalshi_trader_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(kalshi_trader_lib
  PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    cpr::cpr
)

add_executable(kalshi_trader_app main.cpp)
target_link_libraries(kalshi_trader_app PRIVATE kalshi_trader_lib)

install(TARGETS kalshi_trader_app kalshi_trader_lib
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
