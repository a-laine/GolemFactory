#include "SceneManager.h"

//  Default
SceneManager::SceneManager()
{
	setViewDistance(100, 2);
	NodeVirtual* n = new NodeVirtual(nullptr,0x040401);
		n->setPosition(0,0,0);
		n->setSize(glm::vec3(100,100,10));
		//n->split();
	world.push_back(n);

	//for (unsigned int i = 0; i < world[0]->children.size(); i++)
		//world[0]->children[i]->split();
}
SceneManager::~SceneManager()
{
	for (unsigned int i = 0; i < world.size(); i++)
		delete world[i];
	world.clear();
}
//

//	Debug
void SceneManager::print()
{
	std::cout << "****printing scene tree****" << std::endl;
	for (unsigned int i = 0; i < world.size(); i++)
		world[i]->print(0);
	std::cout << "***************************" << std::endl;
}
//

//
bool SceneManager::addStaticObject(InstanceVirtual* obj)
{
	return world[0]->addObject(obj);
}
bool SceneManager::removeObject(InstanceVirtual* obj)
{
	return world[0]->removeObject(obj);
}
//

//  Set/get functions
void SceneManager::setCameraAttributes(glm::vec3 position, glm::vec3 direction, glm::vec3 vertical, glm::vec3 left, float verticalAngle, float horizontalAngle)
{
	NodeVirtual::camPosition = position;
	NodeVirtual::camDirection = direction;
	NodeVirtual::camVerticalAngle = verticalAngle;
	NodeVirtual::camHorizontalAngle = horizontalAngle;
	NodeVirtual::camVertical = vertical;
	NodeVirtual::camLeft = left;
}
void SceneManager::setViewDistance(float distance,int depth)
{
	//	take care of user definition variable (and replace by acceptable value if not in input)
	if (depth < 1)
	{
		depth = 1;
		std::cerr << "WARNING : in scene manager 'setViewDistance' : new depth too low -> depth set to " << depth << " instead." << std::endl;
	}
	else if (depth > 5)
	{
		depth = 5;
		std::cerr << "WARNING : in scene manager 'setViewDistance' : new depth too high -> depth set to " << depth << " instead." << std::endl;
	}

	if (distance < 0.1)
	{
		distance = 10;
		std::cerr << "WARNING : in scene manager 'setViewDistance' : new depth too low -> depth set to " << distance << "meter instead." << std::endl;
	}
	else if (distance > 32000)
	{
		distance = 32000;
		std::cerr << "WARNING : in scene manager 'setViewDistance' : new depth too high -> depth set to " << distance << "meter instead." << std::endl;
	}

	//	compute distance stack
	viewMaxDistance.clear();
	for (int i = 0; i < depth; i++)
		viewMaxDistance.push_back(distance/(float)pow(2,i));
}
void SceneManager::setWorldSize(glm::vec3 size)
{
	world[0]->setSize(size);
}
void SceneManager::setWorldPosition(glm::vec3 position)
{
	world[0]->setPosition(position);
}

void SceneManager::getInstanceList(std::vector<std::pair<int, InstanceVirtual*> >& list)
{
	for (unsigned int i = 0; i < world.size(); i++)
		world[i]->getInstanceList(list);
}
std::vector<float> SceneManager::getMaxViewDistanceStack() { return viewMaxDistance; }
//
