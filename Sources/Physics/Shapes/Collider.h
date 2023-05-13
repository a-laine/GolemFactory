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

		void onDrawImGui() override;
		void onAddToEntity(Entity* entity) override;
		void drawDebug(vec4f color) const;

		//	Attributes
		Shape* m_shape;
		//

	protected:
	#ifdef USE_IMGUI
		bool m_drawShape = false;
	#endif
};

