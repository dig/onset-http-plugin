#pragma once

#include <vector>
#include <tuple>
#include <functional>
#include <map>
#include <any>

#include <PluginSDK.h>
#include "Singleton.hpp"
#include <curl/curl.h>

class Plugin : public Singleton<Plugin>
{
	friend class Singleton<Plugin>;
private:
	Plugin();
	~Plugin() = default;

private:
	using FuncInfo_t = std::tuple<const char *, lua_CFunction>;
	std::vector<FuncInfo_t> _func_list;

private:
	inline void Define(const char * name, lua_CFunction func)
	{
		_func_list.emplace_back(name, func);
	}

public:
	std::string userAgent = "curl/7.42.0";
	decltype(_func_list) const &GetFunctions() const
	{
		return _func_list;
	}
	Lua::LuaTable_t ToLuaTable(std::map<std::string, std::any> args);
	std::map<std::string, std::any> ToMapAny(std::map<std::string, std::string> map);
	struct curl_slist* ParseHeaders(Lua::LuaTable_t table);
	std::string ParseParams(Lua::LuaTable_t table);
	int CompleteRequest(CURL* curl, std::string URL, struct curl_slist* chunk, std::map<std::string, std::any>* output);
};
