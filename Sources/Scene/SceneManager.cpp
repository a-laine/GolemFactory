#include "SceneManager.h"

#include <glm/gtc/matrix_access.hpp>

//  Default
SceneManager::SceneManager()
{
	setViewDistance(100, 2);
	NodeVirtual* n = new NodeVirtual(nullptr, 0x040401);
		n->setPosition(glm::vec3(0.f, 0.f, 0.f));
		n->setSize(glm::vec3(100.f, 100.f, 10.f));
		n->split();
	world.push_back(n);

	for (unsigned int i = 0; i < world[0]->children.size(); i++)
		world[0]->children[i]->split();
}
SceneManager::~SceneManager()
{
	for (unsigned int i = 0; i < world.size(); i++)
		delete world[i];
	world.clear();
}
//


//  Scene modifier
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
	camPosition = position;
	camDirection = direction;
	camVerticalAngle = verticalAngle;
	camHorizontalAngle = horizontalAngle;
	camVertical = vertical;
	camLeft = left;
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

void SceneManager::getInstanceList(std::list<std::pair<int, InstanceVirtual*> >& list, const Grain& grain)
{
	//	iterative path on tree to get instance of node in frustrum
	for (unsigned int i = 0; i < world.size(); i++)
	{
		//	initialize and test root in frustrum
		std::vector<std::pair<NodeVirtual*, unsigned int> > path;
		NodeVirtual* node = world[i];
		int distance = node->isInFrustrum(camPosition, camDirection, camVertical, camLeft, camVerticalAngle, camHorizontalAngle);
		if (distance == std::numeric_limits<int>::lowest())
			continue;
		else
		{
			for (unsigned int i = 0; i < node->instanceList.size(); i++)
				list.push_back(std::pair<int, InstanceVirtual*>(distance, node->instanceList[i]));
		}

		//	init path and iterate on tree
		if(!node->children.empty())
			path.push_back(std::pair<NodeVirtual*, unsigned int>(node, 0));
		while (!path.empty())
		{
			// go up and change branch if invalid current node
			if (path.back().second >= path.back().first->children.size())
			{
				path.pop_back();
				if (!path.empty()) path.back().second++;
				continue;
			}

			//	test if node in frustrum and iterate if not
			node = path.back().first->children[path.back().second];
			distance = node->isInFrustrum(camPosition, camDirection, camVertical, camLeft, camVerticalAngle, camHorizontalAngle);
			if (distance != std::numeric_limits<int>::lowest())
			{
				//	get instance list of node
				for (unsigned int i = 0; i < node->instanceList.size(); i++)
					list.push_back(std::pair<int, InstanceVirtual*>(distance, node->instanceList[i]));
			}

			//	iterate
			if (!node->children.empty() && distance != std::numeric_limits<int>::lowest())	//	go down in tree
				path.push_back(std::pair<NodeVirtual*, int>(node, 0));
			else path.back().second++;														// change branch
		}
	}

	//	refine list
	if (grain == COARSE) return;
}
void SceneManager::getInstanceOnRay(std::list<std::pair<int, InstanceVirtual*> >& list, const Grain& grain)
{
	//	iterative path on tree to get instance of node on ray
	for (unsigned int i = 0; i < world.size(); i++)
	{
		//	initialize and test root in frustrum
		std::vector<std::pair<NodeVirtual*, unsigned int> > path;
		NodeVirtual* node = world[i];
		int distance = node->isOnRay(camPosition, camDirection);
		if (distance == std::numeric_limits<int>::lowest())
			continue;
		else
		{
			for (unsigned int i = 0; i < node->instanceList.size(); i++)
				list.push_back(std::pair<int, InstanceVirtual*>(distance, node->instanceList[i]));
		}

		//	init path and iterate on tree
		if (!node->children.empty())
			path.push_back(std::pair<NodeVirtual*, unsigned int>(node, 0));
		while (!path.empty())
		{
			// go up and change branch if invalid current node
			if (path.back().second >= path.back().first->children.size())
			{
				path.pop_back();
				if (!path.empty()) path.back().second++;
				continue;
			}

			//	test if node in frustrum and iterate if not
			node = path.back().first->children[path.back().second];
			distance = node->isOnRay(camPosition, camDirection);
			if (distance != std::numeric_limits<int>::lowest())
			{
				//	get instance list of node
				for (unsigned int i = 0; i < node->instanceList.size(); i++)
					list.push_back(std::pair<int, InstanceVirtual*>(distance, node->instanceList[i]));
			}

			//	iterate
			if (!node->children.empty() && distance != std::numeric_limits<int>::lowest())	//	go down in tree
				path.push_back(std::pair<NodeVirtual*, int>(node, 0));
			else path.back().second++;														// change branch
		}
	}

	//	refine list checking instance bounding boxes
	if (grain == COARSE) return;
	for (std::list<std::pair<int, InstanceVirtual*> >::iterator it = list.begin(); it != list.end();)
	{
		Mesh* m = it->second->getMesh();
		if (!m) it = list.erase(it);
		else
		{
			std::vector<glm::vec3> const& vBBox = *m->getBBoxVertecies();
			std::vector<unsigned short> const& fBBox = *m->getBBoxFaces();
			bool intersect = false;

			for (unsigned int i = 0; i < fBBox.size(); i += 3)
			{
				//	compute triangles vertices in world space
				glm::vec3 p1 = glm::vec3(it->second->getModelMatrix() * glm::vec4(vBBox[fBBox[i]], 1.f));
				glm::vec3 p2 = glm::vec3(it->second->getModelMatrix() * glm::vec4(vBBox[fBBox[i + 1]], 1.f));
				glm::vec3 p3 = glm::vec3(it->second->getModelMatrix() * glm::vec4(vBBox[fBBox[i + 2]], 1.f));

				//	compute local base
				glm::vec3 v1 = p2 - p1;
				glm::vec3 v2 = p3 - p1;
				glm::vec3 normal = glm::cross(v1, v2);
				if (normal == glm::vec3(0.f)) continue;
				glm::normalize(normal);

				//	compute intersection point
				if (glm::dot(normal, camDirection) == 0.f) continue;
				float depth = glm::dot(normal, p1) / glm::dot(normal, camDirection);
				glm::vec3 intersection = depth * camDirection - p1;

				//	check if point is inside triangle
				float magnitute = glm::dot(v2, v2)*glm::dot(v1, v1) - glm::dot(v1, v2)*glm::dot(v1, v2);
				glm::vec2 barry;
				barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - glm::dot(v2, v1) * glm::dot(intersection, v2)) / magnitute;
				barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - glm::dot(v2, v1) * glm::dot(intersection, v1)) / magnitute;
				if (barry.x < 0.f || barry.y < 0.f || barry.x > 1.f || barry.y > 1.f) continue;
				else intersect =  true;
			}
			if (intersect) ++it;
			else it = list.erase(it);
		}

		//float p = glm::dot(it->second->getPosition() - camPosition, camDirection);
		//float d = glm::length(it->second->getPosition() - camPosition - p * camDirection);

		//if (it->second->getBSRadius() < 0) it = list.erase(it);
		//else ++it;


		/*glm::mat4 base = glm::translate(glm::mat4(), it->second->getPosition()) * it->second->getOrientation();
		glm::vec3 aabb_min = it->second->getBBMin();
		glm::vec3 aabb_max = it->second->getBBMax();

		//	tmin & tmax on x axis
		glm::vec3 axis = glm::vec3(glm::column(base, 0));
		float t1 = (glm::dot(axis, it->second->getPosition() - camPosition) + aabb_min.x) / glm::dot(camDirection, axis);	
		float t2 = (glm::dot(axis, it->second->getPosition() - camPosition) + aabb_max.x) / glm::dot(camDirection, axis);	

		//	tmin & tmax on x axis
		axis = glm::vec3(glm::column(base, 1));
		float t3 = (glm::dot(axis, it->second->getPosition() - camPosition) + aabb_min.y) / glm::dot(camDirection, axis);
		float t4 = (glm::dot(axis, it->second->getPosition() - camPosition) + aabb_max.y) / glm::dot(camDirection, axis);

		//	tmin & tmax on z axis
		axis = glm::vec3(glm::column(base, 2));
		float t5 = (glm::dot(axis, it->second->getPosition() - camPosition) + aabb_min.z) / glm::dot(camDirection, axis);
		float t6 = (glm::dot(axis, it->second->getPosition() - camPosition) + aabb_max.z) / glm::dot(camDirection, axis);

		//	test
		float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
		float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));*/

		//if (tmin > tmax) it = list.erase(it);
		//else ++it;
	}

	//	refine list checking instance meshes
	if (grain == INSTANCE_BB) return;
}
std::vector<float> SceneManager::getMaxViewDistanceStack() { return viewMaxDistance; }
//
