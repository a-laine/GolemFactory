#pragma once

#include <unordered_map>

//#include "Collision.h"
#include "Constraint.h"
#include "Utiles/Singleton.h"
#include "Resources/Mesh.h"
#include "Scene/BoxSceneQuerry.h"
#include "Scene/SceneManager.h"
#include "RigidBody.h"

#include <set>
#include <Physics/Shapes/ShapeCacheContainer.h>

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
		void drawImGui(World& world);
		//

		//	Set / get ...
		void setGravity(const vec4f& g);
		void setDefaultFriction(const float& f);

		vec4f getGravity() const;
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
				std::vector<RigidBody*> dynamicEntities;
				std::vector<Entity*> staticEntities;
				std::vector<Constraint> constraints;
		};
		//

		//	Pipeline steps
		void predictTransform(const float& elapsedTime);
		void computeBoundingShapesAndDetectPairs(const float& elapsedTime, SceneManager* scene);
		void computeDynamicClusters();
		void createConstraint(const unsigned int& clusterIndex, const float& deltaTime);
		void clearTempoaryStruct(SceneManager* scene);
		//

		//  Solveurs
		void solveConstraint(const unsigned int& clusterIndex, const float& deltaTime);
		//

		//	Usefull functions
		RigidBody::SolverType getSolverType(const std::vector<Entity*>& cluster);
		//

		//	Attributes
		vec4f gravity;
		float defaultFriction;
		std::set<Entity*> movingEntity;

			/// Broad phase
			BoxSceneQuerry proximityTest;
			VirtualEntityCollector proximityList;

			/// Second broad phase and cluster computing
			std::set<std::pair<Entity*, Entity*> > dynamicPairs;
			std::map<Entity*, std::vector<Entity*> > dynamicCollisions;
			std::map<Entity*, std::vector<Entity*> > staticCollisions;
			std::vector<Cluster> clusters;
			EntityGraph clusterFinder;

		//


#ifdef USE_IMGUI
			// options
			bool m_drawCollidersAround = false;
			bool m_drawCollidersWired = false;
			bool m_enableZtest = true;
			float m_drawCollidersQuerySize = 100.f;
			BoxSceneQuerry m_collidersQuery;
			VirtualEntityCollector m_colliderColector;
			Entity* mainCameraEntity;

			// obj thrower
			int m_shapeCode;
			float m_velocity = 20;
			vec3f m_size = vec3f(0.5f);

#endif
};
