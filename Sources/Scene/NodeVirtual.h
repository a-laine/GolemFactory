#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <EntityComponent/Entity.hpp>
#include <Scene/VirtualEntityCollector.h>

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
				explicit NodeRange(std::vector<NodeVirtual>& nodes) : begin(nodes.data()), end(nodes.data() + nodes.size()) {}
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
		void init(const glm::vec4 bbMin, const glm::vec4 bbMax, const glm::ivec3& nodeDivision, unsigned int depth);
		void clearChildren();
		void split(unsigned int newDepth);
		void merge(unsigned int newDepth);
		//

		//	Set / get functions
		bool isLeaf() const;
		glm::vec4 getCenter() const;
		glm::vec3 getSize() const;
		glm::vec4 getBBMax() const;
		glm::vec4 getBBMin() const;
		int getChildrenCount() const;
		bool isInside(const glm::vec4& point) const;
		bool isTooSmall(const glm::vec3& size) const;
		bool isTooBig(const glm::vec3& size) const;
		glm::vec4 getPosition() const;
		const float& getAllowanceSize() const;
		//
		
		//	Hierarchy related function
		void addNode(NodeVirtual* node);
		bool removeNode(NodeVirtual* node);
		NodeVirtual* getChildAt(const glm::vec4& pos);
		void getChildren(std::vector<NodeVirtual*>& result);
		void getChildren(std::vector<NodeRange>& result);
		void getChildrenInBox(std::vector<NodeVirtual*>& result, const glm::vec4& boxMin, const glm::vec4& boxMax);
		void getChildrenInBox(std::vector<NodeRange>& result, const glm::vec4& boxMin, const glm::vec4& boxMax);
		//

		//	Entities / objects related
		void addObject(Entity* object);
		bool removeObject(Entity* object);
		unsigned int  getObjectCount() const;
		const std::vector<Entity*>& getEntitiesList() const;
		//

		//	Debug
		Entity* getDebugCube();
		void draw() const;
		//

		//	Attributes
		static World* debugWorld;
		//

	private:
		//	Attributes
		float allowanceSize;
		glm::vec4 position;							//!< Node position in scene coordinate
		glm::vec4 halfSize;							//!< Half of node size
		glm::ivec3 division;

		std::vector<NodeVirtual> children;			//!< Subdivision children container (empty if leaf)
		std::vector<NodeVirtual*> adoptedChildren;	//!< Children added to, for special tree
		std::vector<Entity*> objectList;			//!< Instance container (list of instance attached to node)

		//std::vector<Swept*> sweptObject;			//!< Physics entities (has to be only used by the physics engine)

		Entity* debugCube;							//!< A 3D cube to represent the node area
		//
};


