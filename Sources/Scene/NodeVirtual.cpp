#include "NodeVirtual.h"

const glm::ivec3 NodeVirtual::izero = glm::ivec3(0);
const glm::ivec3 NodeVirtual::ione = glm::ivec3(1);

//	Default
NodeVirtual::NodeVirtual(NodeVirtual* p, glm::ivec3 d) : parent(p), position(izero), size(ione),  debuginstance(nullptr), allowanceSize(ione)
{
	division = glm::max(d, 1);
	debuginstance = InstanceManager::getInstance()->getInstanceDrawable("cube2.obj", "wired");
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


void NodeVirtual::split(const int& targetDepth, const int& depth)
{
	if (children.empty() && targetDepth >= depth)
	{
		for (int i = 0; i < division.x; i++)
		{
			float xpos = position.x - size.x / 2 + size.x / division.x / 2 + i*size.x / division.x;
			for (int j = 0; j < division.y; j++)
			{
				float ypos = position.y - size.y / 2 + size.y / division.y / 2 + j*size.y / division.y;
				for (int k = 0; k < division.z; k++)
				{
					float zpos = position.z - size.z / 2 + size.z / division.z / 2 + k*size.z / division.z;
					NodeVirtual* n = new NodeVirtual(this, division);
						n->setPosition(glm::vec3(xpos, ypos, zpos));
						n->setSize(glm::vec3(size.x / division.x, size.y / division.y, size.z / division.z));
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
void NodeVirtual::setPosition(const glm::vec3& p)
{
	for (unsigned int i = 0; i < children.size(); i++)
		children[i]->setPosition(children[i]->position - position + p);
	position = p;
	debuginstance->setPosition(glm::vec3(p.x, p.y, p.z));
}
void NodeVirtual::setSize(const glm::vec3& s)
{
	size = s;
	allowanceSize = glm::vec3(std::min(s.x, std::min(s.y, s.z)));
	debuginstance->setSize(0.5f * s);

	if (children.empty()) return;
	for (int i = 0; i < division.x; i++)
	{
		float xpos = position.x - size.x / 2 + size.x / division.x / 2 + i*size.x / division.x;
		for (int j = 0; j < division.y; j++)
		{
			float ypos = position.y - size.y / 2 + size.y / division.y / 2 + j*size.y / division.y;
			for (int k = 0; k < division.z; k++)
			{
				float zpos = position.z - size.z / 2 + size.z / division.z / 2 + k*size.z / division.z;
				int index = division.z * division.y * i + division.z * j + k;
				children[index]->setPosition(glm::vec3(xpos, ypos, zpos));
				children[index]->setSize(glm::vec3(size.x / division.x, size.y / division.y, size.z / division.z));
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
	p += 0.5f * size - position;
	glm::ivec3 result = p / size * glm::vec3(division);
	return division.z * division.y * result.x + division.z * result.y + result.z;
}
glm::ivec3 NodeVirtual::getChildrenKeyVector(glm::vec3 p) const
{
	p += 0.5f * size - position;
		p.x *= division.x / size.x;
		p.y *= division.y / size.y;
		p.z *= division.z / size.z;
	return glm::clamp(glm::ivec3(p), izero, division  - ione);
}
glm::ivec3 NodeVirtual::getDivisionVector() const { return division; }
//
