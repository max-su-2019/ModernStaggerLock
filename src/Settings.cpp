#include "Settings.h"

namespace ModernStaggerLock
{
	using json = nlohmann::json;

	static auto MainConfig = COMPILE_PROXY("ModernStaggerLock.ini"sv);

	MSLSettings::MSLSettings()
	{
		MainConfig.Bind<0.0, 1.0>(SmallStaggerMagnitude, 0.0);
		MainConfig.Bind<0.0, 1.0>(MediumStaggerMagnitude, 0.5);
		MainConfig.Bind<0.0, 1.0>(LargeStaggerMagnitude, 0.75);
		MainConfig.Bind<0.0, 1.0>(LargestStaggerMagnitude, 1.0);
		MainConfig.Bind(DisableJumpWhenStagger, true);

		MainConfig.Load();

		PrintSettingValue(SmallStaggerMagnitude);
		PrintSettingValue(MediumStaggerMagnitude);
		PrintSettingValue(LargeStaggerMagnitude);
		PrintSettingValue(LargestStaggerMagnitude);

		auto MSLFilesName = DKUtil::Config::GetAllFiles<false>("Data\\SKSE\\Plugins\\ModernStaggerLock"sv, ".json"sv, {}, "_MSL"sv);
		for (const auto& fileName : MSLFilesName) {
			try {
				json j = json::parse(std::ifstream(fileName));
				if (j.contains("QuickRecoveryEvents")) {
					auto eventLists = j.at("QuickRecoveryEvents").get<std::vector<std::string>>();
					quickRecoveryEvents.insert(quickRecoveryEvents.end(), eventLists.begin(), eventLists.end());
				}
			} catch (const json::exception& ex) {
				ERROR("Caught expection when parsing file \"{}\":\n {}", fileName, ex.what());
			}
		}

		for (auto eventName : quickRecoveryEvents) {
			INFO("Get a Quick Recovery Event Name: \"{}\"", eventName);
		}
	}

}