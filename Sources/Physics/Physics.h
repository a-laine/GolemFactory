#pragma once

#include <unordered_map>

#include "Collision.h"
#include "Constraint.h"
#include "Utiles/Singleton.h"
#include "Resources/Mesh.h"
#include "Scene/BoxSceneQuerry.h"
#include "Scene/SceneManager.h"
#include "RigidBody.h"

#include <set>

class Physics
{
	public:
		//	Debug
		static bool drawSweptBoxes;
		static bool drawClustersAABB; 
		static bool drawCollisions;
		//

		//  Default
		Physics();
		~Physics();
		//

		//	Public functions
		void stepSimulation(const float& elapsedTime, SceneManager* scene);

		void debugDraw();
		void drawImGui();
		//

		//	Set / get ...
		void setGravity(const glm::vec3& g);
		void setDefaultFriction(const float& f);

		glm::vec3 getGravity() const;
		float getDefaultFriction() const;

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

				std::map<Entity*, std::pair<std::set<Entity*>, bool> > graph;
		};
		class Cluster
		{
			public:
				std::vector<Entity*> dynamicEntities;
				std::vector<RigidBody*> bodies;
				std::vector<Entity*> staticEntities;
				std::vector<Constraint> constraints;
		};
		//

		//	Pipeline steps
		void predictTransform(const float& elapsedTime);
		void computeBoundingShapesAndDetectPairs(const float& elapsedTime, SceneManager* scene);
		void computeClusters();
		void createConstraint(const unsigned int& clusterIndex, const float& deltaTime);
		void clearTempoaryStruct(SceneManager* scene);
		//

		//  Solveurs
		void solveConstraint(const unsigned int& clusterIndex, const float& deltaTime);
		//

		//	Usefull functions
		RigidBody::SolverType getSolverType(const std::vector<Entity*>& cluster);
		//void createReportConstraints(Cluster& cluster, CollisionReport& report);
		//

		//	Attributes
		glm::vec3 gravity;
		float defaultFriction;
		std::set<Entity*> movingEntity;

			/// Broad phase
			BoxSceneQuerry proximityTest;
			VirtualEntityCollector proximityList;
			std::vector<Swept*> sweptList;

			/// Second broad phase and cluster computing
			std::set<std::pair<Entity*, Entity*> > dynamicPairs;
			std::map<Entity*, std::vector<Entity*> > dynamicCollisions;
			std::map<Entity*, std::vector<Entity*> > staticCollisions;
			std::vector<Cluster> clusters;
			EntityGraph clusterFinder;

		//
};
