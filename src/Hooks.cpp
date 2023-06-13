#include "Hooks.h"
#include "Settings.h"

namespace ModernStaggerLock
{

	bool NotifyAnimationGraphHook::ProcessStaggerHanlder(RE::IAnimationGraphManagerHolder* a_graphMgr, const RE::BSFixedString& a_eventName)
	{
		using StaggerType = MSLSettings::StaggerType;

		auto actorRef = a_graphMgr ? skyrim_cast<RE::Actor*>(a_graphMgr) : nullptr;
		if (!actorRef)
			return false;

		if (_strcmpi("StaggerStart", a_eventName.c_str()) == 0) {
			float staggerDirection = 0.f, staggerMagnitude = 0.f;

			actorRef->GetGraphVariableFloat("staggerDirection", staggerDirection);
			actorRef->SetGraphVariableInt("msl_staggerDirection", staggerDirection < 0.25f || staggerDirection > 0.75 ? 0 : 1);

			actorRef->GetGraphVariableFloat("staggerMagnitude", staggerMagnitude);

			auto GetStaggerLevel = [](const float staggerMagnitude) -> std::int32_t {
				auto mslSetting = MSLSettings::GetSingleton();
				const float StaggerLevelRange[StaggerType::kTotal] = {
					*mslSetting->SmallStaggerMagnitude,
					*mslSetting->MediumStaggerMagnitude,
					*mslSetting->LargeStaggerMagnitude,
					*mslSetting->LargestStaggerMagnitude
				};

				for (std::int32_t i = StaggerType::kLargest; i >= StaggerType::kSmall; i--) {
					if (staggerMagnitude >= StaggerLevelRange[i]) {
						return i + 1;
					}
				}

				return 1;
			};

			actorRef->SetGraphVariableInt("msl_staggerLevel", GetStaggerLevel(staggerMagnitude));

			return true;
		}

		return false;
	}

	bool NotifyAnimationGraphHook::ShouldQuickRecovery(RE::IAnimationGraphManagerHolder* a_graphMgr, const RE::BSFixedString& a_eventName)
	{
		auto actorRef = a_graphMgr ? skyrim_cast<RE::Actor*>(a_graphMgr) : nullptr;
		if (!actorRef || !actorRef->IsStaggering())
			return false;

		const std::vector<std::string> QuickRecovEvents{
			"TKDodgeForward",
			"TKDodgeLeft",
			"TKDodgeBack",
			"TKDodgeRight",
			"Dodge"
		};

		bool shouldQuickRecovery = false;
		if (actorRef->GetGraphVariableBool("MSL_IsStaggerRecovery", shouldQuickRecovery) && shouldQuickRecovery) {
			for (const auto& recovEvent : QuickRecovEvents) {
				if (_strcmpi(recovEvent.c_str(), a_eventName.c_str()) == 0)
					return true;
			}
		}

		return false;
	}
}