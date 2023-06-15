#pragma once
#include "DKUtil/Config.hpp"
#include "DKUtil/Utility.hpp"

namespace ModernStaggerLock
{
	using namespace DKUtil::Alias;

	class MSLSettings : public DKUtil::model::Singleton<MSLSettings>
	{
		friend DKUtil::model::Singleton<MSLSettings>;

	private:
		MSLSettings();

		template <class T>
		static void PrintSettingValue(const T& a_setting)
		{
			INFO("Setting:\"{}\" is {}"sv, a_setting.get_key(), a_setting.get_data());
		}

	public:
		Boolean DisableJumpWhenStagger{ "DisableJumpWhenStagger", "Control" };

		Double SmallStaggerMagnitude{ "SmallStaggerMagnitude", "StaggerLevel" };
		Double MediumStaggerMagnitude{ "MediumStaggerMagnitude", "StaggerLevel" };
		Double LargeStaggerMagnitude{ "LargeStaggerMagnitude", "StaggerLevel" };
		Double LargestStaggerMagnitude{ "LargestStaggerMagnitude", "StaggerLevel" };

		enum StaggerType : std::int32_t
		{
			kSmall = 0,
			kMedium,
			kLarge,
			kLargest,
			kTotal
		};

		std::vector<std::string> quickRecoveryEvents;
	};
}