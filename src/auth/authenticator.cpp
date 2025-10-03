#include "kalshi/auth.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <cstring>
#include <memory>
#include <iomanip>
#include <sstream>
#include <vector>

namespace kalshi {
namespace auth {

Authenticator::Authenticator(const std::string& api_key, const std::string& private_key_pem)
    : api_key_(api_key), private_key_pem_(private_key_pem) {
    // If private key was provided with escaped newlines (e.g. from env var), convert them
    // so OpenSSL can parse the PEM correctly.
    size_t pos = 0;
    std::string normalized;
    normalized.reserve(private_key_pem_.size());
    while ((pos = private_key_pem_.find("\\n")) != std::string::npos) {
        normalized += private_key_pem_.substr(0, pos);
        normalized.push_back('\n');
        private_key_pem_.erase(0, pos + 2);
    }
    normalized += private_key_pem_;
    private_key_pem_ = normalized;
}


std::string Authenticator::get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return std::to_string(timestamp);
}

std::string Authenticator::sign_pss_message(const std::string& message) {
    BIO* bio = BIO_new_mem_buf(private_key_pem_.c_str(), -1);
    if (!bio) {
        spdlog::error("Failed to create BIO from private key");
        return "";
    }

    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    
    if (!pkey) {
        spdlog::error("Failed to load private key");
        return "";
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        // Clean up and log via helper
        if (pkey) EVP_PKEY_free(pkey);
        spdlog::error("Failed to create EVP_MD_CTX");
        return "";
    }

    // helper to consolidate cleanup + logging
    auto sign_error = [&](const std::string& err_msg, EVP_MD_CTX* ctx_ptr, EVP_PKEY* pkey_ptr) -> std::string {
        if (ctx_ptr) EVP_MD_CTX_free(ctx_ptr);
        if (pkey_ptr) EVP_PKEY_free(pkey_ptr);
        spdlog::error("{}", err_msg);
        return std::string();
    };

    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) {
        return sign_error("Failed to initialize digest signing", ctx, pkey);
    }

    // Set PSS padding
    EVP_PKEY_CTX* pkey_ctx = EVP_MD_CTX_pkey_ctx(ctx);
    if (EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, RSA_PKCS1_PSS_PADDING) <= 0 ||
        EVP_PKEY_CTX_set_rsa_pss_saltlen(pkey_ctx, RSA_PSS_SALTLEN_DIGEST) <= 0) {
        return sign_error("Failed to set PSS padding", ctx, pkey);
    }

    if (EVP_DigestSignUpdate(ctx, message.c_str(), message.length()) != 1) {
        return sign_error("Failed to update digest", ctx, pkey);
    }

    size_t signature_len = 0;
    if (EVP_DigestSignFinal(ctx, nullptr, &signature_len) != 1) {
        return sign_error("Failed to get signature length", ctx, pkey);
    }

    std::vector<unsigned char> signature(signature_len);
    if (EVP_DigestSignFinal(ctx, signature.data(), &signature_len) != 1) {
        return sign_error("Failed to sign message", ctx, pkey);
    }

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    // Base64 encode the signature
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    
    BIO_write(b64, signature.data(), signature_len);
    BIO_flush(b64);
    
    char* encoded_data;
    long encoded_len = BIO_get_mem_data(bmem, &encoded_data);
    
    std::string result(encoded_data, encoded_len);
    BIO_free_all(b64);
    
    return result;
}

std::string Authenticator::sign_request(const std::string& method, const std::string& path, const std::string& timestamp) {
    std::string message_to_sign = timestamp + method + path;
    return sign_pss_message(message_to_sign);
}

std::map<std::string, std::string> Authenticator::create_websocket_headers() {
    std::string timestamp = get_current_timestamp();
    std::string signature = sign_request("GET", "/trade-api/ws/v2", timestamp);
    
    std::map<std::string, std::string> headers;
    if (signature.empty()) {
        spdlog::error("Failed to create signature for websocket headers; refusing to return headers");
        return headers;
    }

    headers["KALSHI-ACCESS-KEY"] = api_key_;
    headers["KALSHI-ACCESS-SIGNATURE"] = signature;
    headers["KALSHI-ACCESS-TIMESTAMP"] = timestamp;

    spdlog::info("Created WebSocket authentication headers");
    return headers;
}

} // namespace auth
} // namespace kalshi