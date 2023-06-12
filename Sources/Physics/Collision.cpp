#include "Collision.h"
#include "Utiles/Assert.hpp"
#include "Resources/Mesh.h"



Collision::CollisionTest Collision::dispatchMatrix[8][8] = {
	{nullptr, nullptr , nullptr , nullptr , nullptr , nullptr , nullptr , nullptr },
	{nullptr, nullptr , nullptr , nullptr , nullptr , nullptr , nullptr , nullptr },
	{nullptr, nullptr , nullptr , nullptr , nullptr , nullptr , nullptr , nullptr },
	{nullptr, nullptr , nullptr , nullptr , nullptr , nullptr , nullptr , nullptr },
	{nullptr, nullptr , nullptr , nullptr , nullptr , nullptr , nullptr , nullptr },
	{nullptr, nullptr , nullptr , nullptr , nullptr , nullptr , nullptr , nullptr },
	{nullptr, nullptr , nullptr , nullptr , nullptr , nullptr , nullptr , nullptr }
};

bool Collision::collide(const Shape* a, const Shape* b, CollisionReport* report)
{
	GF_ASSERT(((int)a->type >= 0 && (int)a->type <= (int)Shape::ShapeType::HULL), "Collision not in dispatch matrix");
	GF_ASSERT(((int)b->type >= 0 && (int)b->type <= (int)Shape::ShapeType::HULL), "Collision not in dispatch matrix");

	return (dispatchMatrix[(int)(a->type)][(int)(b->type)])(a, b, report);
}


void Collision::DispatchMatrixInit()
{
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
		{
			dispatchMatrix[i][j] = _GJKCollision;
		}

	dispatchMatrix[(int)Shape::ShapeType::POINT][(int)Shape::ShapeType::POINT] = _PointvsPoint;
	dispatchMatrix[(int)Shape::ShapeType::POINT][(int)Shape::ShapeType::SEGMENT] = _PointvsSegment;
	dispatchMatrix[(int)Shape::ShapeType::POINT][(int)Shape::ShapeType::TRIANGLE] = _PointvsTriangle;
	dispatchMatrix[(int)Shape::ShapeType::POINT][(int)Shape::ShapeType::SPHERE] = _PointvsSphere;
	dispatchMatrix[(int)Shape::ShapeType::POINT][(int)Shape::ShapeType::ORIENTED_BOX] = _PointvsOrientedBox;
	dispatchMatrix[(int)Shape::ShapeType::POINT][(int)Shape::ShapeType::AXIS_ALIGNED_BOX] = _PointvsAxisAlignedBox;
	dispatchMatrix[(int)Shape::ShapeType::POINT][(int)Shape::ShapeType::CAPSULE] = _PointvsCapsule;
	dispatchMatrix[(int)Shape::ShapeType::POINT][(int)Shape::ShapeType::HULL] = _PointvsHull;

	dispatchMatrix[(int)Shape::ShapeType::SEGMENT][(int)Shape::ShapeType::POINT] = _SegmentvsPoint;
	dispatchMatrix[(int)Shape::ShapeType::SEGMENT][(int)Shape::ShapeType::SEGMENT] = _SegmentvsSegment;
	dispatchMatrix[(int)Shape::ShapeType::SEGMENT][(int)Shape::ShapeType::TRIANGLE] = _SegmentvsTriangle;
	dispatchMatrix[(int)Shape::ShapeType::SEGMENT][(int)Shape::ShapeType::SPHERE] = _SegmentvsSphere;
	dispatchMatrix[(int)Shape::ShapeType::SEGMENT][(int)Shape::ShapeType::CAPSULE] = _SegmentvsCapsule;

	dispatchMatrix[(int)Shape::ShapeType::SPHERE][(int)Shape::ShapeType::POINT] = _SpherevsPoint;
	dispatchMatrix[(int)Shape::ShapeType::SPHERE][(int)Shape::ShapeType::SEGMENT] = _SpherevsSegment;
	dispatchMatrix[(int)Shape::ShapeType::SPHERE][(int)Shape::ShapeType::SPHERE] = _SpherevsSphere;
	dispatchMatrix[(int)Shape::ShapeType::SPHERE][(int)Shape::ShapeType::ORIENTED_BOX] = _SpherevsOrientedBox;
	dispatchMatrix[(int)Shape::ShapeType::SPHERE][(int)Shape::ShapeType::AXIS_ALIGNED_BOX] = _SpherevsAxisAlignedBox;
	dispatchMatrix[(int)Shape::ShapeType::SPHERE][(int)Shape::ShapeType::CAPSULE] = _SpherevsCapsule;

	dispatchMatrix[(int)Shape::ShapeType::ORIENTED_BOX][(int)Shape::ShapeType::SPHERE] = _OrientedBoxvsSphere;

}





bool Collision::_GJKCollision(const Shape* a, const Shape* b, CollisionReport* report)
{
	return GJK::collide(*a, *b, report);
}

