#include "NodeVirtual.h"
#include <World/World.h>
#include <Utiles/Assert.hpp>
#include <Utiles/Debug.h>

//#include <glm/gtx/component_wise.hpp>
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

void NodeVirtual::init(const vec4f bbMin, const vec4f bbMax, const vec3i& nodeDivision, unsigned int depth)
{
	GF_ASSERT(children.empty());
	position = (bbMax + bbMin) * 0.5f;
	position.w = 1.f;
	vec4f delta = bbMax - bbMin;
	halfSize = delta * 0.5f;
	halfSize.w = 0.f;
	float compMin = std::min(delta.x, std::min(delta.y, delta.z));
	allowanceSize = compMin * ALLOWANCE_SIZE_FACTOR;
	inflatedHalfSize = halfSize + vec4f(allowanceSize);
	inflatedHalfSize.w = 0.f;

	/*if(debugWorld)
		debugCube = debugWorld->getEntityFactory().createObject("cube", position, halfSize, glm::quat(1, 0, 0, 0));*/

	if(depth > 0)
	{
		division = nodeDivision;
		children.resize(nodeDivision.x * nodeDivision.y * nodeDivision.z);
		vec4f min;
		vec4f childSize = getSize() / vec4f((vec3f)nodeDivision, 1.f);
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


vec4f NodeVirtual::getCenter() const { return position; }
vec4f NodeVirtual::getSize() const { return halfSize * 2.f; }
vec4f NodeVirtual::getHalfSize() const { return halfSize; }
vec4f NodeVirtual::getInflatedHalfSize() const { return inflatedHalfSize; }
vec4f NodeVirtual::getBBMax() const { return position + halfSize; }
vec4f NodeVirtual::getBBMin() const { return position - halfSize; }
int NodeVirtual::getChildrenCount() const { return (int)(children.size() + adoptedChildren.size()); }
bool NodeVirtual::isLeaf() const { return children.empty(); }

bool NodeVirtual::isInside(const vec4f& point) const
{
	vec4f p = point - getCenter();
	vec4f s = halfSize;
	return std::abs(p.x) <= s.x && std::abs(p.y) <= s.y && std::abs(p.z) <= s.z;
}

bool NodeVirtual::isTooSmall(const vec4f& size) const
{
	return vec4f::dot(size, size) < 4 * allowanceSize * allowanceSize;
}

bool NodeVirtual::isTooBig(const vec4f& size) const
{
	return vec4f::dot(size, size) > allowanceSize * allowanceSize;
}
vec4f NodeVirtual::getPosition() const
{
	return position;
}
const float& NodeVirtual::getAllowanceSize() const { return allowanceSize; }
vec3i NodeVirtual::getDivision() const { return division; }


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


NodeVirtual* NodeVirtual::getChildAt(const vec4f& pos)
{
	const vec4f fdiv = vec4f(division.x, division.y, division.z, 1.f);
	const vec4f childSizeInv = fdiv / getSize();
	const vec4i result = (pos - getBBMin()) * childSizeInv;
	int index = division.z * division.y * result.x + division.z * result.y + result.z;
	return children.data() + index;
}

void NodeVirtual::getChildren(std::vector<NodeVirtual*>& result)
{
	for (auto& n : children)
		result.push_back(&n);
	//std::transform(children.begin(), children.end(), result.end(), [](NodeVirtual& n) { return &n; });
}

void NodeVirtual::getChildren(std::vector<NodeRange>& result)
{
	result.push_back(NodeRange(children));
}

void NodeVirtual::getChildrenInBox(std::vector<NodeVirtual*>& result, const vec4f& boxMin, const vec4f& boxMax)
{
	const vec4f fdiv = vec4f(division.x, division.y, division.z, 1.f);
	const vec4f childSizeInv = fdiv / getSize();
	vec4i childMin = (boxMin - getBBMin()) * childSizeInv;
	vec4i childMax = (boxMax - getBBMin()) * childSizeInv;
	const vec4i clampMax = vec4i(division.x - 1, division.y - 1, division.z - 1, 1);

	vec4i::clamp(childMin, vec4i::zero, clampMax);
	vec4i::clamp(childMax, vec4i::zero, clampMax);

	for(int x = childMin.x; x < childMax.x; x++)
		for(int y = childMin.y; y < childMax.y; y++)
			for(int z = childMin.z; z < childMax.z; z++)
			{
				unsigned int index = division.z * division.y * x + division.z * y + z;
				result.push_back(children.data() + index);
			}
}

void NodeVirtual::getChildrenInBox(std::vector<NodeRange>& result, const vec4f& boxMin, const vec4f& boxMax)
{
	const vec4f fdiv = vec4f(division.x, division.y, division.z, 1.f);
	const vec4f childSizeInv = fdiv / getSize();
	vec4i childMin = (boxMin - getBBMin()) * childSizeInv;
	vec4i childMax = (boxMax - getBBMin()) * childSizeInv;
	const vec4i clampMax = vec4i(division.x - 1, division.y - 1, division.z - 1, 1);

	vec4i::clamp(childMin, vec4i::zero, clampMax);
	vec4i::clamp(childMax, vec4i::zero, clampMax);

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
	const vec4f shrink = vec4f(0.1f);
	Debug::color = objectList.empty() ? Debug::black : Debug::red;
	Debug::drawLineCube(mat4f::identity, getBBMin() + shrink, getBBMax() - shrink);
}
