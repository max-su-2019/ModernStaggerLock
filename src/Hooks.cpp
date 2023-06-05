#include "Hooks.h"

namespace ModernStaggerLock
{
	std::int32_t PeformStagggerHook::PerformActionImp(RE::TESActionData* a_action, float a2)
	{
		if (a_action) {
			auto actorRef = a_action->source ? a_action->source->As<RE::Actor>() : nullptr;
			if (actorRef) {
				float staggerDirection = 0.f, staggerMagnitude = 0.f;

				actorRef->GetGraphVariableFloat("staggerDirection", staggerDirection);
				actorRef->SetGraphVariableInt("msl_staggerDirection", staggerDirection < 0.25f || staggerDirection > 0.75 ? 0 : 1);

				actorRef->GetGraphVariableFloat("staggerMagnitude", staggerMagnitude);
				actorRef->SetGraphVariableInt("msl_staggerLevel", std::clamp(std::int32_t(floor(staggerMagnitude * 4.f)), 1, 4));
			}
		}

		return _PerformActionImp(a_action, a2);
	}
}