#include "kalshi/client.h"
#include <spdlog/spdlog.h>

namespace kalshi {

Client::Client(const std::string& api_key, const std::string& api_secret)
    : api_key_(api_key), api_secret_(api_secret) {
    spdlog::info("Kalshi client created");
}

Client::~Client() {
    spdlog::info("Kalshi client destroyed");
}

double Client::get_balance() {
    spdlog::warn("get_balance() is a placeholder and returns 0");
    return 0.0;
}

} // namespace kalshi
