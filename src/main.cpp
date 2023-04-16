DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept
{
	SKSE::PluginVersionData data{};

	data.PluginVersion(Plugin::Version);
	data.PluginName(Plugin::NAME);
	data.AuthorName(Plugin::AUTHOR);
	data.UsesAddressLibrary(true);

	return data;
}();


DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
    pluginInfo->name = SKSEPlugin_Version.pluginName;
    pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
    pluginInfo->version = SKSEPlugin_Version.pluginVersion;

    return true;
}


DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) { Sleep(100); }
#endif

	DKUtil::Logger::Init(Plugin::NAME, REL::Module::get().version().string());

	REL::Module::reset();
	SKSE::Init(a_skse);
	
	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

	// do stuff


	return true;
}
