#include "CollisionReport.h"


CollisionReport::CollisionReport()
{
	clear();
}
CollisionReport::~CollisionReport()
{

}
//

// Public Methode
void CollisionReport::clear()
{
	collision = false;

	shape1 = nullptr;
	shape2 = nullptr;
	body1 = nullptr;
	body2 = nullptr;

	points.clear();
	depths.clear();
	shape1face.clear();
	shape2face.clear();
}
//





