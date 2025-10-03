#include "kalshi/client.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>

namespace kalshi {

Client::Client(const std::string& api_key, const std::string& private_key)
    : api_key_(api_key), private_key_(private_key), message_id_(1) {
    spdlog::info("Kalshi client created");
    
    authenticator_ = std::make_unique<auth::Authenticator>(api_key, private_key);
    ws_client_ = std::make_unique<websocket::WebSocketClient>();
    
    // Set up WebSocket event handlers
    ws_client_->set_message_handler([this](const std::string& msg) {
        on_message(msg);
    });
    
    ws_client_->set_connection_handler([this]() {
        on_connected();
    });
    
    ws_client_->set_disconnection_handler([this](int code, const std::string& reason) {
        on_disconnected(code, reason);
    });
}

Client::~Client() {
    spdlog::info("Kalshi client destroyed");
}

void Client::connect() {
    // Use the production Kalshi WebSocket endpoint per docs
    std::string url = "wss://api.kalshi.com/trade-api/ws/v2";

    spdlog::info("Connecting to Kalshi WebSocket with authentication");
    spdlog::info("API Key: {}", api_key_.substr(0, 8) + "...");
    spdlog::info("URL: {}", url);

    // Build authentication headers and initiate the websocket connection
    auto headers = authenticator_->create_websocket_headers();
    // If headers are empty, signature failed — abort connect attempt
    if (headers.empty()) {
        spdlog::error("Authentication headers are empty. Check your private key (PEM) and API key.");
        std::cout << "Authentication failed — not attempting WebSocket connection." << std::endl;
        return;
    }

    // Informational prints
    std::cout << "Authentication configured with API key: " << api_key_.substr(0, 8) << "..." << std::endl;
    std::cout << "Private key loaded (length: " << private_key_.length() << " chars)" << std::endl;

    // Call the websocket client's connect method (the client currently simulates behavior; we'll replace it next)
    bool ok = ws_client_->connect(url, headers);
    if (!ok) {
        spdlog::error("WebSocket client failed to start connection");
    }
}

void Client::subscribe_to_ticker() {
    if (!ws_client_->is_connected()) {
        spdlog::error("Not connected to WebSocket");
        return;
    }
    
    nlohmann::json subscription = {
        {"id", message_id_++},
        {"cmd", "subscribe"},
        {"params", {
            {"channels", nlohmann::json::array({"ticker"})}
        }}
    };
    
    ws_client_->send(subscription.dump());
}

void Client::subscribe_to_orderbook(const std::string& market_ticker) {
    if (!ws_client_->is_connected()) {
        spdlog::error("Not connected to WebSocket");
        return;
    }
    
    nlohmann::json subscription = {
        {"id", message_id_++},
        {"cmd", "subscribe"},
        {"params", {
            {"channels", nlohmann::json::array({"orderbook_delta"})},
            {"market_ticker", market_ticker}
        }}
    };
    
    ws_client_->send(subscription.dump());
}

void Client::on_message(const std::string& message) {
    std::cout << "\n=== LIVE KALSHI DATA ===" << std::endl;
    std::cout << "Raw: " << message << std::endl;
    
    try {
        auto data = nlohmann::json::parse(message);
        std::string msg_type = data.value("type", "unknown");
        
        if (msg_type == "ticker") {
            auto ticker_data = data["data"];
            std::cout << "TICKER UPDATE: " 
                      << ticker_data.value("market_ticker", "N/A") 
                      << " | Bid: $" << ticker_data.value("bid", 0.0)
                      << " | Ask: $" << ticker_data.value("ask", 0.0)
                      << " | Last: $" << ticker_data.value("last_price", 0.0)
                      << " | Vol: " << ticker_data.value("volume", 0) << std::endl;
        } else if (msg_type == "orderbook_delta") {
            auto book_data = data["data"];
            std::cout << "ORDERBOOK UPDATE: " << book_data.value("market_ticker", "N/A") << std::endl;
            if (book_data.contains("bids") && book_data["bids"].is_array()) {
                std::cout << "   Bids: ";
                for (const auto& bid : book_data["bids"]) {
                    if (bid.is_array() && bid.size() >= 2) {
                        std::cout << "$" << bid[0] << "x" << bid[1] << " ";
                    }
                }
                std::cout << std::endl;
            }
            if (book_data.contains("asks") && book_data["asks"].is_array()) {
                std::cout << "   Asks: ";
                for (const auto& ask : book_data["asks"]) {
                    if (ask.is_array() && ask.size() >= 2) {
                        std::cout << "$" << ask[0] << "x" << ask[1] << " ";
                    }
                }
                std::cout << std::endl;
            }
        } else if (msg_type == "subscribed") {
            std::cout << "SUBSCRIPTION CONFIRMED: " << data.value("channel", "unknown") << std::endl;
        } else if (msg_type == "error") {
            std::cout << "ERROR: " << message << std::endl;
        } else {
            std::cout << "ℹOTHER MESSAGE: " << msg_type << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Failed to parse message: " << e.what() << std::endl;
    }
    std::cout << "=========================" << std::endl;
}

void Client::on_connected() {
    std::cout << "\nConnected to Kalshi WebSocket!" << std::endl;
    // Allow configuring which channels/market to subscribe to via environment variables.
    // KALSHI_CHANNELS: comma-separated list, e.g. "ticker,orderbook_delta"
    // KALSHI_MARKET_TICKER: market ticker string to use with orderbook subscription
    const char* channels_env = std::getenv("KALSHI_CHANNELS");
    const char* market_env = std::getenv("KALSHI_MARKET_TICKER");

    if (channels_env) {
        std::string channels_str(channels_env);
        if (channels_str.find("ticker") != std::string::npos) {
            subscribe_to_ticker();
        }
        if (channels_str.find("orderbook_delta") != std::string::npos) {
            if (market_env) {
                subscribe_to_orderbook(std::string(market_env));
            } else {
                spdlog::warn("KALSHI_CHANNELS requests orderbook_delta but KALSHI_MARKET_TICKER is not set");
            }
        }
    } else {
        // Default behaviour: subscribe to ticker; if a market ticker is provided, subscribe to its orderbook
        subscribe_to_ticker();
        if (market_env) {
            subscribe_to_orderbook(std::string(market_env));
        }
    }
}

void Client::on_disconnected(int code, const std::string& reason) {
    std::cout << "\nDisconnected from Kalshi WebSocket (code: " << code << ", reason: " << reason << ")" << std::endl;
}

void Client::run() {
    std::cout << "\n=== KALSHI TRADING CLIENT ===" << std::endl;
    std::cout << "Setting up authenticated connection to Kalshi..." << std::endl;
    
    try {
        connect();
        std::cout << "Connection initiated. Waiting for live data..." << std::endl;

        // Let the websocket client run its loop and call our event handlers.
        // WebSocketClient::run() blocks until disconnected or should_stop is set.
        ws_client_->run();

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

}
