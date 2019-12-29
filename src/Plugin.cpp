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
        else if (val.type() == typeid(char*))
            table->Add(key, *std::any_cast<char*>(val));
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

struct curl_slist* Plugin::ParseHeaders(Lua::LuaTable_t table)
{
    struct curl_slist* chunk = NULL;
    table->ForEach([&chunk](Lua::LuaValue k, Lua::LuaValue v) {
        if (k.IsString() && v.IsString()) {
            std::string headerStr = k.GetValue<std::string>() + ": " + v.GetValue<std::string>();
            chunk = curl_slist_append(chunk, headerStr.c_str());
        }
        else if (k.IsString() && v.IsInteger()) {
            std::string headerStr = k.GetValue<std::string>() + ": " + std::to_string(v.GetValue<int>());
            chunk = curl_slist_append(chunk, headerStr.c_str());
        }
    });

    return chunk;
}

std::string Plugin::ParseParams(Lua::LuaTable_t table)
{
    std::string params = "?";

    table->ForEach([&params](Lua::LuaValue k, Lua::LuaValue v) {
        std::string toAdd;
        if (params.length() > 1) toAdd = "&";

        if (k.IsString()) {
            toAdd = toAdd + k.GetValue<std::string>();
        }
        else if (k.IsInteger()) {
            toAdd = toAdd + std::to_string(k.GetValue<int>());
        }

        toAdd = toAdd + "=";

        if (v.IsString()) {
            toAdd = toAdd + v.GetValue<std::string>();
        }
        else if (v.IsInteger()) {
            toAdd = toAdd + std::to_string(v.GetValue<int>());
        }

        params = params + toAdd;
    });

    return params;
}

int Plugin::CompleteRequest(CURL* curl, std::string URL, struct curl_slist* chunk, std::map<std::string, std::any>* output)
{
    CURLcode res;

    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, Plugin::Get()->userAgent.c_str());
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    std::string response_string;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    std::map<std::string, std::string> headers;
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);

    char* url;
    long response_code;
    double elapsed;

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        return 0;

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);

    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);
    curl = NULL;

    (*output)["status"] = response_code;
    (*output)["elapsed"] = elapsed;

    (*output)["body"] = response_string;
    (*output)["headers"] = headers;

    return 1;
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

        CURL* curl = curl_easy_init();
        if (curl) {
            std::map<std::string, std::any> output;

            struct curl_slist* chunk = NULL;
            if (args_size >= 2) {
                Lua::LuaTable_t table = args[1].GetValue<Lua::LuaTable_t>();
                chunk = Plugin::Get()->ParseHeaders(table);
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
            }

            if (args_size >= 3) {
                Lua::LuaTable_t table = args[2].GetValue<Lua::LuaTable_t>();
                URL = URL + Plugin::Get()->ParseParams(table);
            }

            Plugin::Get()->CompleteRequest(curl, URL, chunk, &output);
            return Lua::ReturnValues(L, Plugin::Get()->ToLuaTable(output));
        }

        return 1;
	});

    LUA_DEFINE(http_post)
    {
        Lua::LuaArgs_t args;
        Lua::ParseArguments(L, args);

        int args_size = static_cast<int>(args.size());
        if (args_size < 1) return 0;

        std::string URL = args[0].GetValue<std::string>();

        CURL* curl = curl_easy_init();
        if (curl) {
            std::map<std::string, std::any> output;

            struct curl_slist* chunk = NULL;
            if (args_size >= 2) {
                Lua::LuaTable_t table = args[1].GetValue<Lua::LuaTable_t>();
                chunk = Plugin::Get()->ParseHeaders(table);
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
            }

            if (args_size >= 3) {
                if (args[2].IsTable()) {
                    Lua::LuaTable_t table = args[2].GetValue<Lua::LuaTable_t>();
                    std::string params = Plugin::Get()->ParseParams(table);

                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.substr(1, params.length()).c_str());
                }
                else if (args[2].IsString()) {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, args[2].GetValue<std::string>().c_str());
                }
            }
            else {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
            }

            Plugin::Get()->CompleteRequest(curl, URL, chunk, &output);
            return Lua::ReturnValues(L, Plugin::Get()->ToLuaTable(output));
        }

        return 1;
    });

    LUA_DEFINE(http_put)
    {
        Lua::LuaArgs_t args;
        Lua::ParseArguments(L, args);

        int args_size = static_cast<int>(args.size());
        if (args_size < 1) return 0;

        std::string URL = args[0].GetValue<std::string>();

        CURL* curl = curl_easy_init();
        if (curl) {
            std::map<std::string, std::any> output;

            struct curl_slist* chunk = NULL;
            if (args_size >= 2) {
                Lua::LuaTable_t table = args[1].GetValue<Lua::LuaTable_t>();
                chunk = Plugin::Get()->ParseHeaders(table);
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
            }

            if (args_size >= 3) {
                if (args[2].IsTable()) {
                    Lua::LuaTable_t table = args[2].GetValue<Lua::LuaTable_t>();
                    std::string params = Plugin::Get()->ParseParams(table);

                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.substr(1, params.length()).c_str());
                }
                else if (args[2].IsString()) {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, args[2].GetValue<std::string>().c_str());
                }
            }
            else {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
            }

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            Plugin::Get()->CompleteRequest(curl, URL, chunk, &output);
            return Lua::ReturnValues(L, Plugin::Get()->ToLuaTable(output));
        }

        return 1;
    });
}
