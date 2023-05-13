#pragma once

#include <GL/glew.h>
//#include <glm/glm.hpp>

#include "Math/TMath.h"
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
	friend class SceneManager;
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
		void init(const vec4f bbMin, const vec4f bbMax, const vec3i& nodeDivision, unsigned int depth);
		void clearChildren();
		void split(unsigned int newDepth);
		void merge(unsigned int newDepth);
		//

		//	Set / get functions
		bool isLeaf() const;
		vec4f getCenter() const;
		vec4f getSize() const;
		vec4f getHalfSize() const;
		vec4f getInflatedHalfSize() const;
		vec4f getBBMax() const;
		vec4f getBBMin() const;
		int getChildrenCount() const;
		bool isInside(const vec4f& point) const;
		bool isTooSmall(const vec4f& size) const;
		bool isTooBig(const vec4f& size) const;
		vec4f getPosition() const;
		const float& getAllowanceSize() const;
		vec3i getDivision() const;
		//
		
		//	Hierarchy related function
		void addNode(NodeVirtual* node);
		bool removeNode(NodeVirtual* node);
		NodeVirtual* getChildAt(const vec4f& pos);
		void getChildren(std::vector<NodeVirtual*>& result);
		void getChildren(std::vector<NodeRange>& result);
		void getChildrenInBox(std::vector<NodeVirtual*>& result, const vec4f& boxMin, const vec4f& boxMax);
		void getChildrenInBox(std::vector<NodeRange>& result, const vec4f& boxMin, const vec4f& boxMax);
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
		vec4f position;							//!< Node position in scene coordinate
		vec4f halfSize;							//!< Half of node size
		vec4f inflatedHalfSize;
		vec3i division;

		std::vector<NodeVirtual> children;			//!< Subdivision children container (empty if leaf)
		std::vector<NodeVirtual*> adoptedChildren;	//!< Children added to, for special tree
		std::vector<Entity*> objectList;			//!< Instance container (list of instance attached to node)

		//std::vector<Swept*> sweptObject;			//!< Physics entities (has to be only used by the physics engine)

		Entity* debugCube;							//!< A 3D cube to represent the node area
		//
};


