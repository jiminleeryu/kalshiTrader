#pragma once

#include <string>

namespace kalshi {

class Client {
public:
    Client(const std::string& api_key, const std::string& api_secret);
    ~Client();

    // Example: get account balance (placeholder)
    double get_balance();

private:
    std::string api_key_;
    std::string api_secret_;
};

} // namespace kalshi
