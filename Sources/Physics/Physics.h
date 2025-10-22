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
#include <Utiles/BankArray.h>

class Physics
{
	public:
		//	Debug
		static bool drawSweptBoxes;
		static bool drawClustersAABB; 
		static bool drawCollisions;
		//

		//	Miscellaneous
		class CollisionCache
		{
			public:
				class Element
				{
					public:
						Shape* m_shape;
						Entity* m_entity;
				};

				AxisAlignedBox m_aabb;
				BankArray<Triangle> m_triangles;
				BankArray<Sphere> m_spheres;
				BankArray<OrientedBox> m_boxes;
				BankArray<Capsule> m_capsules;
				//BankArray<Hull> m_hulls;

				std::vector<Element> m_elements;

				void clear();
				void debugDraw(bool wireframe, vec4f baseColor) const;
		};
		class Cluster
		{
			public:
				std::vector<RigidBody*> dynamicEntities;
				//std::vector<Entity*> staticEntities;
				std::vector<Constraint> constraints;

				CollisionCache cache;
		};
		//

		//  Default
		Physics();
		~Physics();
		//

		//	Public functions
		void stepSimulation(const float& elapsedTime, SceneManager* scene);
		void stepSimulation2(const float& elapsedTime, SceneManager* scene);

		bool raycast(Segment ray, SceneManager* scene, uint64_t flags, uint64_t noFlags, bool skipTriggers = true, RaycastReport* _report = nullptr);

		bool raycastInCache(const CollisionCache& cache, Segment ray, SceneManager* scene, uint64_t flags, uint64_t noFlags, RaycastReport* _report = nullptr);
		bool collisionInCache(const CollisionCache& cache, Shape* shape, SceneManager* scene, uint64_t flags, uint64_t noFlags, CollisionReport* _report = nullptr);

		void getCollisionCache(SceneManager* scene, CollisionCache& cache, uint64_t flags, uint64_t noFlags);


		//bool collisionTest(const Shape& _shape, SceneManager* scene, uint64_t flags, uint64_t noFlags, CollisionReport* _report = nullptr);

		void debugDraw();
		void drawImGui(World& world);
		//

		//	Set / get ...
		void setGravity(const vec4f& g);
		void setDefaultFriction(const float& f);

		vec4f getGravity() const;
		float getDefaultFriction() const;

		void addMovingEntity(Entity* e);
		void removeMovingEntity(Entity* e);
		Cluster* getCLuster(int clusterIndex);
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
		//

		//	Pipeline steps
		void predictTransform(const float& elapsedTime);
		void predictTransform2(const unsigned int& clusterIndex, const float& deltaTime);
		void computeBoundingShapesAndDetectPairs(const float& elapsedTime, SceneManager* scene);
		void computeDynamicClusters(SceneManager* scene);
		void computeBoundingPairsClusters2(const float& elapsedTime, SceneManager* scene);
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
		BankArray<RigidBody*> m_physicObjList;

			/// Broad phase
			//BoxSceneQuerry proximityTest;
			//VirtualEntityCollector proximityList;

			/// Second broad phase and cluster computing
			std::set<std::pair<Entity*, Entity*> > dynamicPairs;
			std::map<Entity*, std::vector<Entity*> > dynamicCollisions;
			//std::map<Entity*, std::vector<Entity*> > staticCollisions;
			BankArray<Cluster> m_clusters;
			EntityGraph clusterFinder;

		//


#ifdef USE_IMGUI
			// options
			bool m_drawCollidersAround = false;
			bool m_drawCollidersWired = false;
			bool m_autoSelectThrowedObject = true;
			bool m_enableZtest = true;
			float m_drawCollidersQuerySize = 100.f;
			BoxSceneQuerry m_collidersQuery;
			VirtualEntityCollector m_colliderColector;
			Entity* mainCameraEntity = nullptr;

			// obj thrower
			int m_shapeCode = 0;
			float m_velocity = 20;
			vec3f m_size = vec3f(0.5f);

#endif
};
