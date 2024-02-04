#include "Hooks.h"
#include "Settings.h"
#include "SpecialStaggerHandler.h"
#include "Utils.h"

namespace ModernStaggerLock
{

	bool NotifyAnimationGraphHook::ProcessStaggerHanlder(RE::IAnimationGraphManagerHolder* a_graphMgr, const RE::BSFixedString& a_eventName)
	{
		using StaggerType = MSLSettings::StaggerType;

		auto actorRef = a_graphMgr ? skyrim_cast<RE::Actor*>(a_graphMgr) : nullptr;
		if (!actorRef)
			return false;

		if (_strcmpi("StaggerStart", a_eventName.c_str()) == 0) {
			// Set Stagger Direction
			float staggerDirection = 0.f;
			actorRef->GetGraphVariableFloat("staggerDirection", staggerDirection);
			actorRef->SetGraphVariableInt("msl_staggerDirection", staggerDirection < 0.25f || staggerDirection > 0.75 ? 0 : 1);

			//Set Stagger Level
			float staggerMagnitude = 0.f;
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
			auto staggerlevel = GetStaggerLevel(staggerMagnitude);
			actorRef->SetGraphVariableInt("msl_staggerLevel", staggerlevel);

			//Set Speical Stagger Type
			SpecialStaggerHandler::GetSingleton()->ProcessSpeicalStagger(actorRef, staggerlevel);

			return true;
		}

		return false;
	}

	bool NotifyAnimationGraphHook::ShouldQuickRecovery(RE::IAnimationGraphManagerHolder* a_graphMgr, const RE::BSFixedString& a_eventName)
	{
		auto actorRef = a_graphMgr ? skyrim_cast<RE::Actor*>(a_graphMgr) : nullptr;
		if (!actorRef || !actorRef->IsStaggering())
			return false;

		bool shouldQuickRecovery = false;
		if (actorRef->GetGraphVariableBool("MSL_IsStaggerRecovery", shouldQuickRecovery) && shouldQuickRecovery) {
			for (const auto& recovEvent : MSLSettings::quickRecoveryEvents) {
				if (_strcmpi(recovEvent.c_str(), a_eventName.c_str()) == 0)
					return true;
			}
		}

		return false;
	}

	bool DisableStaggerJumpHook::Hook_CanProcess(RE::JumpHandler* a_handler, RE::InputEvent* a_event)
	{
		auto result = func(a_handler, a_event);
		if (result) {
			auto playerRef = RE::PlayerCharacter::GetSingleton();
			auto settings = MSLSettings::GetSingleton();
			if (playerRef && playerRef->IsStaggering() && *settings->DisableJumpWhenStagger) {
				return false;
			}
		}

		return result;
	}

	constexpr uint32_t HashToUInt(const char* data, size_t const size) noexcept
	{
		uint32_t HashToUInt = 5381;

		for (const char* c = data; c < data + size; ++c)
			HashToUInt = ((HashToUInt << 5) + HashToUInt) + charToLower(*c);

		return HashToUInt;
	}

	inline constexpr uint32_t operator"" _h(const char* str, size_t size) noexcept
	{
		return HashToUInt(str, size);
	}

	EventResult AnimEventHook::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		if (!a_event || !a_event->holder)
			return _ProcessEvent(a_sink, a_event, a_eventSource);

		auto actorRef = a_event->holder->As<RE::Actor>();
		if (!actorRef) {
			return _ProcessEvent(a_sink, a_event, a_eventSource);
		}

		std::string_view eventTag = a_event->tag.data();

		static auto SetStaggerVariable = [](RE::Actor* a_actor, const std::string_view eventTag) {
			std::int32_t SmallStaggerCounter = 0,
						 MediumStaggerCounter = 0,
						 LargeStaggerCounter = 0,
						 LargestStaggerCounter = 0,
						 SpecialStaggerCounter = 0;

			switch (HashToUInt(eventTag.data(), eventTag.size())) {
			case "staggerStop"_h:
				a_actor->SetGraphVariableInt("msl_SpecialStaggerID", 0);
				break;

			case "MSL_SmallStaggerStart"_h:
				a_actor->SetGraphVariableInt("msl_SpecialStaggerID", 0);
				a_actor->GetGraphVariableInt("msl_SmallStaggerCounter", SmallStaggerCounter);
				SmallStaggerCounter += 1;
				break;

			case "MSL_MediumStaggerStart"_h:
				a_actor->SetGraphVariableInt("msl_SpecialStaggerID", 0);
				a_actor->GetGraphVariableInt("msl_MediumStaggerCounter", MediumStaggerCounter);
				MediumStaggerCounter += 1;
				break;

			case "MSL_LargeStaggerStart"_h:
				a_actor->SetGraphVariableInt("msl_SpecialStaggerID", 0);
				a_actor->GetGraphVariableInt("msl_LargeStaggerCounter", LargeStaggerCounter);
				LargeStaggerCounter += 1;
				break;

			case "MSL_LargestStaggerStart"_h:
				a_actor->SetGraphVariableInt("msl_SpecialStaggerID", 0);
				a_actor->GetGraphVariableInt("msl_LargestStaggerCounter", LargestStaggerCounter);
				LargestStaggerCounter += 1;
				break;

			case "MSL_SpecialStaggerStart"_h:
				a_actor->GetGraphVariableInt("msl_SpecialStaggerCounter", SpecialStaggerCounter);
				SpecialStaggerCounter += 1;
				break;

			default:
				break;
			}

			a_actor->SetGraphVariableInt("msl_SmallStaggerCounter", SmallStaggerCounter);
			a_actor->SetGraphVariableInt("msl_MediumStaggerCounter", MediumStaggerCounter);
			a_actor->SetGraphVariableInt("msl_LargeStaggerCounter", LargeStaggerCounter);
			a_actor->SetGraphVariableInt("msl_LargestStaggerCounter", LargestStaggerCounter);
			a_actor->SetGraphVariableInt("msl_SpecialStaggerCounter", SpecialStaggerCounter);
		};

		static std::vector<std::string_view> events = {
			{ "staggerStop" },
			{ "MSL_SmallStaggerStart" },
			{ "MSL_MediumStaggerStart" },
			{ "MSL_LargeStaggerStart" },
			{ "MSL_LargestStaggerStart" },
			{ "MSL_SpecialStaggerStart" }
		};

		for (const auto& eventName : events) {
			if (_strcmpi(eventName.data(), a_event->tag.c_str()) == 0) {
				SetStaggerVariable(actorRef, eventName);
				break;
			}
		}

		return _ProcessEvent(a_sink, a_event, a_eventSource);
	}

}