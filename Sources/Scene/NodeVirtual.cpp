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
NodeVirtual::NodeVirtual(NodeVirtual* p, unsigned int d) : parent(p)
{
	if (!(d&X)) d |= 0x01 << dX;
	if (!(d&Y)) d |= 0x01 << dY;
	if (!(d&Z)) d |= 0x01 << dZ;
	division = d;

	position = glm::vec3(0, 0, 0);
	size = glm::vec3(1, 1, 1);
}
NodeVirtual::~NodeVirtual()
{
	for (unsigned int i = 0; i < instanceList.size(); i++)
		InstanceManager::getInstance()->release(instanceList[i]);
	instanceList.clear();

	merge();
}
//

//
void NodeVirtual::print(int lvl)
{
	//	check if node intersect frustrum
	glm::vec3 p = position - camPosition;
	float forwardFloat = glm::dot(p, camDirection) + std::abs(size.x*camDirection.x)/2 + abs(size.y*camDirection.y)/2 + abs(size.z*camDirection.z)/2;
	if (forwardFloat < 0) return;
	
	float maxAbsoluteDimension = 0;// (std::max)(size.x, (std::max)(size.y, size.z)) / 2;
	float maxTangentDimension = std::abs(size.x*camLeft.x)/2 + abs(size.y*camLeft.y)/2 + abs(size.z*camLeft.z)/2;
	if (glm::dot(p, camLeft) - maxTangentDimension > std::abs(forwardFloat)*std::tan(glm::radians(camHorizontalAngle)) + maxAbsoluteDimension) return;

	maxTangentDimension = std::abs(size.x*camVertical.x)/2 + abs(size.y*camVertical.y)/2 + abs(size.z*camVertical.z)/2;
	if (glm::dot(p, camVertical) - maxTangentDimension > std::abs(forwardFloat)*std::tan(glm::radians(camVerticalAngle)) + maxAbsoluteDimension) return;

	//	dummy debug
	for (int i = 0; i < lvl; i++)
		std::cout << "  ";
	std::cout << (unsigned int) this << std::endl;

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
	glm::vec3 s = obj->getSize();
	if (s.x < 0.1*size.x && s.y < 0.1*size.y && s.z < 0.1*size.z)
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
	glm::vec3 p = camPosition - position;
	if (glm::dot(p, camDirection) < 0) return;
	if (std::abs(glm::dot(p, camLeft)) < std::tan(glm::radians(camHorizontalAngle))) return;
	if (std::abs(glm::dot(p, camVertical)) < std::tan(glm::radians(camVerticalAngle))) return;
	
	//	give personnal instance and continue with recursive call on children
	for (unsigned int i = 0; i < instanceList.size(); i++)
		list.push_back(std::pair<int, InstanceVirtual*>(glm::length(p),instanceList[i]));

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
		children[i]->setPosition(children[i]->getPosition() - position + p);
	position = p;
}
void NodeVirtual::setPosition(float x, float y, float z) { setPosition(glm::vec3(x,y,z)); }
void NodeVirtual::setSize(glm::vec3 s)
{
	size = s;
	uint8_t xChild = (division&X) >> dX;
	uint8_t yChild = (division&Y) >> dY;
	uint8_t zChild = (division&Z) >> dZ;

	for (unsigned int i = 0; i < children.size(); i++)
	{
		uint8_t ix = i%xChild;
		uint8_t iy = (i / xChild) % yChild;
		uint8_t iz = (i / (xChild*yChild)) % yChild;

		glm::vec3 v((2 * ix - xChild + 1)*size.x / xChild,
					(2 * iy - yChild + 1)*size.y / yChild,
					(2 * iz - zChild + 1)*size.z / zChild );

		children[i]->setSize(s.x/xChild, s.y / yChild, s.z / zChild);
		children[i]->setPosition(position + v);
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

	unsigned int xChild = (division&X) >> dX;
	unsigned int yChild = (division&Y) >> dY;
	unsigned int zChild = (division&Z) >> dZ;

	unsigned int x, y, z;   p -= position;
	x = (unsigned int)((p.x / 2 / size.x + 0.5)*xChild);
	if (x >= xChild)
		return -1;

	y = (unsigned int)((p.y / 2 / size.y + 0.5)*yChild);
	if (y >= yChild)
		return -1;

	z = (unsigned int)((p.z / 2 / size.z + 0.5)*zChild);
	if (z >= zChild)
		return -1;

	return zChild*yChild*x + zChild*y + z;
}
//