// Point dispatch
bool Collision::_PointvsPoint(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Point& point1 = *(Point*)a;
	const Point& point2 = *(Point*)b;
	return collide_PointvsPoint(point1.p, point2.p, report);
}
bool Collision::_PointvsSegment(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Point& point = *(Point*)a;
	const Segment& segment = *(Segment*)b;
	return collide_PointvsSegment(point.p, segment.p1, segment.p2, report);
}
bool Collision::_PointvsTriangle(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Point& point = *(Point*)a;
	const Triangle& triangle = *(Triangle*)b;
	return collide_PointvsTriangle(point.p, triangle.p1, triangle.p2, triangle.p3, report);
}
bool Collision::_PointvsSphere(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Point& point = *(Point*)a;
	const Sphere& sphere = *(Sphere*)b;
	return collide_PointvsSphere(point.p, sphere.center, sphere.radius, report);
}
bool Collision::_PointvsOrientedBox(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Point& point = *(Point*)a;
	const OrientedBox& box = *(OrientedBox*)b;
	return collide_PointvsOrientedBox(point.p, box.base, box.min, box.max, report);
}
bool Collision::_PointvsCapsule(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Point& point = *(Point*)a;
	const Capsule& capsule = *(Capsule*)b;
	return collide_PointvsCapsule(point.p, capsule.p1, capsule.p2, capsule.radius, report);
}
bool Collision::_PointvsHull(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Point& point = *(Point*)a;
	const Hull& hull = *(Hull*)b;
	return collide_PointvsHull(point.p, *hull.mesh->getVertices(), *hull.mesh->getNormals(), *hull.mesh->getFaces(), hull.base, report);
}
bool Collision::_PointvsAxisAlignedBox(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Point& point = *(Point*)a;
	const AxisAlignedBox& box = *(AxisAlignedBox*)b;
	return collide_PointvsAxisAlignedBox(point.p, box.min, box.max, report);
}

// Segment dispatch
bool Collision::_SegmentvsPoint(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Segment& segment = *(Segment*)a;
	const Point& point = *(Point*)b;
	return collide_SegmentvsPoint(point.p, segment.p1, segment.p2, report);
}
bool Collision::_SegmentvsSegment(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Segment& segment1 = *(Segment*)a;
	const Segment& segment2 = *(Segment*)b;
	return collide_SegmentvsSegment(segment1.p1, segment1.p2, segment2.p1, segment2.p2, report);
}
bool Collision::_SegmentvsTriangle(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Segment& segment = *(Segment*)a;
	const Triangle& triangle = *(Triangle*)b;
	return collide_SegmentvsTriangle(segment.p1, segment.p2, triangle.p1, triangle.p2, triangle.p3, report);
}
bool Collision::_SegmentvsSphere(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Segment& segment = *(Segment*)a;
	const Sphere& sphere = *(Sphere*)b;
	return collide_SegmentvsSphere(segment.p1, segment.p2, sphere.center, sphere.radius, report);
}
bool Collision::_SegmentvsCapsule(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Segment& segment = *(Segment*)a;
	const Capsule& capsule = *(Capsule*)b;
	return collide_SegmentvsCapsule(segment.p1, segment.p2, capsule.p1, capsule.p2, capsule.radius, report);
}

// Sphere dispatch
bool Collision::_SpherevsPoint(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Sphere& sphere = *(Sphere*)a;
	const Point& point = *(Point*)b;
	return collide_SpherevsPoint(point.p, sphere.center, sphere.radius, report);
}
bool Collision::_SpherevsSegment(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Sphere& sphere = *(Sphere*)a;
	const Segment& segment = *(Segment*)b;
	return collide_SpherevsSegment(segment.p1, segment.p2, sphere.center, sphere.radius, report);
}
bool Collision::_SpherevsSphere(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Sphere& sphere1 = *(Sphere*)a;
	const Sphere& sphere2 = *(Sphere*)b;
	return collide_SpherevsSphere(sphere1.center, sphere1.radius, sphere2.center, sphere2.radius, report);
}
bool Collision::_SpherevsOrientedBox(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Sphere& sphere = *(Sphere*)a;
	const OrientedBox& box = *(OrientedBox*)b;
	return collide_SpherevsOrientedBox(sphere.center, sphere.radius, box.base, box.min, box.max, report);
}
bool Collision::_SpherevsAxisAlignedBox(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Sphere& sphere = *(Sphere*)a;
	const AxisAlignedBox& box = *(AxisAlignedBox*)b;
	return collide_SpherevsAxisAlignedBox(box.min, box.max, sphere.center, sphere.radius, report);
}
bool Collision::_SpherevsCapsule(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Sphere& sphere = *(Sphere*)a;
	const Capsule& capsule = *(Capsule*)b;
	return collide_SpherevsCapsule(sphere.center, sphere.radius, capsule.p1, capsule.p2, capsule.radius, report);
}

// OrientedBox dispatch
bool Collision::_OrientedBoxvsSphere(const Shape* a, const Shape* b, CollisionReport* report)
{
	const Sphere& sphere = *(Sphere*)b;
	const OrientedBox& box = *(OrientedBox*)a;
	return collide_OrientedBoxvsSphere(sphere.center, sphere.radius, box.base, box.min, box.max, report);
}



