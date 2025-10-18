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



//	Default
RaycastReport::RaycastReport()
{
	clear();
}
RaycastReport::~RaycastReport()
{

}
//


// Public Methode
void RaycastReport::clear()
{
	m_distance = 1E12f;
	m_intersection = vec4f(0, 0, 0, 1);
	m_normal = vec4f::zero;
	m_entity = nullptr;
	m_shape = nullptr;
}
//


