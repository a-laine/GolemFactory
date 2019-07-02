#pragma once

#include <unordered_map>

#include "Collision.h"
#include "Intersection.h"
#include "Utiles/Singleton.h"
#include "Resources/Mesh.h"
//#include "Scene/SceneManager.h"
#include "Scene/BoxSceneQuerry.h"

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
		//	Miscellaneous
		class ArtefactsGraph
		{
			public:
				void clear();
				void initialize(const std::set<PhysicsArtefacts>& n);
				void addLink(const PhysicsArtefacts& n1, const PhysicsArtefacts& n2);
				std::vector<std::vector<PhysicsArtefacts*> > getCluster();

			private:
				void getNeighbours(PhysicsArtefacts* node, std::vector<PhysicsArtefacts*>& result);

				std::set<PhysicsArtefacts> nodes;
				std::map<PhysicsArtefacts*, std::pair<std::set<PhysicsArtefacts*>, bool> > graph;
		};
		//

		//	Pipeline steps
		void predictTransform(const float& elapsedTime);
		void computeBoundingShapesAndDetectPairs(const float& elapsedTime, SceneManager* scene);
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
		BoxSceneQuerry proximityList;
		std::vector<Swept*> sweptList;
		std::vector<NodeVirtual*> updatedNodes;
		std::set<std::pair<PhysicsArtefacts, PhysicsArtefacts> > collidingPairs;
		std::vector<Intersection::Contact> collisionList;
		ArtefactsGraph clusterFinder;
		//
};
