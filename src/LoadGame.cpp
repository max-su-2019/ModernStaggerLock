#include "LoadGame.h"
#include "Hooks.h"
#include "OpenAnimationReplacer/Conditions.h"
#include "Patches.h"
#include "Settings.h"
#include "SpecialStaggerHandler.h"

namespace ModernStaggerLock
{
	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (msg->type == SKSE::MessagingInterface::kPostLoad) {
			ActorUpdateHook::CharacterEx::InstallHook();
			ActorUpdateHook::PlayerEx::InstallHook();

			PerformLandActionHook_NPC::InstallHook();
			PerformLandActionHook_PC::InstallHook();

			AnimEventHook::InstallHook();

			StaggeredStateCheckPatch::Install();

			NotifyAnimationGraphHook::CharacterEx::InstallHook();
			NotifyAnimationGraphHook::PlayerEx::InstallHook();

			DisableStaggerJumpHook::InstallHook();

			MSLSettings::ersh_Precision = reinterpret_cast<PRECISION_API::IVPrecision5*>(PRECISION_API::RequestPluginAPI(PRECISION_API::InterfaceVersion::V5));
			if (MSLSettings::ersh_Precision) {
				//	auto res = ersh_Precision->AddPreHitCallback(SKSE::GetPluginHandle(), SpecialStaggerHandler::precisionPreHitCallbackFunc);
				//	if (res == PRECISION_API::APIResult::OK || res == PRECISION_API::APIResult::AlreadyRegistered)
				//		INFO("Collision prehit callback successfully registered.");

				auto res = MSLSettings::ersh_Precision->AddExtraParameterName(Plugin::NAME.data());
				if (res == PRECISION_API::APIResult::OK || res == PRECISION_API::APIResult::AlreadyRegistered)
					INFO("AddExtraParameterName successfully registered.");
			}

			OAR_API::Conditions::GetAPI(OAR_API::Conditions::InterfaceVersion::V2);
			if (g_oarConditionsInterface) {
				RegisterCondition<GetSpeicalStaggerNameCondition>();
			} else {
				ERROR("Failed to request Open Animation Replacer API"sv);
			}
		} else if (msg->type == SKSE::MessagingInterface::kDataLoaded) {
			MSLSettings::GetSingleton();
		}
	}

}
