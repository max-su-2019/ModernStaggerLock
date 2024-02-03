#pragma once
#include "DKUtil/Config.hpp"
#include "DKUtil/Utility.hpp"
#include "PrecisionAPI.h"
#include "nlohmann/json.hpp"

namespace ModernStaggerLock
{
	using namespace DKUtil::Alias;
	using json = nlohmann::json;

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

		using SpecailStaggerTargetMap = std::unordered_map<RE::Actor*, SpecialStaggerData>;

		SpecialStaggerHandler() = default;

		static PRECISION_API::PreHitCallbackReturn precisionPreHitCallbackFunc(const PRECISION_API::PrecisionHitData& a_precisionHitData);

		void EraseStaggerDataFromMap(RE::Actor* a_actor);
		bool FindSpecialStaggerData(RE::Actor* a_actor, SpecialStaggerData& a_data);
		void ProcessSpeicalStagger(RE::Actor* a_actor, std::int32_t a_StaggerLevel);

	private:
		static bool ParseSpecialStaggerData(const std::string_view a_string, SpecialStaggerData& a_data);

		SpecailStaggerTargetMap targets;
		mutable std::mutex m_mutex;
	};
	using SpecialStaggerData = SpecialStaggerHandler::SpecialStaggerData;

	void from_json(const json& j, SpecialStaggerData& a_data);
}