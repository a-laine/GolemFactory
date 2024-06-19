#include "NodeVirtual.h"
#include <World/World.h>
#include <Utiles/Assert.hpp>
#include <Utiles/Debug.h>
#include <Utiles/ObjectPool.h>

//#include <glm/gtx/component_wise.hpp>
#include <algorithm>


ObjectPool<NodeVirtual> g_nodePool;


#define ALLOWANCE_SIZE_FACTOR 0.33f

World* NodeVirtual::debugWorld = nullptr;


//	Default
NodeVirtual::NodeVirtual() : position(0.f) , halfSize(0.f), m_parent(nullptr), division(0) , allowanceSize(1.f){}
NodeVirtual::~NodeVirtual()
{
	merge();
}

void NodeVirtual::init(const vec4f bbMin, const vec4f bbMax, const vec3i& nodeDivision)
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
	division = nodeDivision;
}


void NodeVirtual::split()
{
	children.resize((size_t)division.x * division.y * division.z);
	vec4f bbmin = position - halfSize;
	vec4f min;
	vec4f childSize = getSize() / vec4f((vec3f)division, 1.f);
	unsigned int index = 0;
	for (int x = 0; x < division.x; x++)
	{
		min.x = bbmin.x + x * childSize.x;
		for (int y = 0; y < division.y; y++)
		{
			min.y = bbmin.y + y * childSize.y;
			for (int z = 0; z < division.z; z++)
			{
				min.z = bbmin.z + z * childSize.z;
				children[index] = g_nodePool.getFreeObject();
				children[index]->init(min, min + childSize, division);
				children[index]->m_parent = this;
				index++;
			}
		}
	}
}

void NodeVirtual::merge()
{
	for (NodeVirtual* n : children)
	{
		n->merge();
		g_nodePool.releaseObject(n);
		n->m_parent = nullptr;
	}
	children.clear();

	for (unsigned int i = 0; i < objectList.size(); i++)
	{
		World* world = objectList[i]->getParentWorld();
		world->removeFromScene(objectList[i]);
		//world->releaseOwnership(objectList[i]);
		//world->getSceneManager().removeObject(objectList[i]);
	}
	objectList.clear();
}

vec4f NodeVirtual::getCenter() const { return position; }
vec4f NodeVirtual::getSize() const { return halfSize * 2.f; }
vec4f NodeVirtual::getHalfSize() const { return halfSize; }
vec4f NodeVirtual::getInflatedHalfSize() const { return inflatedHalfSize; }
vec4f NodeVirtual::getBBMax() const { return position + halfSize; }
vec4f NodeVirtual::getBBMin() const { return position - halfSize; }
int NodeVirtual::getChildrenCount() const { return (int)(children.size()); }
bool NodeVirtual::isLeaf() const { return children.empty(); }

bool NodeVirtual::isInside(const vec4f& point) const
{
	vec4f p = point - position;
	vec4f s = halfSize;
	return std::abs(p.x) <= s.x && std::abs(p.y) <= s.y && std::abs(p.z) <= s.z;
}

/*bool NodeVirtual::isTooSmall(const vec4f& size) const
{
	return vec4f::dot(size, size) < 4 * allowanceSize * allowanceSize;
}

bool NodeVirtual::isTooBig(const vec4f& size) const
{
	return vec4f::dot(size, size) > allowanceSize * allowanceSize;
}*/
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



NodeVirtual* NodeVirtual::getChildAt(const vec4f& pos)
{
	const vec4f fdiv = vec4f(division.x, division.y, division.z, 1.f);
	const vec4f childSizeInv = fdiv / getSize();
	const vec4i result = (pos - getBBMin()) * childSizeInv;
	int index = division.z * division.y * result.x + division.z * result.y + result.z;
	return children[index];
}

void NodeVirtual::getChildren(std::vector<NodeVirtual*>& result)
{
	for (auto& n : children)
		result.push_back(n);
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
				result.push_back(children[index]);
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
			NodeVirtual* first = children[index + childMin.z];
			NodeVirtual* last  = children[index + childMax.z];
			if(!result.empty() && result.back().end == first)
				result.back().end = last;
			else
				result.push_back(NodeRange(first, last));
		}
	}
}

const std::vector<Entity*>& NodeVirtual::getEntitiesList() const { return objectList; }

void NodeVirtual::draw() const 
{
	const vec4f shrink = vec4f(0.1f);
	Debug::color = objectList.empty() ? Debug::black : Debug::red;
	Debug::drawLineCube(mat4f::identity, getBBMin() + shrink, getBBMax() - shrink);
}
