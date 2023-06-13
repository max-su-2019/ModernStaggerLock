#include "Settings.h"

namespace ModernStaggerLock
{
	static auto MainConfig = COMPILE_PROXY("ModernStaggerLock.ini"sv);

	MSLSettings::MSLSettings()
	{
		MainConfig.Bind<0.0, 1.0>(SmallStaggerMagnitude, 0.0);
		MainConfig.Bind<0.0, 1.0>(MediumStaggerMagnitude, 0.5);
		MainConfig.Bind<0.0, 1.0>(LargeStaggerMagnitude, 0.75);
		MainConfig.Bind<0.0, 1.0>(LargestStaggerMagnitude, 1.0);

		MainConfig.Load();

		PrintSettingValue(SmallStaggerMagnitude);
		PrintSettingValue(MediumStaggerMagnitude);
		PrintSettingValue(LargeStaggerMagnitude);
		PrintSettingValue(LargestStaggerMagnitude);
	}
}