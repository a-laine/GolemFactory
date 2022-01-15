#pragma once

//#include "Collision.h"
#include "GJK.h"

template<typename A, typename B>
bool Collision::collide(const A* a, const B* b, CollisionReport* report)
{
	return GJK::collide(*(Shape*)a, *(Shape*)b, report);
}