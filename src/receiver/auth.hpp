#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <string>
#include <vector>
#include <chrono>

// Helper: parse .env for key-value
inline std::string get_env_value(const std::string& filename, const std::string& key) {
    std::ifstream f(filename);
    std::string line;
    while (std::getline(f, line)) {
        if (line.find(key + "=") == 0) {
            return line.substr(key.size() + 1);
        }
    }
    return "";
}

// Helper: get current timestamp in ms
inline std::string get_timestamp_ms() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    return std::to_string(ms);
}


// Helper: base64 encode
inline std::string to_base64(const unsigned char* data, size_t len) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data, len);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return result;
}


// Minimal RSA-PSS signature using OpenSSL
inline std::string sign_message_rsa_pss(const std::string& pem_path, const std::string& message) {
    FILE* fp = fopen(pem_path.c_str(), "r");
    if (!fp) { std::cerr << "Cannot open PEM file: " << pem_path << std::endl; return ""; }
    EVP_PKEY* pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);
    if (!pkey) { std::cerr << "Cannot read private key from PEM" << std::endl; return ""; }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) { EVP_PKEY_free(pkey); std::cerr << "EVP_MD_CTX_new failed" << std::endl; return ""; }

    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) <= 0) {
        EVP_MD_CTX_free(ctx); EVP_PKEY_free(pkey); std::cerr << "DigestSignInit failed" << std::endl; return "";
    }
    if (EVP_PKEY_CTX_set_rsa_padding(EVP_MD_CTX_pkey_ctx(ctx), RSA_PKCS1_PSS_PADDING) <= 0) {
        EVP_MD_CTX_free(ctx); EVP_PKEY_free(pkey); std::cerr << "Set RSA PSS padding failed" << std::endl; return "";
    }
    if (EVP_PKEY_CTX_set_rsa_pss_saltlen(EVP_MD_CTX_pkey_ctx(ctx), -1) <= 0) {
        EVP_MD_CTX_free(ctx); EVP_PKEY_free(pkey); std::cerr << "Set saltlen failed" << std::endl; return "";
    }

    size_t siglen = 0;
    if (EVP_DigestSign(ctx, nullptr, &siglen, (const unsigned char*)message.data(), message.size()) <= 0) {
        EVP_MD_CTX_free(ctx); EVP_PKEY_free(pkey); std::cerr << "DigestSign (size) failed" << std::endl; return "";
    }
    std::vector<unsigned char> sig(siglen);
    if (EVP_DigestSign(ctx, sig.data(), &siglen, (const unsigned char*)message.data(), message.size()) <= 0) {
        EVP_MD_CTX_free(ctx); EVP_PKEY_free(pkey); std::cerr << "DigestSign failed" << std::endl; return "";
    }
    EVP_MD_CTX_free(ctx); EVP_PKEY_free(pkey);
    return to_base64(sig.data(), siglen);
}