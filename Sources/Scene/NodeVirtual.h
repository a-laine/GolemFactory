#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Instances/InstanceManager.h"

/*! \class NodeVirtual
 *  \brief Base class for node implementation.
 *
 *	 A node can be represented by a cube container.
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

		NodeVirtual();
		NodeVirtual(const NodeVirtual& other) = delete;
		NodeVirtual(NodeVirtual&& other) = default;
		virtual ~NodeVirtual();

		void init(const glm::vec3 bbMin, const glm::vec3 bbMax, const glm::ivec3& nodeDivision, unsigned int depth);
		void clearChildren();
		void split(unsigned int newDepth);
		void merge(unsigned int newDepth);

		bool isLeaf() const;
		glm::vec3 getCenter() const;
		glm::vec3 getSize() const;
		glm::vec3 getBBMax() const;
		glm::vec3 getBBMin() const;
		int getChildrenCount() const;
		bool isInside(const glm::vec3& point) const;
		bool isTooSmall(const glm::vec3& size) const;
		bool isTooBig(const glm::vec3& size) const;
		
		void addObject(InstanceVirtual* object);
		bool removeObject(InstanceVirtual* object);
		void addNode(NodeVirtual* node);
		bool removeNode(NodeVirtual* node);

		NodeVirtual* getChildAt(const glm::vec3& pos);
		void getChildren(std::vector<NodeVirtual*>& result);
		void getChildren(std::vector<NodeRange>& result);
		void getChildrenInBox(std::vector<NodeVirtual*>& result, const glm::vec3& boxMin, const glm::vec3& boxMax);
		void getChildrenInBox(std::vector<NodeRange>& result, const glm::vec3& boxMin, const glm::vec3& boxMax);

		template<typename ObjectCollector>
		void getObjectList(ObjectCollector& collector);
		template<>
		void getObjectList<std::vector<InstanceVirtual*>>(std::vector<InstanceVirtual*>& collector);

	public:
		float allowanceSize;

	private:
		glm::vec3 position;							//!< Node position in scene coordinate
		glm::vec3 halfSize;							//!< Half of node size
		glm::ivec3 division;

		std::vector<NodeVirtual> children;			//!< Subdivision children container (empty if leaf)
		std::vector<NodeVirtual*> adoptedChildren;	//!< Children added to, for special tree
		std::vector<InstanceVirtual*> objectList;	//!< Instance container (list of instance attached to node)
};




template<typename ObjectCollector>
void NodeVirtual::getObjectList(ObjectCollector& collector)
{
	for(InstanceVirtual* object : objectList)
		collector(this, object);
}

template<>
void NodeVirtual::getObjectList<std::vector<InstanceVirtual*>>(std::vector<InstanceVirtual*>& collector)
{
	collector.insert(collector.end(), objectList.begin(), objectList.end());
}

