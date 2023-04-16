#pragma once
#include "DKUtil/Hook.hpp"

namespace ModernStaggerLock
{
	using namespace DKUtil::Alias;

	class StaggeredStateCheckPatch
	{
		static constexpr OpCode NOP = 0x90;
		static constexpr OpCode StaggeredCheckNop[6]{ NOP, NOP, NOP, NOP, NOP, NOP };

		// 1-5-97-0 1405FA205
		static constexpr std::uint64_t FuncID = 36700;  //1405fa1b0
		static constexpr std::ptrdiff_t Offset = 0x55;

	public:
		static void Install()
		{
			const auto funcAddr = REL::ID(FuncID).address();
			DKUtil::Hook::WriteData(funcAddr + Offset, &StaggeredCheckNop, sizeof(StaggeredCheckNop));
			INFO("{} Done!", __FUNCTION__);
		}

	private:
		StaggeredStateCheckPatch() = delete;
		~StaggeredStateCheckPatch() = delete;
	};
}