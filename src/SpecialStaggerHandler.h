#pragma once
#include "DKUtil/Config.hpp"
#include "DKUtil/Utility.hpp"
#include "PrecisionAPI.h"
#include "nlohmann/json.hpp"

namespace ModernStaggerLock
{
	using namespace DKUtil::Alias;
	using json = nlohmann::json;

	constexpr int32_t HashSpecialStaggerID(const char* data, size_t const size) noexcept;

	class SpecialStaggerHandler : public DKUtil::model::Singleton<SpecialStaggerHandler>
	{
		friend DKUtil::model::Singleton<SpecialStaggerHandler>;

	public:
		struct SpecialStaggerData
		{
			constexpr std::uint32_t GetNameHash() noexcept;

			bool IsIgnoreStaggerLevel() const noexcept { return ignoreStaggerLevel; }

			friend void from_json(const json& j, SpecialStaggerData& a_data);

		private:
			std::string animName = "";

			bool ignoreStaggerLevel = false;
		};

		struct SpecialStaggerTransactionData
		{
			std::string animName = "";
			bool Unrestricted = false;
			std::set<std::int32_t> transAnimList = {};
		};

		using SpecialStaggerTargetMap = std::unordered_map<RE::Actor*, SpecialStaggerData>;
		using SpecialStaggerTransMap = std::unordered_map<std::int32_t, SpecialStaggerTransactionData>;

		SpecialStaggerHandler() = default;

		bool FindSpecialStaggerData(RE::Actor* a_actor, SpecialStaggerData& a_data);
		void ProcessSpeicalStagger(RE::Actor* a_actor, std::int32_t a_StaggerLevel);

		SpecialStaggerTransMap transDataMap;

	private:
		static bool ParseSpecialStaggerData(const std::string_view a_string, SpecialStaggerData& a_data);
		bool CanTriggerTransaction(const std::int32_t a_curAnimID, const std::int32_t a_NextAnimID) const;
	};
	using SpecialStaggerData = SpecialStaggerHandler::SpecialStaggerData;
	using SpecialStaggerTransactionData = SpecialStaggerHandler::SpecialStaggerTransactionData;

	void from_json(const json& j, SpecialStaggerData& a_data);
	void from_json(const json& j, SpecialStaggerTransactionData& a_data);
}