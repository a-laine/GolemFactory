#include "NodeVirtual.h"

#define FRUSTRUM_COEFF			2.f	//	coefficient for frustrum intersection computation (to avoid artefacts)
#define RAY_COEFF				0.2f
#define SIZE_ALLOWANCE_COEFF	0.1f


//	Default
NodeVirtual::NodeVirtual(NodeVirtual* p, unsigned int d) : parent(p),debuginstance(nullptr)
{
	if (!(d&X)) d |= 0x01 << dX;
	if (!(d&Y)) d |= 0x01 << dY;
	if (!(d&Z)) d |= 0x01 << dZ;
	division = d;

	position = glm::vec3(0, 0, 0);
	size = glm::vec3(1, 1, 1);

	debuginstance = InstanceManager::getInstance()->getInstanceDrawable("cube2.obj");
}
NodeVirtual::~NodeVirtual()
{
	//	free instance
	for (unsigned int i = 0; i < instanceList.size(); i++)
		InstanceManager::getInstance()->release(instanceList[i]);
	instanceList.clear();
	InstanceManager::getInstance()->release(debuginstance);

	//	delete all children
	merge();
	for (unsigned int i = 0; i < adoptedChildren.size(); i++)
		delete adoptedChildren[i];
}
//


//	Public functions
int NodeVirtual::getChildrenCount() const { return children.size() + adoptedChildren.size(); }
bool NodeVirtual::isLastBranch() const
{
	if (!children.empty() && children[0]->children.empty()) return true;
	return false;
}
bool NodeVirtual::addObject(InstanceVirtual* obj)
{
	if (!obj) return false;
	if (glm::length(obj->getBBMax() - obj->getBBMin()) < SIZE_ALLOWANCE_COEFF * glm::length(size) && !children.empty())
	{
		int key = getChildrenKey(obj->getPosition());
		if (key < 0) return false;
		else return children[key]->addObject(obj);
	}
	else
	{
		obj->count++;
		instanceList.push_back(obj);
		return true;
	}
}
bool NodeVirtual::removeObject(InstanceVirtual* obj)
{
	if (!obj) return false;
	if (glm::length(obj->getBBMax() - obj->getBBMin()) < 0.1f * glm::length(size) && !children.empty())
	{
		int key = getChildrenKey(obj->getPosition());
		if (key < 0) return false;
		else return children[key]->removeObject(obj);
	}
	else
	{
		auto it = std::find(instanceList.begin(), instanceList.end(), obj);
		if (it != instanceList.end()) 
		{
			InstanceManager::getInstance()->release(*it);
			instanceList.erase(it);
			return true;
		}
		else return false;
	}
}


void NodeVirtual::split()
{
	if (!children.empty())return;
	int xChild = (division&X) >> dX;
	int yChild = (division&Y) >> dY;
	int zChild = (division&Z) >> dZ;

	for (int i = 0; i < xChild; i++)
	{
		float xpos = position.x - size.x / 2 + size.x / xChild / 2 + i*size.x / xChild;
		for (int j = 0; j < yChild; j++)
		{
			float ypos = position.y - size.y / 2 + size.y / yChild / 2 + j*size.y / yChild;
			for (int k = 0; k < zChild; k++)
			{
				float zpos = position.z - size.z / 2 + size.z / zChild / 2 + k*size.z / zChild;
				NodeVirtual* n = new NodeVirtual(this,division);
					n->setPosition(glm::vec3(xpos,ypos,zpos));
					n->setSize(glm::vec3(size.x / xChild, size.y / yChild, size.z / zChild));
					children.push_back(n);
			}
		}
	}
}
void NodeVirtual::merge()
{
	for (unsigned int i = 0; i < children.size(); i++)
		delete children[i];
	children.clear();
}
void NodeVirtual::add(NodeVirtual* n)
{
	adoptedChildren.push_back(n);
	n->parent = this;
}
bool NodeVirtual::remove(NodeVirtual* n)
{
	std::vector<NodeVirtual*>::iterator it = std::find(adoptedChildren.begin(), adoptedChildren.end(), n);
	if (it != adoptedChildren.end())
	{
		adoptedChildren.erase(it);
		n->parent = nullptr;
		return true;
	}
	else return false;
}
//


