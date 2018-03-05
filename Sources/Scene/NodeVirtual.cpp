#include "NodeVirtual.h"

#include <glm/gtx/component_wise.hpp>

#include <World/World.h>
#include <Utiles/Assert.hpp>



//	Default
NodeVirtual::NodeVirtual()
	: position(0)
	, halfSize(0)
	, division(0)
	, allowanceSize(1)
{
}

NodeVirtual::~NodeVirtual()
{
	//	free instance
	for (unsigned int i = 0; i < objectList.size(); i++)
	{
		World* world = objectList[i]->getParentWorld();
		world->releaseObject(objectList[i]);
	}

	//	delete all children
	for (unsigned int i = 0; i < adoptedChildren.size(); i++)
		delete adoptedChildren[i];
}


void NodeVirtual::init(const glm::vec3 bbMin, const glm::vec3 bbMax, const glm::ivec3& nodeDivision, unsigned int depth)
{
	GF_ASSERT(children.empty());
	position = (bbMax + bbMin) * 0.5f;
	halfSize = (bbMax - bbMin) * 0.5f;
	allowanceSize = glm::compMin(halfSize) * 2;

	if(depth > 0)
	{
		division = nodeDivision;
		children.resize(nodeDivision.x * nodeDivision.y * nodeDivision.z);
		glm::vec3 min;
		glm::vec3 childSize = getSize() / glm::vec3(nodeDivision);
		unsigned int index = 0;
		for(int x = 0; x < nodeDivision.x; x++)
		{
			min.x = bbMin.x + x * childSize.x;
			for(int y = 0; y < nodeDivision.y; y++)
			{
				min.y = bbMin.y + y * childSize.y;
				for(int z = 0; z < nodeDivision.z; z++)
				{
					min.z = bbMin.z + z * childSize.z;
					children[index].init(min, min + childSize, nodeDivision, depth - 1);
					index++;
				}
			}
		}
	}
}

void NodeVirtual::clearChildren()
{
	children.clear();
}

void NodeVirtual::split(unsigned int newDepth)
{
	if(newDepth > 0)
	{
		if(children.empty())
		{
			init(getBBMin(), getBBMax(), division, newDepth);
		}
		else
		{
			for(NodeVirtual& node : children)
				node.split(newDepth - 1);
		}
	}
}

void NodeVirtual::merge(unsigned int newDepth)
{
	if(newDepth > 0)
	{
		for(NodeVirtual& node : children)
			node.merge(newDepth - 1);
	}
	else
	{
		clearChildren();
	}
}


glm::vec3 NodeVirtual::getCenter() const { return position; }
glm::vec3 NodeVirtual::getSize() const { return halfSize * 2.f; }
glm::vec3 NodeVirtual::getBBMax() const { return position + halfSize; }
glm::vec3 NodeVirtual::getBBMin() const { return position - halfSize; }
int NodeVirtual::getChildrenCount() const { return children.size() + adoptedChildren.size(); }
bool NodeVirtual::isLeaf() const { return children.empty(); }

bool NodeVirtual::isInside(const glm::vec3& point) const
{
	glm::vec3 p = point - getCenter();
	glm::vec3 s = getSize() * 0.5f;
	return glm::abs(p.x) <= s.x && glm::abs(p.y) <= s.y && glm::abs(p.z) <= s.z;
}

bool NodeVirtual::isTooSmall(const glm::vec3& size) const
{
	return glm::dot(size, size) < allowanceSize * allowanceSize;
}

bool NodeVirtual::isTooBig(const glm::vec3& size) const
{
	return glm::dot(size, size) > 4 * 4 * allowanceSize * allowanceSize;
}


void NodeVirtual::addObject(InstanceVirtual* object)
{
	objectList.push_back(object);
	object->getParentWorld()->getObject(object);
}

bool NodeVirtual::removeObject(InstanceVirtual* object)
{
	auto it = std::find(objectList.begin(), objectList.end(), object);
	if(it != objectList.end())
	{
		if(it + 1 != objectList.end())
			std::swap(*it, objectList.back());
		objectList.pop_back();
		object->getParentWorld()->releaseObject(object);
		return true;
	}
	return false;
}

void NodeVirtual::addNode(NodeVirtual* node)
{
	adoptedChildren.push_back(node);
}

bool NodeVirtual::removeNode(NodeVirtual* node)
{
	auto it = std::find(adoptedChildren.begin(), adoptedChildren.end(), node);
	if(it != adoptedChildren.end())
	{
		adoptedChildren.erase(it);
		return true;
	}
	else return false;
}


NodeVirtual* NodeVirtual::getChildAt(const glm::vec3& pos)
{
	const glm::vec3 childSizeInv = glm::vec3(division) / getSize();
	const glm::ivec3 result = (pos - getBBMin()) * childSizeInv;
	int index = division.z * division.y * result.x + division.z * result.y + result.z;
	return children.data() + index;
}

void NodeVirtual::getChildren(std::vector<NodeVirtual*>& result)
{
	for(NodeVirtual& node : children)
		result.push_back(&node);
}

void NodeVirtual::getChildren(std::vector<NodeRange>& result)
{
	result.push_back(NodeRange(children));
}

void NodeVirtual::getChildrenInBox(std::vector<NodeVirtual*>& result, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	const glm::vec3 childSizeInv = glm::vec3(division) / getSize();
	glm::ivec3 childMin = (boxMin - getBBMin()) * childSizeInv;
	glm::ivec3 childMax = (boxMax - getBBMin()) * childSizeInv;

	const glm::ivec3 clampMin = glm::ivec3(0);
	const glm::ivec3 clampMax = division - glm::ivec3(1);
	glm::clamp(childMin, clampMin, clampMax);
	glm::clamp(childMax, clampMin, clampMax);

	for(int x = childMin.x; x < childMax.x; x++)
		for(int y = childMin.y; y < childMax.y; y++)
			for(int z = childMin.z; z < childMax.z; z++)
			{
				unsigned int index = division.z * division.y * x + division.z * y + z;
				result.push_back(children.data() + index);
			}
}

void NodeVirtual::getChildrenInBox(std::vector<NodeRange>& result, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	const glm::vec3 childSizeInv = glm::vec3(division) / getSize();
	glm::ivec3 childMin = (boxMin - getBBMin()) * childSizeInv;
	glm::ivec3 childMax = (boxMax - getBBMin()) * childSizeInv;

	const glm::ivec3 clampMin = glm::ivec3(0);
	const glm::ivec3 clampMax = division - glm::ivec3(1);
	glm::clamp(childMin, clampMin, clampMax);
	glm::clamp(childMax, clampMin, clampMax);

	for(int x = childMin.x; x < childMax.x; x++)
	{
		for(int y = childMin.y; y < childMax.y; y++)
		{
			int index = division.z * division.y * x + division.z * y;
			NodeVirtual* first = children.data() + (index + childMin.z);
			NodeVirtual* last  = children.data() + (index + childMax.z);
			if(!result.empty() && result.back().end == first)
				result.back().end = last;
			else
				result.push_back(NodeRange(first, last));
		}
	}
}


