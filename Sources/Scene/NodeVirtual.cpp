#include "NodeVirtual.h"

//	static attributes initialization
glm::vec3 NodeVirtual::camPosition = glm::vec3(0, 0, 0);
glm::vec3 NodeVirtual::camDirection = glm::vec3(1, 0, 0);
glm::vec3 NodeVirtual::camVertical = glm::vec3(0, 0, 1);
glm::vec3 NodeVirtual::camLeft = glm::vec3(0, 1, 0);
float NodeVirtual::camVerticalAngle = 45;
float NodeVirtual::camHorizontalAngle = 45;
//


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
	for (unsigned int i = 0; i < instanceList.size(); i++)
		InstanceManager::getInstance()->release(instanceList[i]);
	instanceList.clear();
	InstanceManager::getInstance()->release(debuginstance);

	merge();
}
//

//
void NodeVirtual::print(int lvl)
{
	//	check if node intersect frustrum
	int distance = isInFrustrum();
	if (distance == std::numeric_limits<int>::lowest()) return;

	//	dummy debug
	for (int i = 0; i < lvl; i++)
		std::cout << "  ";
	std::cout <<  this << ' ' << instanceList.size() << std::endl;

	for (unsigned int i = 0; i < children.size(); i++)
		children[i]->print(lvl + 1);
}
//

//	Public functions
int NodeVirtual::getNbFils(int level) { return children.size(); }
bool NodeVirtual::isLastBranch()
{
	if (!children.empty() && children[0]->children.empty()) return true;
	return false;
}

bool NodeVirtual::addObject(InstanceVirtual* obj)
{
	if (!obj) return false;
	glm::vec3 s = glm::vec3(obj->getSize().x*obj->getBBSize().x, obj->getSize().y*obj->getBBSize().y, obj->getSize().z*obj->getBBSize().z);
	if (s.x < 0.1*size.x && s.y < 0.1*size.y && s.z < 0.1*size.z && !children.empty())
	{
		int key = getChildrenKey(obj->getPosition());
		if (key < 0) return false;
		else return children[key]->addObject(obj);
	}
	else
	{
		instanceList.push_back(obj);
		return true;
	}
	instanceList.push_back(obj);
	return true;
}
bool NodeVirtual::removeObject(InstanceVirtual* obj)
{
	if (obj) 
	{
		auto it = std::find(instanceList.begin(), instanceList.end(), obj);
		if (it != instanceList.end()) instanceList.erase(it);
		else return false;
	}
	return true;
}
void NodeVirtual::getInstanceList(std::vector<std::pair<int, InstanceVirtual*> >& list)
{
	//	check if not in frustrum
	int distance = isInFrustrum();
	if (distance == std::numeric_limits<int>::lowest()) return;

	/*
	debuginstance->setPosition(position);
	debuginstance->setSize(0.4f*size);

	if (children.empty())
		list.push_back(std::pair<int, InstanceVirtual*>(distance, debuginstance));
	else
	{
		for (unsigned int i = 0; i < children.size(); i++)
			children[i]->getInstanceList(list);
	}
	return;
	*/

	//	give personnal instance and continue with recursive call on children
	for (unsigned int i = 0; i < instanceList.size(); i++)
		list.push_back(std::pair<int, InstanceVirtual*>(distance,instanceList[i]));

	for (unsigned int i = 0; i < children.size(); i++)
		children[i]->getInstanceList(list);
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
//

//Set/Get functions
void NodeVirtual::setPosition(glm::vec3 p)
{
	for (unsigned int i = 0; i < children.size(); i++)
		children[i]->setPosition(children[i]->position - position + p);
	position = p;
}
void NodeVirtual::setPosition(float x, float y, float z) { setPosition(glm::vec3(x,y,z)); }
void NodeVirtual::setSize(glm::vec3 s)
{
	size = s;

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
void NodeVirtual::setSize(float x, float y, float z) { setSize(glm::vec3(x, y, z)); }

glm::vec3 NodeVirtual::getPosition() { return position; }
glm::vec3 NodeVirtual::getSize() { return size; }
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
int NodeVirtual::isInFrustrum() const
{
	glm::vec3 p = position - camPosition;
	float forwardFloat = glm::dot(p, camDirection) + std::abs(size.x*camDirection.x) + abs(size.y*camDirection.y) + abs(size.z*camDirection.z);
	if (forwardFloat < 0) return std::numeric_limits<int>::lowest();

	float maxAbsoluteDimension = (std::max)(size.x, (std::max)(size.y, size.z)) / 2;
	float maxTangentDimension = std::abs(size.x*camLeft.x) / 2 + abs(size.y*camLeft.y) / 2 + abs(size.z*camLeft.z) / 2;
	if (std::abs(glm::dot(p, camLeft)) - maxTangentDimension > std::abs(forwardFloat)*std::tan(glm::radians(camHorizontalAngle)) + maxAbsoluteDimension) return std::numeric_limits<int>::lowest();

	maxTangentDimension = std::abs(size.x*camVertical.x) / 2 + abs(size.y*camVertical.y) / 2 + abs(size.z*camVertical.z) / 2;
	if (std::abs(glm::dot(p, camVertical)) - maxTangentDimension > std::abs(forwardFloat)*std::tan(glm::radians(camVerticalAngle)) + maxAbsoluteDimension) return std::numeric_limits<int>::lowest();

	return glm::length(p);
}
//