//Set/Get functions
void NodeVirtual::setPosition(glm::vec3 p)
{
	for (unsigned int i = 0; i < children.size(); i++)
		children[i]->setPosition(children[i]->position - position + p);
	position = p;
	debuginstance->setPosition(glm::vec3(p.x, p.y, getLevel()));
}
void NodeVirtual::setSize(glm::vec3 s)
{
	size = s;
	debuginstance->setSize(glm::vec3(0.4*s.x, 0.4*s.y, 0.2));

	if (children.empty()) return;
	int xChild = (division&X) >> dX;
	int yChild = (division&Y) >> dY;
	int zChild = (division&Z) >> dZ;

	for (int i = 0; i < xChild; i++)
	{
		float xpos = position.x - size.x / 2 + size.x / xChild / 2 + i*size.x / xChild;
		for (int j = 0; j < yChild; j++)
		{
			float ypos = position.y - size.y / 2 + size.y / yChild / 2 + j*size.y / yChild;
			for (int k = 0; k < zChild; k++)
			{
				float zpos = position.z - size.z / 2 + size.z / zChild / 2 + k*size.z / zChild;
				int index = zChild*yChild*i + zChild*j + k;
				children[index]->setPosition(glm::vec3(xpos, ypos, zpos));
				children[index]->setSize(glm::vec3(size.x / xChild, size.y / yChild, size.z / zChild));
			}
		}
	}
}


glm::vec3 NodeVirtual::getPosition() const { return position; }
glm::vec3 NodeVirtual::getSize() const { return size; }
//


//	Protected functions
uint8_t NodeVirtual::getLevel() const
{
	if (parent) return parent->getLevel() + 1;
	else return 0;
}
int NodeVirtual::getChildrenKey(glm::vec3 p) const
{
	if (children.empty())
		return -1;

	int xChild = (division&X) >> dX;
	int yChild = (division&Y) >> dY;
	int zChild = (division&Z) >> dZ;

	int x, y, z;
	p -= position;
	p += 0.5f*size;

	x = (int)(p.x / size.x * xChild);
	if (x >= xChild)
		return -1;
	
	y = (int)(p.y / size.y * yChild);
	if (y >= yChild)
		return -1;

	z = (int)(p.z / size.z * zChild);
	if (z >= zChild)
		return -1;

	return zChild*yChild*x + zChild*y + z;
}
int NodeVirtual::isInFrustrum(const glm::vec3& camP, const glm::vec3& camD, const glm::vec3& camV, const glm::vec3& camL, const float& camVa, const float& camHa) const
{
	//	test if in front of camera
	glm::vec3 p = position - camP;
	float forwardFloat = glm::dot(p, camD) + FRUSTRUM_COEFF * (abs(size.x * camD.x) + abs(size.y * camD.y) + abs(size.z * camD.z));
	if (forwardFloat < 0.f)
		return std::numeric_limits<int>::lowest();

	//	out of horizontal range
	float maxAbsoluteDimension = (std::max)(size.x, (std::max)(size.y, size.z)) / 2.f;
	float maxTangentDimension = abs(size.x * camL.x) / 2.f + abs(size.y * camL.y) / 2.f + abs(size.z * camL.z) / 2.f;
	if (abs(glm::dot(p, camL)) - maxTangentDimension > std::abs(forwardFloat) * tan(glm::radians(camHa)) + FRUSTRUM_COEFF * maxAbsoluteDimension)
		return std::numeric_limits<int>::lowest();

	//	out of vertical range
	maxTangentDimension = abs(size.x * camV.x) / 2.f + abs(size.y * camV.y) / 2.f + abs(size.z * camV.z) / 2.f;
	if (abs(glm::dot(p, camV)) - maxTangentDimension > abs(forwardFloat) * tan(glm::radians(camVa)) + FRUSTRUM_COEFF * maxAbsoluteDimension)
		return std::numeric_limits<int>::lowest();

	//	return distance to camera in int
	return (int)glm::length(p);
}
int NodeVirtual::isOnRay(const glm::vec3& origin, const glm::vec3& ray, const glm::vec3& rayV, const glm::vec3& rayL) const
{
	//	test if in front of camera
	glm::vec3 p = position - origin;
	if (glm::dot(p, ray) + 1.f * (abs(size.x * ray.x) + abs(size.y * ray.y) + abs(size.z * ray.z)) < 0.f)
		return std::numeric_limits<int>::lowest();

	//	out of horizontal range
	float maxAbsoluteDimension = std::max(size.x, std::max(size.y, size.z)) / 2.f;
	float maxTangentDimension = abs(size.x * rayL.x) / 2.f + abs(size.y * rayL.y) / 2.f + abs(size.z * rayL.z) / 2.f;
	if (abs(glm::dot(p, rayL)) - maxTangentDimension > RAY_COEFF * maxAbsoluteDimension)
		return std::numeric_limits<int>::lowest();

	//	out of vertical range
	maxTangentDimension = abs(size.x * rayV.x) / 2.f + abs(size.y * rayV.y) / 2.f + abs(size.z * rayV.z) / 2.f;
	if (abs(glm::dot(p, rayV)) - maxTangentDimension > RAY_COEFF * maxAbsoluteDimension)
		return std::numeric_limits<int>::lowest();

	//	return distance to camera in int
	return (int)glm::length(p);
}
//
