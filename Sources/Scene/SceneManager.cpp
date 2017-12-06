#include "SceneManager.h"

#include <glm/gtc/matrix_access.hpp>


#define MAX_DISTANCE_VIEW	32000
#define MIN_DISTANCE_VIEW	10
#define MAX_TREE_DEPTH		5
#define MIN_TREE_DEPTH		2



//  Default
SceneManager::SceneManager()
{
	const float initialDistance = MAX_DISTANCE_VIEW;
	const int initialDepth = MIN_TREE_DEPTH;

	setViewDistance(initialDistance, initialDepth);
	NodeVirtual* n = new NodeVirtual(nullptr, 0x040401);
		n->setPosition(glm::vec3(0.f, 0.f, initialDistance / 10.f));
		n->setSize(glm::vec3(initialDistance, initialDistance, initialDistance / 5.f));
		n->split(2,0);
	world.push_back(n);
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
	for (unsigned int i = 0; i < world.size(); i++)
		if (world[i]->addObject(obj)) return true;
	return false;
}
bool SceneManager::removeObject(InstanceVirtual* obj)
{
	for (unsigned int i = 0; i < world.size(); i++)
		if (world[i]->removeObject(obj)) return true;
	return false;
}
//


//  Set/get functions
void SceneManager::setCameraAttributes(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& vertical, const glm::vec3& left, const float& verticalAngle, const float& horizontalAngle)
{
	camPosition = position;
	camDirection = direction;
	camVerticalAngle = verticalAngle;
	camHorizontalAngle = horizontalAngle;
	camVertical = vertical;
	camLeft = left;
}
void SceneManager::setViewDistance(float distance, int depth)
{
	//	take care of user definition variable (and replace by acceptable value if not in input)
	if (depth < MIN_TREE_DEPTH)
	{
		depth = MIN_TREE_DEPTH;
		std::cerr << "WARNING : in scene manager 'setViewDistance' : new depth too low -> depth set to " << depth << " instead." << std::endl;
	}
	else if (depth > MAX_TREE_DEPTH)
	{
		depth = MAX_TREE_DEPTH;
		std::cerr << "WARNING : in scene manager 'setViewDistance' : new depth too high -> depth set to " << depth << " instead." << std::endl;
	}

	if (distance < MIN_DISTANCE_VIEW)
	{
		distance = MIN_DISTANCE_VIEW;
		std::cerr << "WARNING : in scene manager 'setViewDistance' : new depth too low -> depth set to " << distance << "meter instead." << std::endl;
	}
	else if (distance > MAX_DISTANCE_VIEW)
	{
		distance = MAX_DISTANCE_VIEW;
		std::cerr << "WARNING : in scene manager 'setViewDistance' : new depth too high -> depth set to " << distance << "meter instead." << std::endl;
	}

	//	compute distance stack
	viewMaxDistance.clear();
	for (int i = 0; i < depth; i++)
		viewMaxDistance.push_back(distance / (float)pow(2,i));
}
void SceneManager::setWorldSize(glm::vec3 size)
{
	for (unsigned int i = 0; i < world.size(); i++)
		world[i]->setSize(size);
}
void SceneManager::setWorldPosition(glm::vec3 position)
{
	for (unsigned int i = 0; i < world.size(); i++)
		world[i]->setPosition(position);
}


