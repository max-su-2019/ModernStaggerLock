#include "LoadGame.h"
#include "Hooks.h"
#include "OpenAnimationReplacer/Conditions.h"
#include "Patches.h"
#include "Settings.h"
#include "SpecialStaggerHandler.h"

namespace ModernStaggerLock
{
	PRECISION_API::IVPrecision5* ersh_Precision = nullptr;

	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (msg->type == SKSE::MessagingInterface::kPostLoad) {
			ModernStaggerLock::ActorUpdateHook::PlayerEx::InstallHook();
			ModernStaggerLock::ActorUpdateHook::CharacterEx::InstallHook();

			ModernStaggerLock::AnimEventHook::InstallHook();

			ModernStaggerLock::StaggeredStateCheckPatch::Install();

			ModernStaggerLock::NotifyAnimationGraphHook::CharacterEx::InstallHook();
			ModernStaggerLock::NotifyAnimationGraphHook::PlayerEx::InstallHook();

			ModernStaggerLock::DisableStaggerJumpHook::InstallHook();

			MSLSettings::GetSingleton();

			ersh_Precision = reinterpret_cast<PRECISION_API::IVPrecision5*>(PRECISION_API::RequestPluginAPI(PRECISION_API::InterfaceVersion::V5));
			if (ersh_Precision) {
				auto res = ersh_Precision->AddPreHitCallback(SKSE::GetPluginHandle(), SpecialStaggerHandler::precisionPreHitCallbackFunc);
				if (res == PRECISION_API::APIResult::OK || res == PRECISION_API::APIResult::AlreadyRegistered)
					INFO("Collision prehit callback successfully registered.");
				res = ersh_Precision->AddExtraParameterName(Plugin::NAME);
				if (res == PRECISION_API::APIResult::OK || res == PRECISION_API::APIResult::AlreadyRegistered)
					INFO("AddExtraParameterName successfully registered.");
			}

			OAR_API::Conditions::GetAPI(OAR_API::Conditions::InterfaceVersion::V2);
			if (g_oarConditionsInterface) {
				RegisterCondition<ModernStaggerLock::GetSpeicalStaggerNameCondition>();
			} else {
				ERROR("Failed to request Open Animation Replacer API"sv);
			}
		}
	}

}
