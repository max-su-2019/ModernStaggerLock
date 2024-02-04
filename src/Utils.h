#pragma once

namespace ModernStaggerLock
{
	constexpr char charToLower(const char c)
	{
		return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
	}

}