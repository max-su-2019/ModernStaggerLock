#pragma once

namespace ModernStaggerLock
{
	class PeformStagggerHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> hook{ REL::ID(36700) };  //1405FA1B0		1.5.97
			auto& trampoline = SKSE::GetTrampoline();
			SKSE::AllocTrampoline(1 << 4);
			_PerformActionImp = trampoline.write_call<5>(hook.address() + 0X1E6, PerformActionImp);
		};

	private:
		static std::int32_t PerformActionImp(RE::TESActionData* a_action, float a2);

		static inline REL::Relocation<decltype(PerformActionImp)> _PerformActionImp;
	};

}