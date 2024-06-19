#pragma once
#include <cstdint>


struct Xoshiro256
{
	void srand(uint64_t seed);
	uint64_t randomInt();
	double randomDouble();

	uint64_t s[4];
};