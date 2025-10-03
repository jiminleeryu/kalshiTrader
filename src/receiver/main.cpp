// Auth helpers
#include "auth.hpp"

#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketHttpHeaders.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <thread>
#include <chrono>

int main() {
	std::string api_key = get_env_value(".env", "KALSHI_API_KEY_ID");
	std::string pem_path = ".secrets/kalshi_private.pem";

	std::string timestamp = get_timestamp_ms();

	// build message string (timestamp + method + path)
	std::string method = "GET";
	std::string path = "/trade-api/ws/v2";
	std::string msg_string = timestamp + method + path;

	// sign message string with RSA-PSS
	std::string signature = sign_message_rsa_pss(pem_path, msg_string);

	std::cout << "API Key: " << api_key << std::endl;
	std::cout << "Timestamp: " << timestamp << std::endl;
	std::cout << "Signature: " << signature << std::endl;
	std::cout << "Message string: " << msg_string << std::endl;

	// set headers for HTTP request (or WebSocket)
	ix::WebSocket webSocket;
	webSocket.setUrl("wss://api.elections.kalshi.com/trade-api/ws/v2");
	ix::WebSocketHttpHeaders headers = {
		{"KALSHI-ACCESS-KEY", api_key},
		{"KALSHI-ACCESS-SIGNATURE", signature},
		{"KALSHI-ACCESS-TIMESTAMP", timestamp}
	};
	webSocket.setExtraHeaders(headers);

	// Register message handler
	webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
		if (msg->type == ix::WebSocketMessageType::Open) {
			std::cout << "Connected to Kalshi WebSocket" << std::endl;
		} else if (msg->type == ix::WebSocketMessageType::Message) {
			std::cout << "Received: " << msg->str << std::endl;
		} else if (msg->type == ix::WebSocketMessageType::Error) {
			std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
		}
	});

	webSocket.start();

	std::this_thread::sleep_for(std::chrono::seconds(10));
	webSocket.stop();
	return 0;
}
