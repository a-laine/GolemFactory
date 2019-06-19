#pragma once

#include "EntityComponent/Entity.hpp"
#include "Physics/BoundingVolume.h"

class Swept
{
	public:
		//	Default
		Swept(Entity* e);
		//

		//	Set / get functions
		const AxisAlignedBox& getBox() const;
		//

	protected:
		//	Attributes
		Entity* entity;
		AxisAlignedBox box;
		glm::vec3 dp;
		glm::fquat dq;
		//
};

class PhysicsArtefacts
{
	public:
		//	Msc
		enum Type
		{
			ENTITY = 0,
			SWEPT
		};
		//

		//	Default
		PhysicsArtefacts(Entity* e);
		PhysicsArtefacts(Swept* s);
		//

		//	Attributes
		Type type;
		union {
			Entity* entity;
			Swept* swept;
		} data;
		//
};