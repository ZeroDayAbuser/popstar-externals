#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <ctime>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <functional>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <nlohmann/json.hpp>

// ── Local secret overrides (NOT committed — see .gitignore) ──────────────
#if defined(__has_include)
#  if __has_include("secrets.local.h")
#    include "secrets.local.h"
#  endif
#endif
#ifndef PWF_BASE_URL_DEFAULT
#  define PWF_BASE_URL_DEFAULT ""
#endif
#ifndef PWF_APP_SECRET_DEFAULT
#  define PWF_APP_SECRET_DEFAULT ""
#endif
#ifndef PWF_APP_ID_DEFAULT
#  define PWF_APP_ID_DEFAULT ""
#endif
#ifndef PS_SESSION_URL_DEFAULT
#  define PS_SESSION_URL_DEFAULT ""
#endif

namespace rustcfg {
    inline std::string env(const char* name, const char* fallback) {
        const char* v = std::getenv(name);
        return (v && *v) ? std::string(v) : std::string(fallback);
    }
}

using json = nlohmann::json;

class CryptoEnvelope {
    std::vector<unsigned char> encKey, macKey;
    static const int MAX_DRIFT = 300;

    static std::vector<unsigned char> sha256Bytes(const std::string& s) {
        std::vector<unsigned char> out(SHA256_DIGEST_LENGTH);
        SHA256(reinterpret_cast<const unsigned char*>(s.data()), s.size(), out.data());
        return out;
    }

    static std::string base64Encode(const std::vector<unsigned char>& data) {
        static const char* tbl =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        int val = 0, bits = -6;
        for (unsigned char c : data) {
            val = (val << 8) + c; bits += 8;
            while (bits >= 0) { out.push_back(tbl[(val >> bits) & 0x3F]); bits -= 6; }
        }
        if (bits > -6) out.push_back(tbl[((val << 8) >> (bits + 8)) & 0x3F]);
        while (out.size() % 4) out.push_back('=');
        return out;
    }

    static std::vector<unsigned char> base64Decode(const std::string& in) {
        static const char* tbl =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) T[(unsigned char)tbl[i]] = i;
        std::vector<unsigned char> out;
        int val = 0, bits = -8;
        for (unsigned char c : in) {
            if (T[c] == -1) break;
            val = (val << 6) + T[c]; bits += 6;
            if (bits >= 0) { out.push_back((unsigned char)((val >> bits) & 0xFF)); bits -= 8; }
        }
        return out;
    }

    std::string hmacHex(const std::string& msg) {
        unsigned char h[EVP_MAX_MD_SIZE];
        unsigned int len = 0;
        HMAC(EVP_sha256(), macKey.data(), (int)macKey.size(),
             reinterpret_cast<const unsigned char*>(msg.data()), msg.size(), h, &len);
        static const char* hex = "0123456789abcdef";
        std::string out;
        for (unsigned int i = 0; i < len; i++) {
            out.push_back(hex[h[i] >> 4]);
            out.push_back(hex[h[i] & 0xF]);
        }
        return out;
    }

public:
    CryptoEnvelope(const std::string& appSecret) {
        encKey = sha256Bytes("enc:" + appSecret);
        macKey = sha256Bytes("mac:" + appSecret);
    }

    std::string encrypt(const json& data) {
        std::string plain = data.dump();
        unsigned char iv[16];
        if (RAND_bytes(iv, 16) != 1) throw std::runtime_error("RAND_bytes failed");

        std::vector<unsigned char> ct(plain.size() + 32);
        int len = 0, total = 0;
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        bool ok = EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, encKey.data(), iv) == 1
               && EVP_EncryptUpdate(ctx, ct.data(), &len,
                      reinterpret_cast<const unsigned char*>(plain.data()), (int)plain.size()) == 1;
        total = len;
        ok = ok && EVP_EncryptFinal_ex(ctx, ct.data() + total, &len) == 1;
        total += len;
        EVP_CIPHER_CTX_free(ctx);
        if (!ok) throw std::runtime_error("AES encryption failed");
        ct.resize(total);

        std::vector<unsigned char> combined(iv, iv + 16);
        combined.insert(combined.end(), ct.begin(), ct.end());
        std::string p = base64Encode(combined);
        long t = (long)time(nullptr);
        std::string s = hmacHex(p + std::to_string(t));

        json env = { {"p", p}, {"t", t}, {"s", s} };
        return env.dump();
    }

    json decrypt(const std::string& envelopeJson) {
        json env = json::parse(envelopeJson);
        if (!env.contains("p") || !env.contains("t") || !env.contains("s"))
            throw std::runtime_error("Invalid envelope format");

        std::string p = env["p"];
        long t = env["t"];
        std::string s = env["s"];

        std::string expected = hmacHex(p + std::to_string(t));
        if (expected.size() != s.size() ||
            CRYPTO_memcmp(expected.data(), s.data(), expected.size()) != 0)
            throw std::runtime_error("HMAC verification failed");
        if (std::abs((long)time(nullptr) - t) > MAX_DRIFT)
            throw std::runtime_error("Request expired (replay protection) - check the system clock");

        std::vector<unsigned char> combined = base64Decode(p);
        if (combined.size() <= 16) throw std::runtime_error("Malformed ciphertext");
        unsigned char iv[16];
        std::copy(combined.begin(), combined.begin() + 16, iv);
        std::vector<unsigned char> ctData(combined.begin() + 16, combined.end());

        std::vector<unsigned char> plain(ctData.size() + 32);
        int len = 0, total = 0;
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        bool ok = EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, encKey.data(), iv) == 1
               && EVP_DecryptUpdate(ctx, plain.data(), &len, ctData.data(), (int)ctData.size()) == 1;
        total = len;
        ok = ok && EVP_DecryptFinal_ex(ctx, plain.data() + total, &len) == 1;
        total += len;
        EVP_CIPHER_CTX_free(ctx);
        if (!ok) throw std::runtime_error("AES decryption failed (bad key or corrupted payload)");
        plain.resize(total);

        return json::parse(std::string(plain.begin(), plain.end()));
    }
};

