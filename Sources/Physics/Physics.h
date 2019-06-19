#pragma once

#include <unordered_map>

#include "Collision.h"
#include "Intersection.h"
#include "Utiles/Singleton.h"
#include "Resources/Mesh.h"
//#include "Scene/SceneManager.h"
#include "Scene/SceneQueryTests.h"

#include <set>

class Physics
{
	public:
		//  Default
		Physics();
		~Physics();
		//

		//	Public functions
		void stepSimulation(const float& elapsedTime, SceneManager* s);
		//

		//	Set / get ...
		void setGravity(const glm::vec3& g);

		glm::vec3 getGravity() const;

		void addMovingEntity(Entity* e);
		//

	private:
		//	Pipeline steps
		void predictTransform(const float& elapsedTime);
		void computeBoundingShapes(const float& elapsedTime, SceneManager* scene);
		void detectPairs(const float& elapsedTime);
		void computeContacts(const float& elapsedTime);
		void solveConstraints(const float& elapsedTime);
		void integratePositions(const float& elapsedTime);
		void clearTepoaryStruct();
		//

		//	Usefull functions
		Mesh* extractMesh(Entity* entity) const;
		bool extractIsAnimatable(Entity* entity) const;
		//

		//	Attributes
		glm::vec3 gravity;
		std::set<Entity*> movingEntity;
		DefaultBoxCollector proximityList;
		std::vector<Swept*> sweptList;
		std::vector<NodeVirtual*> updatedNodes;
		std::set<std::pair<Entity*, Entity*> > collidingPairs;
		std::vector<Intersection::Contact> collisionList;
		//
};
