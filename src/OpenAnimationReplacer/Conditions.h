#pragma once
#include "API/OpenAnimationReplacerAPI-Conditions.h"

namespace ModernStaggerLock
{
	using namespace Conditions;

	template <typename T>
	void RegisterCondition()
	{
		auto result = OAR_API::Conditions::AddCustomCondition<T>();
		switch (result) {
		case OAR_API::Conditions::APIResult::OK:
			INFO("Registered {} condition!", T::CONDITION_NAME);
			break;
		case OAR_API::Conditions::APIResult::AlreadyRegistered:
			WARN("Condition {} is already registered!",
				T::CONDITION_NAME);
			break;
		case OAR_API::Conditions::APIResult::Invalid:
			ERROR("Condition {} is invalid!", T::CONDITION_NAME);
			break;
		case OAR_API::Conditions::APIResult::Failed:
			ERROR("Failed to register condition {}!", T::CONDITION_NAME);
			break;
		}
	}

	// The bare minimum
	class GetSpeicalStaggerNameCondition : public CustomCondition
	{
	public:
		constexpr static inline std::string_view CONDITION_NAME = "GetSpeicalStaggerNameCondition"sv;

		GetSpeicalStaggerNameCondition();

		RE::BSString GetName() const override { return CONDITION_NAME.data(); }
		RE::BSString GetDescription() const override { return "Get the name of the current activated special stagger animation of modern staggerlock."sv.data(); }

		// the earliest version of your plugin that provided this condition in its current form
		constexpr REL::Version GetRequiredVersion() const override { return { 1, 0, 0 }; }

	protected:
		// this is the function with the actual condition logic
		bool EvaluateImpl(RE::TESObjectREFR* a_refr, RE::hkbClipGenerator* a_clipGenerator) const override;

		// this is displayed in the UI in the right column, next to the condition name
		RE::BSString GetArgument() const override;

		// this is displayed in the UI below the condition components while there's a target selected - it's supposed to be the current value of whatever the condition is checking
		RE::BSString GetCurrent(RE::TESObjectREFR* a_refr) const override;

		ITextConditionComponent* textComponent;
	};

}
