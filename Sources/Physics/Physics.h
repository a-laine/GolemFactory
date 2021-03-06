#pragma once

#include <unordered_map>

#include "Collision.h"
#include "Intersection.h"
#include "Utiles/Singleton.h"
#include "Resources/Mesh.h"
#include "Scene/BoxSceneQuerry.h"
#include "Scene/SceneManager.h"
#include "RigidBody.h"

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
		class EntityGraph
		{
			public:
				void clear();
				void initialize(const std::set<Entity*>& n);
				void addLink(const Entity* n1, const Entity* n2);
				std::vector<std::vector<Entity*> > getCluster();

			private:
				void getNeighbours(Entity* node, std::vector<Entity*>& result);

				std::set<Entity*> nodes;
				std::map<Entity*, std::pair<std::set<Entity*>, bool> > graph;
		};
		//

		//	Pipeline steps
		void predictTransform(const float& elapsedTime);
		void computeBoundingShapesAndDetectPairs(const float& elapsedTime, SceneManager* scene);
		void computeClusters();
		
		void clearTempoaryStruct(SceneManager* scene);
		//

		//  Solveurs
		void impactSolver(const Intersection::Contact& contact, RigidBody* rb1, RigidBody* rb2, const glm::vec3& actionLine);
		void discreteSolver(const std::pair<std::vector<Entity*>, std::vector<Entity*> >& cluster);
		void continuousSolver(const std::pair<std::vector<Entity*>, std::vector<Entity*> >& cluster);
		void supersamplingSolver(const std::pair<std::vector<Entity*>, std::vector<Entity*> >& cluster);
		//

		//	Usefull functions
		RigidBody::SolverType getSolverType(const std::vector<Entity*>& cluster);
		/*Mesh* extractMesh(Entity* entity) const;
		bool extractIsAnimatable(Entity* entity) const;*/
		//

		//	Attributes
		glm::vec3 gravity;
		std::set<Entity*> movingEntity;
		BoxSceneQuerry proximityTest;
		VirtualEntityCollector proximityList;
		std::vector<Swept*> sweptList;
		std::set<std::pair<Entity*, Entity*> > collidingPairs;
		std::vector<std::pair<std::vector<Entity*>, std::vector<Entity*> > > clusters;
		std::vector<Intersection::Contact> collisionList;
		EntityGraph clusterFinder;
		//
};
