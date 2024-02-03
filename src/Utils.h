#pragma once

namespace ModernStaggerLock
{
	constexpr char charToLower(const char c)
	{
		return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
	}

	constexpr int32_t HashToInt(const char* data, size_t const size) noexcept;

}