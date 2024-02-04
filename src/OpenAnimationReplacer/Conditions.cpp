#include "Conditions.h"
#include "SpecialStaggerHandler.h"

namespace ModernStaggerLock
{

	GetSpeicalStaggerNameCondition::GetSpeicalStaggerNameCondition()
	{
		textComponent = static_cast<ITextConditionComponent*>(AddBaseComponent(
			ConditionComponentType::kText, "Special Stagger Animation Name"));
	}

	bool GetSpeicalStaggerNameCondition::EvaluateImpl([[maybe_unused]] RE::TESObjectREFR* a_refr,
		[[maybe_unused]] RE::hkbClipGenerator* a_clipGenerator) const
	{
		std::int32_t curSpecialStaggerId = 0;
		if (a_refr && a_refr->GetGraphVariableInt("msl_specialStaggerId", curSpecialStaggerId) && curSpecialStaggerId) {
			const auto textArgument = textComponent->GetTextValue();
			if (HashSpecialStaggerID(textArgument.data(), textArgument.size()) == curSpecialStaggerId) {
				return true;
			}
		}

		return false;
	}

	RE::BSString GetSpeicalStaggerNameCondition::GetArgument() const
	{
		const auto textArgument = textComponent->GetTextValue();
		return std::format("{}(hash:{})", textArgument.data(), HashSpecialStaggerID(textArgument.data(), textArgument.size())).data();
	}

	RE::BSString GetSpeicalStaggerNameCondition::GetCurrent(RE::TESObjectREFR* a_refr) const
	{
		std::int32_t curSpecialStaggerId = 0;
		if (a_refr && a_refr->GetGraphVariableInt("msl_specialStaggerId", curSpecialStaggerId)) {
			return std::format("{}", curSpecialStaggerId).data();
		}

		return "0";
	}
}
