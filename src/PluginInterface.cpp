#include <PluginSDK.h>
#include "Plugin.hpp"
#include "version.hpp"


Onset::IServerPlugin* Onset::Plugin::_instance = nullptr;


EXPORT(int) OnPluginGetApiVersion()
{
	return PLUGIN_API_VERSION;
}

EXPORT(void) OnPluginCreateInterface(Onset::IBaseInterface *PluginInterface)
{
	Onset::Plugin::Init(PluginInterface);
}

EXPORT(int) OnPluginStart()
{
	Plugin::Get();
	Onset::Plugin::Get()->Log("OnsetJavaPlugin (" PLUGIN_VERSION ") loaded!");
	return PLUGIN_API_VERSION;
}

EXPORT(void) OnPluginStop()
{
	Plugin::Singleton::Destroy();
	Onset::Plugin::Get()->Log("OnsetJavaPlugin unloaded!");
	Onset::Plugin::Destroy();
}

EXPORT(void) OnPluginTick(float DeltaSeconds)
{
	(void)DeltaSeconds;
}

EXPORT(void) OnPackageLoad(const char *PackageName, lua_State *L)
{
	(void)PackageName;
	for (auto const &f : Plugin::Get()->GetFunctions())
		Lua::RegisterPluginFunction(L, std::get<0>(f), std::get<1>(f));
}

EXPORT(void) OnPackageUnload(const char *PackageName)
{
	(void)PackageName;
}