void SceneManager::getInstanceList(std::vector<std::pair<int, InstanceVirtual*> >& list, const Grain& grain)
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
void SceneManager::getInstanceOnRay(std::vector<std::pair<float, InstanceVirtual*> >& list, const Grain& grain, const float& maxDistance)
{
	//	iterative path on tree to get instance of node on ray
	for (unsigned int i = 0; i < world.size(); i++)
	{
		//	initialize and test root in frustrum
		std::vector<std::pair<NodeVirtual*, unsigned int> > path;
		NodeVirtual* node = world[i];
		float distance = node->isOnRay(camPosition, camDirection, camVertical, camLeft);
		if (distance == std::numeric_limits<float>::lowest())
			continue;
		else
		{
			for (unsigned int i = 0; i < node->instanceList.size(); i++)
				list.push_back(std::pair<float, InstanceVirtual*>(distance, node->instanceList[i]));
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
			distance = node->isOnRay(camPosition, camDirection, camVertical, camLeft);
			if (distance != std::numeric_limits<float>::lowest())
			{
				//	get instance list of node
				for (unsigned int i = 0; i < node->instanceList.size(); i++)
					list.push_back(std::pair<float, InstanceVirtual*>(distance, node->instanceList[i]));
			}

			//	iterate (go down in tree or change branch)
			if (!node->children.empty() && distance != std::numeric_limits<float>::lowest())
				path.push_back(std::pair<NodeVirtual*, int>(node, 0));
			else path.back().second++; // change branch
		}
	}
	if (grain == COARSE) return;

	//	refine list checking instance bounding boxes
	std::vector<std::pair<float, InstanceVirtual*> > refinedList;
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if (it->second->getType() == InstanceVirtual::ANIMATABLE)
		{
			refinedList.push_back(std::pair<float, InstanceVirtual*>(*it));
		}
		else
		{
			Mesh* m = it->second->getMesh();
			if (!m) continue;
			else
			{
				std::vector<glm::vec3> const& vBBox = *m->getBBoxVertices();
				std::vector<unsigned short> const& fBBox = *m->getBBoxFaces();
				glm::mat4 model = it->second->getModelMatrix();
				for (unsigned int i = 0; i < fBBox.size(); i += 6)
				{
					//	compute local base
					glm::vec3 p1 = glm::vec3(model * glm::vec4(vBBox[fBBox[i]], 1.f));				//	vertex 1 of triangle
					glm::vec3 v1 = glm::vec3(model * glm::vec4(vBBox[fBBox[i + 1]], 1.f)) - p1;		//	vertex 2 of triangle - vertex 1 of triangle
					glm::vec3 v2 = glm::vec3(model * glm::vec4(vBBox[fBBox[i + 2]], 1.f)) - p1;		//	vertex 3 of triangle - vertex 1 of triangle
					glm::vec3 normal = glm::cross(v1, v2);
					if (normal == glm::vec3(0.f)) continue;
					glm::normalize(normal);

					//	compute intersection point
					float proj = glm::dot(normal, camDirection);
					if (proj == 0.f) continue;
					if (proj > 0.f) normal *= -1.f;
					float depth = glm::dot(normal, p1 - camPosition) / glm::dot(normal, camDirection);
					if (depth > maxDistance || depth < 0.f) continue;
					glm::vec3 intersection = camPosition + depth * camDirection - p1;

					//	check if point is inside triangle
					float dotv1v2 = glm::dot(v1, v2);
					float magnitute = glm::dot(v2, v2) * glm::dot(v1, v1) - dotv1v2 * dotv1v2;
					glm::vec2 barry;
					barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - dotv1v2 * glm::dot(intersection, v2)) / magnitute;
					barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - dotv1v2 * glm::dot(intersection, v1)) / magnitute;
					if (barry.x < 0.f || barry.y < 0.f || barry.x > 1.f || barry.y > 1.f) continue;
					else refinedList.push_back(std::pair<float, InstanceVirtual*>(depth, it->second));
					break;
				}
			}
		}
	}
	list.swap(refinedList);
	if (grain == INSTANCE_BB || grain == INSTANCE_CAPSULE) return;

	//	refine list checking instance meshes
	refinedList.clear();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		Mesh* m = it->second->getMesh();
		if (!m) continue;
		const std::vector<glm::vec3>& vertices = *m->getVertices();
		const std::vector<unsigned short>& faces = *m->getFaces();
		const glm::mat4 model = it->second->getModelMatrix();

		if (it->second->getType() == InstanceVirtual::ANIMATABLE)
		{
			const std::vector<glm::mat4> ibind = it->second->getSkeleton()->getInverseBindPose();
			const std::vector<glm::mat4> pose = it->second->getPose();
			const std::vector<glm::ivec3>* bones = m->getBones();
			const std::vector<glm::vec3>* weights = m->getWeights();
			if (ibind.empty() || pose.empty() || !bones || !weights) continue;

			for (unsigned int i = 0; i < faces.size(); i += 3)
			{
				glm::mat4 m1(0.f); glm::mat4 m2(0.f); glm::mat4 m3(0.f);
				for (int j = 0; j < 3; j++)
				{
					/*
						faces[i + lambda] -> define index for vertex lambda of triangle
						(*bones)[alpha][j] -> define composante j (x, y, or z), of bone vector definition for vertex alpha
						pose[beta] -> define pose matrix for bone of index beta (idem for ibind)
						(*weights)[alpha][j] -> define composante j (x, y, or z), of weight vector definition for vertex alpha
					*/

					m1 += pose[(*bones)[faces[i    ]][j]] * ibind[(*bones)[faces[i    ]][j]] * (*weights)[(*bones)[faces[i    ]][j]][j];
					m2 += pose[(*bones)[faces[i + 1]][j]] * ibind[(*bones)[faces[i + 1]][j]] * (*weights)[(*bones)[faces[i + 1]][j]][j];
					m3 += pose[(*bones)[faces[i + 2]][j]] * ibind[(*bones)[faces[i + 2]][j]] * (*weights)[(*bones)[faces[i + 2]][j]][j];
				}

				glm::vec3 p1 = glm::vec3(model * m1 * glm::vec4(vertices[faces[i]], 1.f));			//	vertex 1 of triangle
				glm::vec3 v1 = glm::vec3(model * m2 * glm::vec4(vertices[faces[i + 1]], 1.f)) - p1;	//	vertex 2 of triangle - vertex 1 of triangle
				glm::vec3 v2 = glm::vec3(model * m3 * glm::vec4(vertices[faces[i + 2]], 1.f)) - p1;	//	vertex 3 of triangle - vertex 1 of triangle
				glm::vec3 normal = glm::cross(v1, v2);
				if (normal == glm::vec3(0.f)) continue;
				glm::normalize(normal);

				//	compute intersection point
				float proj = glm::dot(normal, camDirection);
				if (proj == 0.f) continue;
				if (proj > 0.f) normal *= -1.f;
				float depth = glm::dot(normal, p1 - camPosition) / glm::dot(normal, camDirection);
				if (depth > maxDistance || depth < 0.f) continue;
				glm::vec3 intersection = camPosition + depth * camDirection - p1;

				//	check if point is inside triangle
				float dotv1v2 = glm::dot(v1, v2);
				float magnitute = glm::dot(v2, v2) * glm::dot(v1, v1) - dotv1v2 * dotv1v2;
				glm::vec2 barry;
				barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - dotv1v2 * glm::dot(intersection, v2)) / magnitute;
				barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - dotv1v2 * glm::dot(intersection, v1)) / magnitute;
				if (barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f) continue;
				else refinedList.push_back(std::pair<float, InstanceVirtual*>(depth, it->second));
				break;
			}
		}
		else
		{
			for (unsigned int i = 0; i < faces.size(); i += 3)
			{
				//	compute local base
				glm::vec3 p1 = glm::vec3(model * glm::vec4(vertices[faces[i]], 1.f));				//	vertex 1 of triangle
				glm::vec3 v1 = glm::vec3(model * glm::vec4(vertices[faces[i + 1]], 1.f)) - p1;		//	vertex 2 of triangle - vertex 1 of triangle
				glm::vec3 v2 = glm::vec3(model * glm::vec4(vertices[faces[i + 2]], 1.f)) - p1;		//	vertex 3 of triangle - vertex 1 of triangle
				glm::vec3 normal = glm::cross(v1, v2);
				if (normal == glm::vec3(0.f)) continue;
				glm::normalize(normal);

				//	compute intersection point
				float proj = glm::dot(normal, camDirection);
				if (proj == 0.f) continue;
				if (proj > 0.f) normal *= -1.f;
				float depth = glm::dot(normal, p1 - camPosition) / glm::dot(normal, camDirection);
				if (depth > maxDistance || depth < 0.f) continue;
				glm::vec3 intersection = camPosition + depth * camDirection - p1;

				//	check if point is inside triangle
				float dotv1v2 = glm::dot(v1, v2);
				float magnitute = glm::dot(v2, v2) * glm::dot(v1, v1) - dotv1v2 * dotv1v2;
				glm::vec2 barry;
				barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - dotv1v2 * glm::dot(intersection, v2)) / magnitute;
				barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - dotv1v2 * glm::dot(intersection, v1)) / magnitute;
				if (barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f) continue;
				else refinedList.push_back(std::pair<float, InstanceVirtual*>(depth, it->second));
				break;
			}
		}
	}
	list.swap(refinedList);
}
std::vector<float> SceneManager::getMaxViewDistanceStack() { return viewMaxDistance; }
//
