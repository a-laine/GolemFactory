#include "NodeVirtual.h"

#define FRUSTRUM_COEFF			2.f	//	coefficient for frustrum intersection computation (to avoid artefacts)
#define RAY_COEFF				0.2f


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
	instanceList.push_back(obj);
	return true;
}
bool NodeVirtual::removeObject(InstanceVirtual* obj)
{
	/*if (!obj) return false;
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
	}*/

	auto it = std::find(instanceList.begin(), instanceList.end(), obj);
	if (it != instanceList.end())
	{
		//InstanceManager::getInstance()->release(*it);
		//instanceList.erase(it);
		std::cout << '*';
		return true;
	}
	else return false;
}


void NodeVirtual::split(const int& targetDepth, const int& depth)
{
	if (children.empty() && targetDepth >= depth)
	{
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

		if (targetDepth > depth)
			for (unsigned int i = 0; i < children.size(); i++)
				children[i]->split(targetDepth, depth + 1);
	}
	else if(targetDepth >= depth)
	{
		for (unsigned int i = 0; i < children.size(); i++)
			children[i]->split(targetDepth, depth + 1);
	}
}
void NodeVirtual::merge(const int& targetDepth, const int& depth)
{
	if (targetDepth <= depth)
	{
		for (unsigned int i = 0; i < children.size(); i++)
			delete children[i];
		children.clear();
	}
	else if (!children.empty())
	{
		for (unsigned int i = 0; i < children.size(); i++)
			children[i]->merge(targetDepth, depth);
	}
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
	const int xChild = (division & X) >> dX;
	const int yChild = (division & Y) >> dY;
	const int zChild = (division & Z) >> dZ;

	p -= position;
	p += 0.5f * size;

	const int x = (int)(p.x / size.x * xChild);
	const int y = (int)(p.y / size.y * yChild);
	const int z = (int)(p.z / size.z * zChild);
	return zChild * yChild * x + zChild * y + z;
}
glm::ivec3 NodeVirtual::getChildrenKeyVector(glm::vec3 p) const
{
	const int xChild = (division & X) >> dX;
	const int yChild = (division & Y) >> dY;
	const int zChild = (division & Z) >> dZ;

	p -= position;
	p += 0.5f * size;

	const int x = (int)(p.x / size.x * xChild);
	const int y = (int)(p.y / size.y * yChild);
	const int z = (int)(p.z / size.z * zChild);

	return glm::ivec3(glm::clamp(x, 0, xChild - 1), glm::clamp(y, 0, yChild - 1), glm::clamp(z, 0, zChild - 1));
}
glm::ivec3 NodeVirtual::getDivisionVector() const { return glm::ivec3((division & X) >> dX, (division & Y) >> dY, (division & Z) >> dZ); }
//
