#pragma once

#include "EntityComponent/Component.hpp"
#include "Physics/Shapes/Shape.h"


class Collider : public Component
{
	GF_DECLARE_COMPONENT_CLASS(Collider, Component)

	public:
		//	Default
		Collider(Shape* _shape = nullptr);
		Collider(const Collider* other);
		virtual ~Collider();
		//

		bool load(Variant& jsonObject, const std::string& objectName) override;
		void save(Variant& jsonObject) override;

		void onDrawImGui() override;
		void onAddToEntity(Entity* entity) override;
		void drawDebug(vec4f color, bool wired = true) const;

		//	Attributes
		Shape* m_shape;
		//

	protected:
	#ifdef USE_IMGUI
		bool m_drawShape = false;
	#endif
};

