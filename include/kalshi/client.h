#pragma once

#include <string>
#include <memory>
#include "kalshi/auth.h"
#include "kalshi/websocket_client.h"

namespace kalshi {

class Client {
public:
    Client(const std::string& api_key, const std::string& private_key);
    ~Client();

    void connect();
    void subscribe_to_ticker();
    void subscribe_to_orderbook(const std::string& market_ticker);
    void run();

private:
    std::string api_key_;
    std::string private_key_;
    std::unique_ptr<auth::Authenticator> authenticator_;
    std::unique_ptr<websocket::WebSocketClient> ws_client_;
    int message_id_;
    
    void on_message(const std::string& message);
    void on_connected();
    void on_disconnected(int code, const std::string& reason);
};

} // namespace kalshi
