#include <iostream>
#include <cstdlib>
#include "kalshi/client.h"
#include <fstream>
#include <string>
#include <sstream>

// Simple .env loader: reads lines KEY=VALUE (ignores comments and empty lines)
static void load_dotenv(const std::string& path = ".env") {
    std::ifstream f(path);
    if (!f) return;

    std::string line;
    while (std::getline(f, line)) {
        // trim
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        // strip possible surrounding quotes
        if (!value.empty() && ((value.front() == '"' && value.back() == '"') || (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }

        // set env (overwrite existing)
        setenv(key.c_str(), value.c_str(), 1);
    }
}

int main() {
    // Load .env if present so env-vars defined there are available to getenv
    load_dotenv();

    const char* api_key = std::getenv("KALSHI_API_KEY_ID");
    const char* private_key_env = std::getenv("KALSHI_PRIVATE_KEY");
    const char* private_key_file = std::getenv("KALSHI_PRIVATE_KEY_FILE");

    std::string private_key_value;
    if (private_key_file && private_key_file[0] != '\0') {
        std::ifstream kf(private_key_file);
        if (!kf) {
            std::cerr << "Failed to open private key file: " << private_key_file << std::endl;
            return 1;
        }
        std::ostringstream ss;
        ss << kf.rdbuf();
        private_key_value = ss.str();
    } else if (private_key_env && private_key_env[0] != '\0') {
        private_key_value = private_key_env;
    }

    if (!api_key || private_key_value.empty()) {
        std::cerr << "Missing KALSHI_API_KEY_ID or KALSHI_PRIVATE_KEY (or KALSHI_PRIVATE_KEY_FILE) environment variables" << std::endl;
        return 1;
    }

    kalshi::Client client(api_key, private_key_value);
    client.run();
    
    return 0;
}
