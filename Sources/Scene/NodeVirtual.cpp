#include "NodeVirtual.h"
#include <World/World.h>
#include <Utiles/Assert.hpp>
#include <Utiles/Debug.h>

#include <glm/gtx/component_wise.hpp>
#include <algorithm>

#define ALLOWANCE_SIZE_FACTOR 0.33f

World* NodeVirtual::debugWorld = nullptr;


//	Default
NodeVirtual::NodeVirtual() : position(0.f) , halfSize(0.f) , division(0) , allowanceSize(1.f), debugCube(nullptr){}
NodeVirtual::~NodeVirtual()
{
	//	free instance
	for (unsigned int i = 0; i < objectList.size(); i++)
	{
		World* world = objectList[i]->getParentWorld();
		world->releaseOwnership(objectList[i]);
	}
	/*if(debugWorld && debugCube)
		debugWorld->releaseOwnership(debugCube);*/

	//	delete all children
	for (unsigned int i = 0; i < adoptedChildren.size(); i++)
		delete adoptedChildren[i];
}

void NodeVirtual::init(const glm::vec4 bbMin, const glm::vec4 bbMax, const glm::ivec3& nodeDivision, unsigned int depth)
{
	GF_ASSERT(children.empty());
	position = (bbMax + bbMin) * 0.5f;
	halfSize = (bbMax - bbMin) * 0.5f;
	allowanceSize = glm::compMin(bbMax - bbMin) * ALLOWANCE_SIZE_FACTOR;

	/*if(debugWorld)
		debugCube = debugWorld->getEntityFactory().createObject("cube", position, halfSize, glm::quat(1, 0, 0, 0));*/

	if(depth > 0)
	{
		division = nodeDivision;
		children.resize(nodeDivision.x * nodeDivision.y * nodeDivision.z);
		glm::vec4 min;
		glm::vec4 childSize = glm::vec4(getSize() / glm::vec3(nodeDivision), 0.f);
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


glm::vec4 NodeVirtual::getCenter() const { return position; }
glm::vec3 NodeVirtual::getSize() const { return (glm::vec3)halfSize * 2.f; }
glm::vec4 NodeVirtual::getBBMax() const { return position + halfSize; }
glm::vec4 NodeVirtual::getBBMin() const { return position - halfSize; }
int NodeVirtual::getChildrenCount() const { return (int)(children.size() + adoptedChildren.size()); }
bool NodeVirtual::isLeaf() const { return children.empty(); }

bool NodeVirtual::isInside(const glm::vec4& point) const
{
	glm::vec4 p = point - getCenter();
	glm::vec4 s = halfSize;
	return glm::abs(p.x) <= s.x && glm::abs(p.y) <= s.y && glm::abs(p.z) <= s.z;
}

bool NodeVirtual::isTooSmall(const glm::vec3& size) const
{
	return glm::dot(size, size) < allowanceSize * allowanceSize;
}

bool NodeVirtual::isTooBig(const glm::vec3& size) const
{
	return glm::dot(size, size) > 1.0f / ALLOWANCE_SIZE_FACTOR * allowanceSize * allowanceSize;
}
glm::vec4 NodeVirtual::getPosition() const
{
	return position;
}
const float& NodeVirtual::getAllowanceSize() const { return allowanceSize; }


void NodeVirtual::addObject(Entity* object)
{
	objectList.push_back(object);
	object->getParentWorld()->getOwnership(object);
}
bool NodeVirtual::removeObject(Entity* object)
{
	auto it = std::find(objectList.begin(), objectList.end(), object);
	if(it != objectList.end())
	{
		if(it + 1 != objectList.end())
			std::swap(*it, objectList.back());
		objectList.pop_back();
		object->getParentWorld()->releaseOwnership(object);
		return true;
	}
	return false;
}
unsigned int  NodeVirtual::getObjectCount() const
{
	return (unsigned int)objectList.size();
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


NodeVirtual* NodeVirtual::getChildAt(const glm::vec4& pos)
{
	const glm::vec3 childSizeInv = glm::vec3(division) / getSize();
	const glm::ivec3 result = (glm::vec3)(pos - getBBMin()) * childSizeInv;
	int index = division.z * division.y * result.x + division.z * result.y + result.z;
	return children.data() + index;
}

void NodeVirtual::getChildren(std::vector<NodeVirtual*>& result)
{
	std::transform(children.begin(), children.end(), result.end(), [](NodeVirtual& n) { return &n; });
}

void NodeVirtual::getChildren(std::vector<NodeRange>& result)
{
	result.push_back(NodeRange(children));
}

void NodeVirtual::getChildrenInBox(std::vector<NodeVirtual*>& result, const glm::vec4& boxMin, const glm::vec4& boxMax)
{
	const glm::vec3 childSizeInv = glm::vec3(division) / getSize();
	glm::ivec3 childMin = (glm::vec3)(boxMin - getBBMin()) * childSizeInv;
	glm::ivec3 childMax = (glm::vec3)(boxMax - getBBMin()) * childSizeInv;

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

void NodeVirtual::getChildrenInBox(std::vector<NodeRange>& result, const glm::vec4& boxMin, const glm::vec4& boxMax)
{
	const glm::vec3 childSizeInv = glm::vec3(division) / getSize();
	glm::ivec3 childMin = (glm::vec3)(boxMin - getBBMin()) * childSizeInv;
	glm::ivec3 childMax = (glm::vec3)(boxMax - getBBMin()) * childSizeInv;

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


/*NodeVirtual* NodeVirtual::addSwept(Swept* object)
{
	sweptObject.push_back(object);
	return this;
}
bool NodeVirtual::removeSwept(Swept* object)
{
	std::remove(sweptObject.begin(), sweptObject.end(), object);
}
void NodeVirtual::clearSwept()
{
	sweptObject.clear();
}
void NodeVirtual::getPhysicsArtefactsList(std::vector<PhysicsArtefacts>& collector)
{
	for (auto it = objectList.begin(); it != objectList.end(); ++it)
		collector.insert(collector.end(), PhysicsArtefacts(*it));
	for (auto it = sweptObject.begin(); it != sweptObject.end(); ++it)
		collector.insert(collector.end(), PhysicsArtefacts(*it));
}*/

const std::vector<Entity*>& NodeVirtual::getEntitiesList() const { return objectList; }


Entity* NodeVirtual::getDebugCube() { return debugCube; }

void NodeVirtual::draw() const 
{
	Debug::getInstance()->color = objectList.empty() ? Debug::black : Debug::red;
	Debug::getInstance()->drawWiredCube(glm::mat4(1.f), (glm::vec3)getBBMin(), (glm::vec3)getBBMax());
}
