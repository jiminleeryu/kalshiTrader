#include "kalshi/websocket_client.h"
#include <spdlog/spdlog.h>
#include <ixwebsocket/IXWebSocket.h>
#include <thread>
#include <atomic>

namespace kalshi {
namespace websocket {

struct WebSocketClient::Impl {
    std::string url;
    std::map<std::string, std::string> headers;
    std::atomic<bool> connected{false};
    std::atomic<bool> should_stop{false};

    MessageHandler message_handler;
    ConnectionHandler connection_handler;
    DisconnectionHandler disconnection_handler;

    std::unique_ptr<ix::WebSocket> ws;
    std::thread worker_thread;
    
    void connect_to_kalshi();
};

WebSocketClient::WebSocketClient() : impl_(std::make_unique<Impl>()) {}

WebSocketClient::~WebSocketClient() {
    impl_->should_stop = true;
    if (impl_->ws) {
        impl_->ws->stop();
    }
    if (impl_->worker_thread.joinable()) impl_->worker_thread.join();
}

bool WebSocketClient::connect(const std::string& url, const std::map<std::string, std::string>& headers) {
    impl_->url = url;
    impl_->headers = headers;

    spdlog::info("Connecting to WebSocket: {}", url);
    for (const auto& [key, value] : headers) {
        spdlog::info("Header: {}: {}", key, value.substr(0, std::min<size_t>(value.size(), 20)) + "...");
    }

    impl_->worker_thread = std::thread([this]() { impl_->connect_to_kalshi(); });
    return true;
}

void WebSocketClient::send(const std::string& message) {
    if (impl_->connected && impl_->ws) {
        spdlog::info("Sending WebSocket message: {}", message);
        impl_->ws->send(message);
    } else {
        spdlog::error("Cannot send message - not connected");
    }
}

void WebSocketClient::close() {
    impl_->should_stop = true;
    if (impl_->ws) impl_->ws->stop();
    impl_->connected = false;
}

bool WebSocketClient::is_connected() const {
    return impl_->connected;
}

void WebSocketClient::set_message_handler(MessageHandler handler) {
    impl_->message_handler = handler;
}

void WebSocketClient::set_connection_handler(ConnectionHandler handler) {
    impl_->connection_handler = handler;
}

void WebSocketClient::set_disconnection_handler(DisconnectionHandler handler) {
    impl_->disconnection_handler = handler;
}

void WebSocketClient::run() {
    while (!impl_->should_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void WebSocketClient::Impl::connect_to_kalshi() {
    spdlog::info("Starting ixwebsocket connection to Kalshi: {}", url);

    ws = std::make_unique<ix::WebSocket>();
    ws->setUrl(url);

    // Add headers (ixwebsocket expects a WebSocketHttpHeaders map)
    ix::WebSocketHttpHeaders extraHeaders;
    for (const auto& [k, v] : headers) {
        extraHeaders[k] = v;
    }
    ws->setExtraHeaders(extraHeaders);

    ws->setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            if (message_handler) message_handler(msg->str);
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            spdlog::info("WebSocket opened");
            connected = true;
            if (connection_handler) connection_handler();
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            // Log close info in detail (code and reason are available)
            spdlog::info("WebSocket closed (code: {}, reason: {})",
                         msg->closeInfo.code, msg->closeInfo.reason);
            connected = false;
            if (disconnection_handler) disconnection_handler(msg->closeInfo.code, msg->closeInfo.reason);
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            // ixwebsocket provides errorInfo which contains fields like http_status and reason
            try {
                const auto& ei = msg->errorInfo;
                spdlog::error("WebSocket error: http_status={}, reason={}, retries={}, wait_time={}, decompressionError={}",
                              ei.http_status, ei.reason, ei.retries, ei.wait_time, ei.decompressionError);
            } catch (const std::exception& e) {
                spdlog::error("WebSocket error: exception while logging errorInfo: {}", e.what());
            }

            connected = false;
            if (disconnection_handler) disconnection_handler(-1, "WebSocket error");
        }
    });

    // Start the socket (this runs internal threads)
    ws->start();

    // Wait while running and not asked to stop
    while (!should_stop) {
        if (!connected) {
            // Not connected yet; sleep and wait for open or stop
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            // Connected: keep the thread alive until stop requested
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    // Stop the socket cleanly
    if (ws) {
        ws->stop();
    }

    connected = false;
    if (disconnection_handler) disconnection_handler(1000, "Client stopped");
}

} // namespace websocket
} // namespace kalshi