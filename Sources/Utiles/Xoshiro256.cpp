#include "Xoshiro256.h"


void Xoshiro256::srand(uint64_t seed)
{
	uint64_t state = seed;
	const auto Splitmix64 = [&state]()
	{
		state += 0x9e3779b97f4a7c15;
		uint64_t z = state;
		z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
		z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
		return z ^ (z >> 31);
	};

	s[0] = Splitmix64();
	s[1] = Splitmix64();
	s[2] = Splitmix64();
	s[3] = Splitmix64();
}

uint64_t Xoshiro256::randomInt()
{
	const uint64_t x = s[0] + s[3];
	const uint64_t result = (x << 23) | (x >> 41) + s[0];
	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];
	s[2] ^= t;
	s[3] = (s[3] << 23) | (s[3] >> 41);

	return result;
}

double Xoshiro256::randomDouble()
{
	uint64_t bitvalue = randomInt() >> 11;
	return reinterpret_cast<double&>(bitvalue);
}
