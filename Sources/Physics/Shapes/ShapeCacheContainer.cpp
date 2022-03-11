#include "ShapeCacheContainer.h"




void ShapeCacheContainer::clear()
{
	m_spheres.clear();
	m_capsules.clear();
	m_obbs.clear();
	m_triangles.clear();
	m_aabbs.clear();

	for (Hull* hull : m_hulls)
		delete hull;
	m_hulls.clear();
}
void ShapeCacheContainer::add(Shape* shape)
{
	switch (shape->type)
	{
		case Shape::ShapeType::SPHERE:			 m_spheres.push_back(*static_cast<Sphere*>(shape)); break;
		case Shape::ShapeType::CAPSULE:			 m_capsules.push_back(*static_cast<Capsule*>(shape)); break;
		case Shape::ShapeType::ORIENTED_BOX:	 m_obbs.push_back(*static_cast<OrientedBox*>(shape)); break;
		case Shape::ShapeType::TRIANGLE:		 m_triangles.push_back(*static_cast<Triangle*>(shape)); break;
		case Shape::ShapeType::AXIS_ALIGNED_BOX: m_aabbs.push_back(*static_cast<AxisAlignedBox*>(shape)); break;
		case Shape::ShapeType::HULL:			 m_hulls.push_back(static_cast<Hull*>(shape)); break;
		default: break;
	}
}