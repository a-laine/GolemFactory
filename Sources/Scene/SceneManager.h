#pragma once

#include <vector>
#include <set>
#include <unordered_map>
#include <glm/glm.hpp>

#include "NodeVirtual.h"
#include "VirtualSceneQuerry.h"

class SceneManager
{
	public:
		//	Default
		SceneManager();
		SceneManager(const SceneManager& other) = delete;
		SceneManager(SceneManager&& other);
		~SceneManager();

		SceneManager& operator=(const SceneManager& other) = delete;
		SceneManager& operator=(SceneManager&& other);
		//

		//	Public functions
		void init(const vec4f& bbMin, const vec4f& bbMax, const vec3i& nodeDivision, unsigned int depth);
		void clear();
		void reserveInstanceTrack(const unsigned int& count);
		unsigned int getObjectCount() const;
		//

		//	Object / Entity related
		bool addObject(Entity* object);
		bool removeObject(Entity* object);
		bool updateObject(Entity* object);
		void addToRootList(Entity* object);

		std::vector<Entity*> getAllObjects();
		Entity* searchEntity(const std::string& _name);
		Entity* getLastSelectedEntity()const;
		std::vector<Entity*> getObjectsOnRay(const vec4f& position, const vec4f& direction, float maxDistance);
		std::vector<Entity*> getObjectsInBox(const vec4f& bbMin, const vec4f& bbMax);

		void getSceneNodes(VirtualSceneQuerry* collisionTest);
		void getEntities(VirtualSceneQuerry* collisionTest, VirtualEntityCollector* entityCollector);
		//

		//	Debug
		void drawImGuiHierarchy(World& world, bool drawSelectedEntityWindow);
		void drawImGuiSpatialPartitioning(World& world);
		void drawSceneNodes();
		void selectEntity(World& world, Entity* entity);
		//

	private:
		//	Miscellaneous
		struct InstanceTrack
		{
			vec4f position;
			NodeVirtual* owner;
		};

		//	Protected functions
		//vec4f getObjectSize(const Entity* entity) const;
		//

		//  Attributes
		std::vector<NodeVirtual*> world;
		std::unordered_map<Entity*, InstanceTrack> instanceTracking;
		std::vector<Entity*> roots;
		//

		//	Debug
#ifdef USE_IMGUI
		void drawRecursiveImGuiEntity(World& world, Entity* entity, int depth);
		void drawRecursiveImGuiSceneNode(World& world, std::map<const NodeVirtual*, VirtualSceneQuerry::CollisionType>& collisionResults, NodeVirtual* node, vec3i nodeIndex, int depth);
		bool isEmptyNode(const NodeVirtual& _node);

		ImGuiTextFilter m_nameFilter;
		uint64_t m_flagFilter = 0xFFFFFFFFFFFFFFFF;
		bool m_allFlagFilter = true;
		std::set<Entity*> m_selectedEntities;
		Entity* m_lastSelectedEntity;
		NodeVirtual* m_selectedSceneNode = nullptr;

		bool m_showEmptyNodes = true;
		bool m_showFailTestNodes = true;
		bool m_printEntities = false;
		bool m_printEmptyNodes = true;
		bool m_showfrustrum = false;
		bool m_openAll = false;
		float m_nodeShrinkFactor;
		vec4f m_emptyNodeColor = vec4f(0,0,0,1);
		vec4f m_defaultNodeColor = vec4f(1,0,0,1);
		vec4f m_testBoxNodeColor = vec4f(1,1,1,1);
#endif // USE_IMGUI
		//
};
