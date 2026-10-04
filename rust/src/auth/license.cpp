#include "license.hpp"
#include "PWFLicense.h"

#include <atomic>
#include <cctype>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <windows.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace
{
	std::atomic<bool> g_ok{ false };
	PWFLicense g_pwf;
	const std::string g_session_url = rustcfg::env( "PS_SESSION_URL", PS_SESSION_URL_DEFAULT );

	size_t WriteCb(void* contents, size_t size, size_t nmemb, std::string* out)
	{
		out->append((char*)contents, size * nmemb);
		return size * nmemb;
	}

	bool SiteSessionOk(const std::string& token)
	{
		if (token.empty()) return false;
		std::string response;
		CURL* curl = curl_easy_init();
		if (!curl) return false;
		struct curl_slist* headers = nullptr;
		headers = curl_slist_append(headers, "Accept: application/json");
		const std::string auth = "Authorization: Bearer " + token;
		headers = curl_slist_append(headers, auth.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, g_session_url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
#ifdef CURLSSLOPT_NATIVE_CA
		curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, (long)CURLSSLOPT_NATIVE_CA);
#endif
		const CURLcode rc = curl_easy_perform(curl);
		long status = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		if (rc != CURLE_OK || status >= 400 || response.empty())
			return false;
		try {
			json r = json::parse(response);
			if (!r.value("success", false)) return false;
			const json products = r.value("products", json::array());
			if (!products.is_array()) return false;
			for (const auto& p : products) {
				std::string slug = p.value("slug", "");
				for (char& c : slug) c = (char)tolower((unsigned char)c);
				if (slug.find("rust") != std::string::npos)
					return true;
			}
			return false;
		} catch (...) {
			return false;
		}
	}

	bool PwfOk(const std::string& key)
	{
		if (key.empty()) return false;
		try {
			json r = g_pwf.login(key);
			if (!r.value("success", false))
				r = g_pwf.loginAccount(key, key);
			return r.value("success", false);
		} catch (...) {
			return false;
		}
	}

	void Heartbeats(bool pwf, const std::string& token)
	{
		if (pwf)
		{
			std::thread([] {
				g_pwf.runHeartbeat([](const std::string&, const std::string&) {
					::TerminateProcess(::GetCurrentProcess(), 0);
				});
			}).detach();
		}

		if (token.empty()) return;
		std::thread([token] {
			while (g_ok.load()) {
				std::this_thread::sleep_for(std::chrono::seconds(12));
				if (!g_ok.load()) break;
				if (!SiteSessionOk(token))
					::TerminateProcess(::GetCurrentProcess(), 0);
			}
		}).detach();
	}
}

namespace license
{
	bool gate(const std::string& key, const std::string& token)
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);
		const bool pwf = PwfOk(key);
		const bool site = SiteSessionOk(token);
		if (!pwf && !site)
			return false;
		g_ok = true;
		Heartbeats(pwf, token);
		return true;
	}
}
