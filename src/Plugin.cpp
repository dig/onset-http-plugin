#include <sstream>
#include <iostream>
#include <cstring>
#include <string>

#include "Plugin.hpp"
#include "HTTPRequest.hpp"

#ifdef LUA_DEFINE
# undef LUA_DEFINE
#endif
#define LUA_DEFINE(name) Define(#name, [](lua_State *L) -> int

Plugin::Plugin()
{

	LUA_DEFINE(Get)
	{
		http::Request request("http://test.com/test");
		const http::Response getResponse = request.send("GET");
		std::cout << std::string(getResponse.body.begin(), getResponse.body.end()) << '\n';

		return 1;
	});
}
