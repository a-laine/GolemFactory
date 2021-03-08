#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

#include "RigidBody.h"

class Entity;
class RigidBody;
class CollisionReport
{
	public:
		//	Default
		CollisionReport();
		~CollisionReport();
		//

		// Public Methode
		void Clear();
		//

		// base infos
		bool collision;					// if the two objects are in collision
		RigidBody *body1, *body2;		
		Entity *entity1, *entity2;

		// GJK / EPA output
		glm::vec3 contactPoint1, contactPoint2;
		glm::vec3 normal1, normal2;
};
