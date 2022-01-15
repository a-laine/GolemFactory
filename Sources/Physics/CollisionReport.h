#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

#include "RigidBody.h"
#include "Shapes/Shape.h"

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
		void clear();
		//

		// base infos
		bool collision;
		bool computeManifoldContacts;	
		RigidBody *body1, *body2;		
		Entity *entity1, *entity2;
		Shape *shape1, *shape2;

		glm::vec3 normal;
		std::vector<glm::vec3> points;
		std::vector<float> depths;

		std::vector<glm::vec3> shape1face;
		std::vector<glm::vec3> shape2face;
};
