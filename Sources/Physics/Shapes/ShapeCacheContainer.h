#pragma once

#include "EntityComponent/Component.hpp"
#include "Physics/Shapes/Shape.h"
#include "../BoundingVolume.h"

class ShapeCacheContainer
{
	public:
		void clear();
		void add(Shape* shape);

		template<class Visitor> void shapeVisitor(Visitor& visitor) const
		{
			for (const Sphere& elem : m_spheres)
				if (visitor(&elem))
					return;
			for (const Capsule& elem : m_capsules)
				if (visitor(&elem))
					return;
			for (const OrientedBox& elem : m_obbs)
				if (visitor(&elem))
					return;
			for (const Triangle& elem : m_triangles)
				if (visitor(&elem))
					return;
			for (const AxisAlignedBox& elem : m_aabbs)
				if (visitor(&elem))
					return;
			for (const Hull* elem : m_hulls)
				if (visitor(elem))
					return;
		}

		std::vector<Sphere> m_spheres;
		std::vector<Capsule> m_capsules;
		std::vector<OrientedBox> m_obbs;
		std::vector<Triangle> m_triangles;
		std::vector<AxisAlignedBox> m_aabbs;
		std::vector<Hull*> m_hulls;
};


