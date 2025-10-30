#pragma once

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
		bool computeManifoldContacts = false;	
		RigidBody *body1, *body2;		
		Entity *entity1, *entity2;
		Shape *shape1, *shape2;

		vec4f normal;
		std::vector<vec4f> points;
		std::vector<float> depths;

		std::vector<vec4f> shape1face;
		std::vector<vec4f> shape2face;
};

class RaycastReport
{
public:
	//	Default
	RaycastReport();
	~RaycastReport();
	//


	// Public Methode
	void clear();
	//

	vec4f m_intersection;
	float m_distance;
	vec4f m_normal;
	Entity* m_entity;
	Shape* m_shape;
};