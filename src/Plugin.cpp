#include <sstream>
#include <iostream>
#include <cstring>
#include <string>

#include <curl/curl.h>
#include "Plugin.hpp"

#ifdef LUA_DEFINE
# undef LUA_DEFINE
#endif
#define LUA_DEFINE(name) Define(#name, [](lua_State *L) -> int

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Plugin::Plugin()
{

	LUA_DEFINE(Get)
	{
        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
            curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            std::cout << readBuffer << std::endl;
        }

		return Lua::ReturnValues(L, 1);
	});
}
