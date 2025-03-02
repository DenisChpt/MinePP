#include "Auth.hpp"
#include <curl/curl.h>
#include <string>
#include <iostream>

namespace {

// Callback pour récupérer les données de réponse de CURL.
// Les données reçues sont ajoutées à la chaîne pointée par userp.
size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t totalSize = size * nmemb;
	std::string *response = static_cast<std::string*>(userp);
	response->append(static_cast<char*>(contents), totalSize);
	return totalSize;
}

} // namespace anonyme

namespace Auth {

bool getAccessToken(std::string &result, const std::string &username, const std::string &identityToken) {
	static const std::string url = "random/url";

	result.clear();

	CURL *curl = curl_easy_init();
	if (curl) {
		// Prépare les données POST au format "username=...&identity_token=..."
		std::string postData = "username=" + username + "&identity_token=" + identityToken;
		std::string response;
		long httpCode = 0;

		#ifdef _WIN32
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		#endif

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

		CURLcode code = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		curl_easy_cleanup(curl);

		if (code == CURLE_OK && httpCode == 200) {
			result = response;
			return true;
		}
	}
	return false;
}

} // namespace Auth
