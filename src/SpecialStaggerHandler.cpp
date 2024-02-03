#include "SpecialStaggerHandler.h"
#include "Utils.h"

namespace ModernStaggerLock
{
	PRECISION_API::PreHitCallbackReturn SpecialStaggerHandler::precisionPreHitCallbackFunc(const PRECISION_API::PrecisionHitData& a_precisionHitData)
	{
		auto specialStaggerHandler = SpecialStaggerHandler::GetSingleton();
		auto actor = a_precisionHitData.target ? a_precisionHitData.target->As<RE::Actor>() : nullptr;
		if (actor) {
			auto it = a_precisionHitData.extraDataMap.find(Plugin::NAME.data());
			if (it != a_precisionHitData.extraDataMap.end()) {
				SpecialStaggerData data;
				if (ParseSpecialStaggerData(it->second, data)) {
					std::scoped_lock lock(specialStaggerHandler->m_mutex);
					specialStaggerHandler->targets.emplace(actor, data);
				}
			}
		}

		PRECISION_API::PreHitCallbackReturn ret;
		ret.modifiers = {};
		return ret;
	}

	void SpecialStaggerHandler::EraseStaggerDataFromMap(RE::Actor* a_actor)
	{
		std::scoped_lock lock(m_mutex);
		if (targets.contains(a_actor)) {
			targets.erase(a_actor);
		}
	}

	bool SpecialStaggerHandler::FindSpecialStaggerData(RE::Actor* a_actor, SpecialStaggerData& a_data)
	{
		std::scoped_lock lock(m_mutex);
		auto it = targets.find(a_actor);
		if (it != targets.end()) {
			a_data = it->second;
			return true;
		}

		return false;
	}

	void SpecialStaggerHandler::ProcessSpeicalStagger(RE::Actor* a_actor, std::int32_t a_StaggerLevel)
	{
		if (a_actor) {
			SpecialStaggerData data;

			if (!FindSpecialStaggerData(a_actor, data) || (!data.IsIgnoreStaggerLevel() && a_StaggerLevel < 4)) {
				return;
			}

			std::int32_t nextSpecialStaggerId = data.GetNameHash();
			if (nextSpecialStaggerId == 0) {
				return;
			}

			std::int32_t curSpecialStaggerId = 0;
			if (a_actor->GetGraphVariableInt("msl_specialStaggerId", curSpecialStaggerId) && (curSpecialStaggerId == 0 || curSpecialStaggerId == nextSpecialStaggerId)) {
				a_actor->SetGraphVariableInt("msl_staggerLevel", 5);
				a_actor->SetGraphVariableInt("msl_specialStaggerId", nextSpecialStaggerId);
			}
		}
	}

	bool SpecialStaggerHandler::ParseSpecialStaggerData(const std::string_view a_string, SpecialStaggerData& a_data)
	{
		try {
			auto j = json::parse(a_string);
			a_data = j.get<SpecialStaggerData>();
			return true;
		} catch (const json::exception&) {
			return false;
		}

		return false;
	}

	constexpr std::uint32_t SpecialStaggerHandler::SpecialStaggerData::GetNameHash() noexcept
	{
		return animName.length() > 0 ? HashToInt(animName.data(), animName.size()) : 0;
	}

	void from_json(const json& j, SpecialStaggerData& a_data)
	{
		j.at("SpecialStaggerName").get_to(a_data.animName);

		if (j.find("IgnoreStaggerLevel") != j.end()) {
			j.at("IgnoreStaggerLevel").get_to(a_data.ignoreStaggerLevel);
		}
	}

}