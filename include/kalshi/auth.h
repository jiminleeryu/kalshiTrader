#pragma once

#include <string>
#include <map>

namespace kalshi {
namespace auth {

class Authenticator {
public:
    Authenticator(const std::string& api_key, const std::string& private_key_pem);
    
    std::map<std::string, std::string> create_websocket_headers();
    std::string sign_request(const std::string& method, const std::string& path, const std::string& timestamp);
    
private:
    std::string api_key_;
    std::string private_key_pem_;
    
    std::string get_current_timestamp();
    std::string sign_pss_message(const std::string& message);
};

} // namespace auth
} // namespace kalshi