#include "LoadGame.h"
#include "Hooks.h"
#include "Patches.h"
#include "Settings.h"

namespace ModernStaggerLock
{
	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (msg->type == SKSE::MessagingInterface::kPostLoad) {
			ModernStaggerLock::StaggeredStateCheckPatch::Install();

			ModernStaggerLock::NotifyAnimationGraphHook::CharacterEx::InstallHook();
			ModernStaggerLock::NotifyAnimationGraphHook::PlayerEx::InstallHook();

			ModernStaggerLock::DisableStaggerJumpHook::InstallHook();

			MSLSettings::GetSingleton();
		}
	}

}
