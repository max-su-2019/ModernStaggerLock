#include "Conditions.h"
#include "Utils.h"

namespace ModernStaggerLock
{
	constexpr int32_t HashToInt(const char* data, size_t const size) noexcept
	{
		int32_t hash = 5381;

		for (const char* c = data; c < data + size; ++c)
			hash = ((hash << 5) + hash) + charToLower(*c);

		return hash;
	}

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
			if (HashToInt(textArgument.data(), textArgument.size()) == curSpecialStaggerId) {
				return true;
			}
		}

		return false;
	}

	RE::BSString GetSpeicalStaggerNameCondition::GetArgument() const
	{
		const auto textArgument = textComponent->GetTextValue();
		return std::format("{}(hash:{})", textArgument.data(), HashToInt(textArgument.data(), textArgument.size())).data();
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
