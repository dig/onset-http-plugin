#include <sstream>
#include <iostream>
#include <cstring>
#include <string>
#include <map>

#include <curl/curl.h>
#include "Plugin.hpp"

#ifdef LUA_DEFINE
# undef LUA_DEFINE
#endif
#define LUA_DEFINE(name) Define(#name, [](lua_State *L) -> int

size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

static size_t headerFunction(void* pData, size_t tSize, size_t tCount, void* pmUser)
{
    size_t length = tSize * tCount, index = 0;
    while (index < length)
    {
        unsigned char* temp = (unsigned char*)pData + index;
        if ((temp[0] == '\r') || (temp[0] == '\n'))
            break;
        index++;
    }

    std::string str((unsigned char*)pData, (unsigned char*)pData + index);
    std::map<std::string, std::string>* pmHeader = (std::map<std::string, std::string>*)pmUser;
    size_t pos = str.find(": ");
    if (pos != std::string::npos)
        pmHeader->insert(std::pair<std::string, std::string>(str.substr(0, pos), str.substr(pos + 2)));

    return (tCount);
}

Lua::LuaTable_t Plugin::ToLuaTable(std::map<std::string, std::any> args) 
{
    Lua::LuaTable_t table(new Lua::LuaTable);

    for (auto& [key, val] : args)
    {
        if (val.type() == typeid(int))
            table->Add(key, std::any_cast<int>(val));
        else if (val.type() == typeid(std::string))
            table->Add(key, std::any_cast<std::string>(val));
        else if (val.type() == typeid(float))
            table->Add(key, std::any_cast<float>(val));
        else if (val.type() == typeid(long))
            table->Add(key, std::any_cast<long>(val));
        else if (val.type() == typeid(double))
            table->Add(key, std::any_cast<double>(val));
        else if (val.type() == typeid(char))
            table->Add(key, std::any_cast<char>(val));
        else if (val.type() == typeid(std::map<std::string, std::string>)) {
            std::map<std::string, std::any> aMap = Plugin::Get()->ToMapAny(std::any_cast<std::map<std::string, std::string>>(val));
            table->Add(key, Plugin::Get()->ToLuaTable(aMap));
        }
    }

    return table;
}

std::map<std::string, std::any> Plugin::ToMapAny(std::map<std::string, std::string> map)
{
    std::map<std::string, std::any> output;
    for (auto& [key, val] : map)
        output[key] = val;

    return output;
}

Plugin::Plugin()
{
    LUA_DEFINE(http_set_user_agent)
    {
        Lua::ParseArguments(L, Plugin::Get()->userAgent);
        return 1;
    });

	LUA_DEFINE(http_get)
	{
        Lua::LuaArgs_t args;
        Lua::ParseArguments(L, args);

        int args_size = static_cast<int>(args.size());
        if (args_size < 1) return 0;

        std::string URL = args[0].GetValue<std::string>();

        auto curl = curl_easy_init();
        if (curl) {
            std::map<std::string, std::any> output;

            curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, Plugin::Get()->userAgent.c_str());
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

            std::string response_string;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

            std::map<std::string, std::string> header;
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header);

            char* url;
            long response_code;
            double elapsed;

            curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
            curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);

            curl_easy_cleanup(curl);
            curl = NULL;

            output["url"] = url;
            output["status"] = response_code;
            output["elapsed"] = elapsed;

            output["body"] = response_string;
            output["header"] = header;

            return Lua::ReturnValues(L, Plugin::Get()->ToLuaTable(output));
        }

        return 1;
	});
}
