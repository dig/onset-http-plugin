#pragma once

#include <vector>
#include <tuple>
#include <functional>
#include <jni.h>
#include <PluginSDK.h>
#include "Singleton.hpp"

class Plugin : public Singleton<Plugin>
{
	friend class Singleton<Plugin>;
private:
	Plugin();
	~Plugin() = default;
	JavaVM *jvms[30];
	JNIEnv* jenvs[30];

private:
	using FuncInfo_t = std::tuple<const char *, lua_CFunction>;
	std::vector<FuncInfo_t> _func_list;

private:
	inline void Define(const char * name, lua_CFunction func)
	{
		_func_list.emplace_back(name, func);
	}

public:
	decltype(_func_list) const &GetFunctions() const
	{
		return _func_list;
	}
	int CreateJava(std::string classPath);
	void DestroyJava(int id);
	JavaVM* GetJavaVM(int id);
	JNIEnv* GetJavaEnv(int id);

	jobject ToJavaObject(JNIEnv* jenv, Lua::LuaValue value);
	Lua::LuaValue ToLuaValue(JNIEnv* jenv, jobject object);
};
