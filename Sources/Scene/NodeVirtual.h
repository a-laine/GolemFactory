#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "EntityComponent/Entity.hpp"
#include "Physics/Swept.h"

class World;

/*! \class NodeVirtual
 *  \brief Base class for node implementation.
 *
 *	A node can be represented by a cube container.
 *  For more information about scene manager node read about quadtree or octtree implementation in literature
 */
class NodeVirtual
{
	public:
		class NodeRange
		{
			friend class NodeVirtual;

			public:
				NodeRange(std::vector<NodeVirtual>& nodes) : begin(nodes.data()), end(nodes.data() + nodes.size()) {}
				NodeRange(NodeVirtual* first, NodeVirtual* last) : begin(first), end(last) {}
				void next() { if(begin != end) ++begin; }
				NodeVirtual* get() { return begin; }
				bool empty() const { return begin == end; }

			private:
				NodeVirtual* begin;
				NodeVirtual* end;
		};

		//	Default
		NodeVirtual();
		NodeVirtual(const NodeVirtual& other) = delete;
		NodeVirtual(NodeVirtual&& other) = default;
		virtual ~NodeVirtual();

		//	Public fonctions
		void init(const glm::vec3 bbMin, const glm::vec3 bbMax, const glm::ivec3& nodeDivision, unsigned int depth);
		void clearChildren();
		void split(unsigned int newDepth);
		void merge(unsigned int newDepth);
		//

		//	Set / get functions
		bool isLeaf() const;
		glm::vec3 getCenter() const;
		glm::vec3 getSize() const;
		glm::vec3 getBBMax() const;
		glm::vec3 getBBMin() const;
		int getChildrenCount() const;
		bool isInside(const glm::vec3& point) const;
		bool isTooSmall(const glm::vec3& size) const;
		bool isTooBig(const glm::vec3& size) const;
		glm::vec3 getPosition() const;
		const float& getAllowanceSize() const;
		//
		
		//	Hierarchy related function
		void addNode(NodeVirtual* node);
		bool removeNode(NodeVirtual* node);
		NodeVirtual* getChildAt(const glm::vec3& pos);
		void getChildren(std::vector<NodeVirtual*>& result);
		void getChildren(std::vector<NodeRange>& result);
		void getChildrenInBox(std::vector<NodeVirtual*>& result, const glm::vec3& boxMin, const glm::vec3& boxMax);
		void getChildrenInBox(std::vector<NodeRange>& result, const glm::vec3& boxMin, const glm::vec3& boxMax);
		//

		//	Entities / objects related
		void addObject(Entity* object);
		bool removeObject(Entity* object);
		template<typename ObjectCollector>
		void getObjectList(ObjectCollector& collector);
		template<>
		void getObjectList(std::vector<Entity*>& collector);
		//

		//	Physics engine related
		void addSwept(Entity* object);
		bool removeSwept(Entity* object);
		void clearSwept();
		void getPhysicsArtefactsList(std::vector<PhysicsArtefacts>& collector);
		//

		//	Debug
		Entity* getDebugCube();
		//

		//	Attributes
		//float allowanceSize;
		static World* debugWorld;
		//

	private:
		//	Attributes
		float allowanceSize;
		glm::vec3 position;							//!< Node position in scene coordinate
		glm::vec3 halfSize;							//!< Half of node size
		glm::ivec3 division;

		std::vector<NodeVirtual> children;			//!< Subdivision children container (empty if leaf)
		std::vector<NodeVirtual*> adoptedChildren;	//!< Children added to, for special tree
		std::vector<Entity*> objectList;			//!< Instance container (list of instance attached to node)

		std::vector<Swept> sweptObject;				//!< Physics entities (has to be only used by the physics engine)

		Entity* debugCube;							//!< A 3D cube to represent the node area
		//
};




template<typename ObjectCollector>
void NodeVirtual::getObjectList(ObjectCollector& collector)
{
	//if(debugCube) collector(this, debugCube);
	for(Entity* object : objectList)
		collector(this, object);
}

template<>
void NodeVirtual::getObjectList<std::vector<Entity*>>(std::vector<Entity*>& collector)
{
	collector.insert(collector.end(), objectList.begin(), objectList.end());
}
