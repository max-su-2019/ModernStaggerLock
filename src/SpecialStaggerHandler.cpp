#include "SpecialStaggerHandler.h"
#include "Settings.h"
#include "Utils.h"

namespace ModernStaggerLock
{
	constexpr int32_t HashSpecialStaggerID(const char* data, size_t const size) noexcept
	{
		int32_t hash = 5381;

		for (const char* c = data; c < data + size; ++c)
			hash = ((hash << 5) + hash) + charToLower(*c);

		return hash;
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

	bool SpecialStaggerHandler::FindSpecialStaggerData(RE::Actor* a_actor, SpecialStaggerData& a_data)
	{
		if (!a_actor) {
			return false;
		}

		if (!MSLSettings::ersh_Precision) {
			return false;
		}

		auto extraData = MSLSettings::ersh_Precision->GetCachedExtraData(static_cast<RE::TESObjectREFR*>(a_actor)->GetHandle(), Plugin::NAME.data());
		if (extraData) {
			return ParseSpecialStaggerData(extraData, a_data);
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
			if (a_actor->GetGraphVariableInt("msl_specialStaggerId", curSpecialStaggerId) && CanTriggerTransaction(curSpecialStaggerId, nextSpecialStaggerId)) {
				a_actor->SetGraphVariableInt("msl_staggerLevel", 5);
				a_actor->SetGraphVariableInt("msl_specialStaggerId", nextSpecialStaggerId);
			}
		}
	}

	bool SpecialStaggerHandler::CanTriggerTransaction(const std::int32_t a_curAnimID, const std::int32_t a_NextAnimID) const
	{
		if (a_curAnimID == 0 || a_curAnimID == a_NextAnimID) {
			DEBUG("Return True");
			return true;
		}

		auto it = transDataMap.find(a_curAnimID);
		if (it != transDataMap.end()) {
			auto transData = it->second;
			if (transData.Unrestricted) {
				DEBUG("Return Unrestricted True");
				return true;
			} else {
				DEBUG("Return List");
				return transData.transAnimList.find(a_NextAnimID) != transData.transAnimList.end();
			}
		}

		return false;
	}

	constexpr std::uint32_t SpecialStaggerHandler::SpecialStaggerData::GetNameHash() noexcept
	{
		return animName.length() > 0 ? HashSpecialStaggerID(animName.data(), animName.size()) : 0;
	}

	void from_json(const json& j, SpecialStaggerData& a_data)
	{
		j.at("SpecialStaggerName").get_to(a_data.animName);

		if (j.find("IgnoreStaggerLevel") != j.end()) {
			j.at("IgnoreStaggerLevel").get_to(a_data.ignoreStaggerLevel);
		}
	}

	void from_json(const json& j, SpecialStaggerTransactionData& a_data)
	{
		j.at("AnimName").get_to(a_data.animName);
		j.at("Unrestricted").get_to(a_data.Unrestricted);
		if (!a_data.Unrestricted) {
			std::vector<std::string> animNames;
			j.at("TransAnimList").get_to(animNames);
			for (auto const& animName : animNames) {
				a_data.transAnimList.emplace(HashSpecialStaggerID(animName.data(), animName.size()));
			}
		}
	}

}