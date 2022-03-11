#pragma once

#include "EntityComponent/Component.hpp"
#include "Physics/Shapes/Shape.h"


class Collider : public Component
{
	GF_DECLARE_COMPONENT_CLASS(Collider, Component)

	public:
		//	Default
		Collider(Shape* _shape = nullptr);
		virtual ~Collider();
		//

		//	Attributes
		Shape* m_shape;
		//
};

