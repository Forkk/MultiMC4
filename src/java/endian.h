#pragma once
#include <stdint.h>

namespace java
{
	/**
	 * Swap bytes between big endian and local number representation
	 */
	#ifdef MULTIMC_BIG_ENDIAN
	uint64_t bigswap(uint64_t x)
	{
		return x;
	};
	uint32_t bigswap(uint32_t x)
	{
		return x;
	};
	uint16_t bigswap(uint16_t x)
	{
		return x;
	};
	#else
	uint64_t bigswap(uint64_t x)
	{
		return (x>>56) | ((x<<40) & 0x00FF000000000000) | ((x<<24) & 0x0000FF0000000000) | ((x<<8)  & 0x000000FF00000000) |
			   ((x>>8)  & 0x00000000FF000000) | ((x>>24) & 0x0000000000FF0000) | ((x>>40) & 0x000000000000FF00) | (x<<56);
	};
	uint32_t bigswap(uint32_t x)
	{
		return (x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24);
	};
	uint16_t bigswap(uint16_t x)
	{
		return (x>>8) | (x<<8);
	};
	#endif
};
