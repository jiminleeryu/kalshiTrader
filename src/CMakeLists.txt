add_library(kalshi_trader_lib
  receiver/client.cpp
  auth/authenticator.cpp
  auth/websocket_client.cpp
)

target_include_directories(kalshi_trader_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

find_package(ixwebsocket CONFIG REQUIRED)

target_link_libraries(kalshi_trader_lib
  PUBLIC
  PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    cpr::cpr
    ixwebsocket::ixwebsocket
)

add_executable(kalshi_trader_app main.cpp)
target_link_libraries(kalshi_trader_app PRIVATE kalshi_trader_lib)

install(TARGETS kalshi_trader_app kalshi_trader_lib
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
