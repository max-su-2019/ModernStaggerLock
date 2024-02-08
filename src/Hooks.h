#pragma once

namespace ModernStaggerLock
{
	class NotifyAnimationGraphHook
	{
	public:
		class CharacterEx : public RE::Character
		{
		public:
			static void InstallHook()
			{
				REL::Relocation<std::uintptr_t> CharacterVtbl{ VTABLE[3] };
				func = CharacterVtbl.write_vfunc(0x1, &Hook_NotifyAnimationGraph);
				INFO("Hook NPC NotifyAnimationGraph!");
			}

		private:
			static bool Hook_NotifyAnimationGraph(RE::IAnimationGraphManagerHolder* a_graphMgr, const RE::BSFixedString& a_eventName)
			{
				if (ProcessStaggerHanlder(a_graphMgr, a_eventName)) {
					//
				} else if (ShouldQuickRecovery(a_graphMgr, a_eventName)) {
					func(a_graphMgr, "StaggerStop");
				}

				return func(a_graphMgr, a_eventName);
			}

			static inline REL::Relocation<decltype(&RE::Character::NotifyAnimationGraph)> func;
		};

		class PlayerEx : public RE::PlayerCharacter
		{
		public:
			static void InstallHook()
			{
				REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ VTABLE[3] };
				func = PlayerCharacterVtbl.write_vfunc(0x1, &Hook_NotifyAnimationGraph);
				INFO("Hook PC NotifyAnimationGraph!");
			}

		private:
			static bool Hook_NotifyAnimationGraph(RE::IAnimationGraphManagerHolder* a_graphMgr, const RE::BSFixedString& a_eventName)
			{
				if (ProcessStaggerHanlder(a_graphMgr, a_eventName)) {
					//
				} else if (ShouldQuickRecovery(a_graphMgr, a_eventName)) {
					func(a_graphMgr, "StaggerStop");
				}

				return func(a_graphMgr, a_eventName);
			}

			static inline REL::Relocation<decltype(&RE::PlayerCharacter::NotifyAnimationGraph)> func;
		};

		friend class CharacterEx;
		friend class PlayerEx;

	private:
		static bool ProcessStaggerHanlder(RE::IAnimationGraphManagerHolder* a_graphMgr, const RE::BSFixedString& a_eventName);
		static bool ShouldQuickRecovery(RE::IAnimationGraphManagerHolder* a_graphMgr, const RE::BSFixedString& a_eventName);

		NotifyAnimationGraphHook() = delete;
		~NotifyAnimationGraphHook() = delete;
	};

	class DisableStaggerJumpHook : public RE::JumpHandler
	{
	public:
		static void InstallHook()
		{
			REL::Relocation<std::uintptr_t> JumpHandlerVtbl{ VTABLE[0] };
			func = JumpHandlerVtbl.write_vfunc(0x1, &Hook_CanProcess);
			INFO("Hook DisableStaggerJumpHook!");
		}

	private:
		static bool Hook_CanProcess(RE::JumpHandler* a_handler, RE::InputEvent* a_event);

		static inline REL::Relocation<decltype(&RE::JumpHandler::CanProcess)> func;
	};

	using EventResult = RE::BSEventNotifyControl;

	class AnimEventHook
	{
	public:
		static void InstallHook()
		{
			REL::Relocation<std::uintptr_t> AnimEventSinkVtbl{ RELOCATION_ID(261399, 207890) };
			_ProcessEvent = AnimEventSinkVtbl.write_vfunc(0x1, ProcessEvent);
			INFO("Hook Process Animation Event!");
		}

	private:
		static EventResult ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);

		static inline REL::Relocation<decltype(ProcessEvent)> _ProcessEvent;
	};

	class ActorUpdateHook
	{
	public:
		class CharacterEx : public RE::Character
		{
		public:
			static void InstallHook()
			{
				REL::Relocation<std::uintptr_t> CharacterVtbl{ VTABLE[0] };
				func = CharacterVtbl.write_vfunc(0x0AD, &Hook_Update);
				INFO("Hook Actor Update!");
			}

		private:
			void Hook_Update(float a_delta);

			static inline REL::Relocation<decltype(&RE::Character::Update)> func;
		};

		class PlayerEx : public RE::PlayerCharacter
		{
		public:
			static void InstallHook()
			{
				REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ VTABLE[0] };
				func = PlayerCharacterVtbl.write_vfunc(0x0AD, &Hook_Update);
				INFO("Hook Actor Update!");
			}

		private:
			void Hook_Update(float a_delta);

			static inline REL::Relocation<decltype(&RE::PlayerCharacter::Update)> func;
		};

		friend class CharacterEx;
		friend class PlayerEx;

		ActorUpdateHook() = delete;
		~ActorUpdateHook() = delete;
	};

	class PerformLandActionHook_NPC
	{
	public:
		static void InstallHook()
		{
			SKSE::AllocTrampoline(1 << 4);
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> callerFunc{ REL::ID(36507) };  //1.5.97 1405E4880
			_PerformAction = trampoline.write_call<5>(callerFunc.address() + 0X89D, PerformLandAction);
			INFO("{} Done!", __FUNCTION__);
		}

	private:
		static bool PerformLandAction(RE::TESActionData* a_data);

		static inline REL::Relocation<decltype(PerformLandAction)> _PerformAction;
	};

	class PerformLandActionHook_PC
	{
	public:
		static void InstallHook()
		{
			SKSE::AllocTrampoline(1 << 4);
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> callerFunc{ REL::ID(41271) };  //1.5.97 140705530
			_PerformAction = trampoline.write_call<5>(callerFunc.address() + 0x241, PerformLandAction);
			INFO("{} Done!", __FUNCTION__);
		}

	private:
		static bool PerformLandAction(RE::TESActionData* a_data);

		static inline REL::Relocation<decltype(PerformLandAction)> _PerformAction;
	};
}