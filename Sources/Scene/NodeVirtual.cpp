#include "NodeVirtual.h"

//	Default
NodeVirtual::NodeVirtual(NodeVirtual* p, unsigned int d) : parent(p),division(d)
{
	position = glm::vec3(0, 0, 0);
	size = glm::vec3(1, 1, 1);
}
NodeVirtual::~NodeVirtual()
{
	for (unsigned int i = 0; i < instanceList.size(); i++)
		InstanceManager::getInstance()->release(instanceList[i]);
	instanceList.clear();

	for (unsigned int i = 0; i < children.size(); i++)
		delete children[i];
	children.clear();
}
//

//	Public functions
int NodeVirtual::getNbFils(int level) { return children.size(); }
bool NodeVirtual::isLastBranch()
{
	if (!children.empty() && children[0]->children.empty()) return true;
	return false;
}

void NodeVirtual::addObject(InstanceVirtual* obj)
{
	if (obj) instanceList.push_back(obj);
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

	return yChild*xChild*z + xChild*y + x;
}
//