class PWFLicense {
    std::string baseUrl   = rustcfg::env("PWF_BASE_URL",   PWF_BASE_URL_DEFAULT);
    std::string appSecret = rustcfg::env("PWF_APP_SECRET", PWF_APP_SECRET_DEFAULT);
    std::string appId     = rustcfg::env("PWF_APP_ID",     PWF_APP_ID_DEFAULT);
    CryptoEnvelope crypto;
    std::string sessionId;
    std::string licenseKey;
    int heartbeatInterval = 30;

    static size_t writeCb(void* contents, size_t size, size_t nmemb, std::string* out) {
        out->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    json parseReply(const std::string& raw, long status) {
        if (raw.find_first_not_of(" \t\r\n") == std::string::npos)
            throw std::runtime_error("License server returned HTTP " + std::to_string(status)
                                     + " with an empty body.");
        json probe;
        try { probe = json::parse(raw); }
        catch (...) {
            throw std::runtime_error("License server returned HTTP " + std::to_string(status)
                                     + " with a non-JSON body. Check the API base URL.");
        }
        if (probe.contains("p") && probe.contains("t") && probe.contains("s"))
            return crypto.decrypt(raw);
        if (status >= 400 && !probe.contains("success"))
            throw std::runtime_error("License server returned HTTP " + std::to_string(status) + ".");
        return probe;
    }

    json post(const std::string& endpoint, const json& body) {
        std::string encrypted = crypto.encrypt(body);
        std::string response;
        long status = 0;
        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("curl_easy_init failed");
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("X-App-Secret: " + appSecret).c_str());
        curl_easy_setopt(curl, CURLOPT_URL, (baseUrl + endpoint).c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, encrypted.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        if (rc != CURLE_OK)
            throw std::runtime_error(std::string("Network error: ") + curl_easy_strerror(rc));
        return parseReply(response, status);
    }

public:
    PWFLicense() : crypto(appSecret) {}

    std::string getHWID() {
#ifdef _WIN32
        FILE* pipe = _popen(
            "powershell -NoProfile -Command \"(Get-CimInstance Win32_BaseBoard).SerialNumber\"", "r");
#else
        FILE* pipe = popen("cat /etc/machine-id 2>/dev/null || hostname", "r");
#endif
        if (!pipe) return "unknown-hwid";
        char buffer[256];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe)) result += buffer;
#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        const char* ws = " \r\n\t";
        size_t b = result.find_first_not_of(ws);
        if (b == std::string::npos) return "unknown-hwid";
        size_t e = result.find_last_not_of(ws);
        return result.substr(b, e - b + 1);
    }

    json login(const std::string& key) {
        json result = post("/api/auth/login.php", {
            {"license_key", key}, {"hwid", getHWID()}
        });
        if (result.value("success", false)) {
            sessionId = result.value("session_id", "");
            licenseKey = key;
            heartbeatInterval = result.value("heartbeat_interval", 30);
        }
        return result;
    }

    json checkKey(const std::string& key) {
        return post("/api/auth/check-key.php", {{"license_key", key}});
    }

    json heartbeat() {
        if (sessionId.empty()) return nullptr;
        return post("/api/auth/heartbeat.php", {
            {"session_id", sessionId}, {"license_key", licenseKey}
        });
    }

    static const int MAX_HEARTBEAT_FAILURES = 3;

    void runHeartbeat(std::function<void(const std::string&, const std::string&)> onRevoked) {
        const std::vector<std::string> kill = {
            "BANNED", "PAUSED", "EXPIRED", "HWID_RESET", "MAINTENANCE",
            "SESSION_REVOKED", "SESSION_EXPIRED", "SESSION_MISMATCH"};
        int failures = 0;
        while (!sessionId.empty()) {
            std::this_thread::sleep_for(std::chrono::seconds(heartbeatInterval));
            json r;
            bool failed = false;
            try { r = heartbeat(); }
            catch (...) { failed = true; }
            if (failed || r.is_null()) {
                if (sessionId.empty()) return;
                if (++failures >= MAX_HEARTBEAT_FAILURES) {
                    sessionId.clear();
                    onRevoked("NETWORK_LOST",
                        "Cannot reach the license server. Please check your connection and sign in again.");
                    return;
                }
                continue;
            }
            failures = 0;
            if (r.value("success", false)) continue;
            std::string code = r.value("error_code", "");
            if (std::find(kill.begin(), kill.end(), code) != kill.end()) {
                sessionId.clear();
                onRevoked(code, r.value("message", ""));
                return;
            }
        }
    }

    json get(const std::string& endpoint, const std::string& bearerKey = "") {
        std::string response;
        long status = 0;
        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("curl_easy_init failed");
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("X-App-Secret: " + appSecret).c_str());
        if (!bearerKey.empty())
            headers = curl_slist_append(headers, ("Authorization: Bearer " + bearerKey).c_str());
        curl_easy_setopt(curl, CURLOPT_URL, (baseUrl + endpoint).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        if (rc != CURLE_OK)
            throw std::runtime_error(std::string("Network error: ") + curl_easy_strerror(rc));
        return parseReply(response, status);
    }

    json postPlain(const std::string& endpoint, const json& body) {
        std::string payload = body.dump();
        std::string response;
        long status = 0;
        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("curl_easy_init failed");
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("X-App-Secret: " + appSecret).c_str());
        curl_easy_setopt(curl, CURLOPT_URL, (baseUrl + endpoint).c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        if (rc != CURLE_OK)
            throw std::runtime_error(std::string("Network error: ") + curl_easy_strerror(rc));
        return parseReply(response, status);
    }

    json getAppInfo() {
        return get("/api/app/info.php");
    }

    json getTexts() {
        return get("/api/app/text.php", licenseKey);
    }

    json getSlides() {
        return post("/api/app/slides.php", {{"action", "get_slides"}});
    }

    json checkUpdate(const std::string& currentVersion) {
        return post("/api/update/check.php", {{"v", currentVersion}, {"channel", "stable"}, {"hwid", getHWID()}, {"license_key", licenseKey}});
    }

    json trackSocialClick(int linkId) {
        return post("/api/app/social-click.php", {{"link_id", linkId}});
    }

    json createTrial() {
        return postPlain("/api/auth/trial.php", {{"hwid", getHWID()}});
    }

    json requestHwidReset(const std::string& reason) {
        return postPlain("/api/auth/request-hwid-reset.php", {{"license_key", licenseKey}, {"reason", reason}});
    }

    json registerAccount(const std::string& username, const std::string& password, const std::string& email) {
        return postPlain("/api/auth/account-register.php", {{"username", username}, {"password", password}, {"email", email}});
    }

    json loginAccount(const std::string& username, const std::string& password) {
        json result = postPlain("/api/auth/account-login.php", {{"username", username}, {"password", password}, {"hwid", getHWID()}});
        if (result.value("success", false)) {
            sessionId = result.value("session_id", "");
            if (result.contains("license_key")) licenseKey = result.value("license_key", "");
        }
        return result;
    }

    json changeAccountPassword(const std::string& username, const std::string& currentPassword, const std::string& newPassword) {
        return postPlain("/api/auth/change-password.php", {{"username", username}, {"current_password", currentPassword}, {"new_password", newPassword}});
    }

    json getPricing() {
        return get("/api/app/pricing.php?app_id=" + appId);
    }

    json logout() {
        if (sessionId.empty()) return nullptr;
        json result = post("/api/auth/logout.php", {
            {"session_id", sessionId}, {"license_key", licenseKey}
        });
        sessionId.clear();
        return result;
    }
};
