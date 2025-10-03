#pragma once

#include <string>
#include <map>
#include <functional>

namespace kalshi {
namespace websocket {

using MessageHandler = std::function<void(const std::string&)>;
using ConnectionHandler = std::function<void()>;
using DisconnectionHandler = std::function<void(int, const std::string&)>;

class WebSocketClient {
public:
    WebSocketClient();
    ~WebSocketClient();
    
    bool connect(const std::string& url, const std::map<std::string, std::string>& headers);
    void send(const std::string& message);
    void close();
    bool is_connected() const;
    
    void set_message_handler(MessageHandler handler);
    void set_connection_handler(ConnectionHandler handler);
    void set_disconnection_handler(DisconnectionHandler handler);
    
    void run();
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace websocket
} // namespace kalshi