#include "SceneManager.h"

#include <algorithm>
#include <glm/gtc/matrix_access.hpp>
#include <Utiles/Assert.hpp>
#include "SceneQueryTests.h"




SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
	for (unsigned int i = 0; i < world.size(); i++)
		delete world[i];
}


void SceneManager::init(const glm::vec3& bbMin, const glm::vec3& bbMax, const glm::ivec3& nodeDivision, unsigned int depth)
{
	GF_ASSERT(world.empty());
	NodeVirtual* n = new NodeVirtual();
	n->init(bbMin, bbMax, nodeDivision, depth);
	world.push_back(n);
}

void SceneManager::reserveInstanceTrack(const unsigned int& count)
{
	instanceTracking.reserve(count);
}

unsigned int SceneManager::getObjectCount() const
{
	return instanceTracking.size();
}


bool SceneManager::addObject(InstanceVirtual* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) != 0)
		return false;

	NodeVirtual* node = world[0];
	if(!node->isInside(object->position))
		return false;

	while(!node->isLeaf() && node->isTooSmall(object->getBBMax() - object->getBBMin()))
		node = node->getChildAt(object->position);

	node->addObject(object);
	instanceTracking[object] = {object->position, node};
	return true;
}

bool SceneManager::removeObject(InstanceVirtual* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) == 0)
		return false;

	auto track = instanceTracking.find(object);
	NodeVirtual* node = track->second.owner;
	instanceTracking.erase(track);
	
	return node->removeObject(object);
}

bool SceneManager::updateObject(InstanceVirtual* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) == 0)
		return false;

	auto track = instanceTracking.find(object);
	NodeVirtual* node = track->second.owner;

	glm::vec3 objectSize = object->getBBMax() - object->getBBMin();
	if (!node->isInside(object->position) || node->isTooSmall(objectSize) || node->isTooBig(objectSize))
	{
		node->removeObject(object);

		node = world[0];
		if (!node->isInside(object->position))
		{
			instanceTracking.erase(track);
			InstanceManager::getInstance()->release(object);
			return false;
		}
		else
		{
			while(!node->isLeaf() && node->isTooSmall(object->getBBMax() - object->getBBMin()))
				node = node->getChildAt(object->position);
			node->addObject(object);
			track->second.owner = node;
		}
	}
	track->second.position = object->position;
	return true;
}


void SceneManager::getAllObjects(std::vector<InstanceVirtual*>& result)
{
	if(!world.empty())
	{
		getObjects(world[0], result, [](NodeVirtual* node) -> CollisionType { return OVERLAP; });
	}
}

void SceneManager::getObjectsOnRay(std::vector<InstanceVirtual*>& result, const glm::vec3& position, const glm::vec3& direction, float maxDistance)
{
	if(world.empty())
		return;

	DefaultSceneManagerRayTest test(position, direction, maxDistance);
	getObjects(world[0], result, test);
}

void SceneManager::getObjectsInBox(std::vector<InstanceVirtual*>& result, const glm::vec3& bbMin, const glm::vec3& bbMax)
{
	if(world.empty())
		return;

	DefaultSceneManagerBoxTest test(bbMin, bbMax);
	getObjectsInBox(world[0], result, test);
}





//void SceneManager::getInstanceOnRay(std::vector<std::pair<float, InstanceVirtual*> >& list, const Grain& grain, const float& maxDistance)
//{
//	//	refine list checking instance bounding boxes
//	std::vector<std::pair<float, InstanceVirtual*> > refinedList;
//	for (auto it = list.begin(); it != list.end(); ++it)
//	{
//		if (it->second->getType() == InstanceVirtual::ANIMATABLE)
//		{
//			refinedList.push_back(std::pair<float, InstanceVirtual*>(*it));
//		}
//		else
//		{
//			Mesh* m = it->second->getMesh();
//			if (!m) continue;
//			else
//			{
//				std::vector<glm::vec3> const& vBBox = *m->getBBoxVertices();
//				std::vector<unsigned short> const& fBBox = *m->getBBoxFaces();
//				glm::mat4 model = it->second->getModelMatrix();
//				for (unsigned int i = 0; i < fBBox.size(); i += 6)
//				{
//					//	compute local base
//					glm::vec3 p1 = glm::vec3(model * glm::vec4(vBBox[fBBox[i]], 1.f));				//	vertex 1 of triangle
//					glm::vec3 v1 = glm::vec3(model * glm::vec4(vBBox[fBBox[i + 1]], 1.f)) - p1;		//	vertex 2 of triangle - vertex 1 of triangle
//					glm::vec3 v2 = glm::vec3(model * glm::vec4(vBBox[fBBox[i + 2]], 1.f)) - p1;		//	vertex 3 of triangle - vertex 1 of triangle
//					glm::vec3 normal = glm::cross(v1, v2);
//					if (normal == glm::vec3(0.f)) continue;
//					glm::normalize(normal);
//
//					//	compute intersection point
//					float proj = glm::dot(normal, camDirection);
//					if (proj == 0.f) continue;
//					if (proj > 0.f) normal *= -1.f;
//					float depth = glm::dot(normal, p1 - camPosition) / glm::dot(normal, camDirection);
//					if (depth > maxDistance || depth < 0.f) continue;
//					glm::vec3 intersection = camPosition + depth * camDirection - p1;
//
//					//	check if point is inside triangle
//					float dotv1v2 = glm::dot(v1, v2);
//					float magnitute = glm::dot(v2, v2) * glm::dot(v1, v1) - dotv1v2 * dotv1v2;
//					glm::vec2 barry;
//					barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - dotv1v2 * glm::dot(intersection, v2)) / magnitute;
//					barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - dotv1v2 * glm::dot(intersection, v1)) / magnitute;
//					if (barry.x < 0.f || barry.y < 0.f || barry.x > 1.f || barry.y > 1.f) continue;
//					else refinedList.push_back(std::pair<float, InstanceVirtual*>(depth, it->second));
//					break;
//				}
//			}
//		}
//	}
//	list.swap(refinedList);
//	if (grain == INSTANCE_BB || grain == INSTANCE_CAPSULE) return;
//
//	//	refine list checking instance meshes
//	refinedList.clear();
//	for (auto it = list.begin(); it != list.end(); ++it)
//	{
//		Mesh* m = it->second->getMesh();
//		if (!m) continue;
//		const std::vector<glm::vec3>& vertices = *m->getVertices();
//		const std::vector<unsigned short>& faces = *m->getFaces();
//		const glm::mat4 model = it->second->getModelMatrix();
//
//		if (it->second->getType() == InstanceVirtual::ANIMATABLE)
//		{
//			const std::vector<glm::mat4> ibind = it->second->getSkeleton()->getInverseBindPose();
//			const std::vector<glm::mat4> pose = it->second->getPose();
//			const std::vector<glm::ivec3>* bones = m->getBones();
//			const std::vector<glm::vec3>* weights = m->getWeights();
//			if (ibind.empty() || pose.empty() || !bones || !weights) continue;
//
//			for (unsigned int i = 0; i < faces.size(); i += 3)
//			{
//				glm::mat4 m1(0.f); glm::mat4 m2(0.f); glm::mat4 m3(0.f);
//				for (int j = 0; j < 3; j++)
//				{
//					/*
//						WARNNING : CRYPTIC PART INCOMING, READ EXPLANATION BEFORE CRYING
//
//						faces[i + lambda] -> define index for vertex lambda of triangle
//						(*bones)[alpha][j] -> define composante j (x, y, or z), of bone vector definition for vertex alpha
//						pose[beta] -> define pose matrix for bone of index beta (idem for ibind)
//						(*weights)[alpha][j] -> define composante j (x, y, or z), of weight vector definition for vertex alpha
//					*/
//
//					m1 += pose[(*bones)[faces[i    ]][j]] * ibind[(*bones)[faces[i    ]][j]] * (*weights)[(*bones)[faces[i    ]][j]][j];
//					m2 += pose[(*bones)[faces[i + 1]][j]] * ibind[(*bones)[faces[i + 1]][j]] * (*weights)[(*bones)[faces[i + 1]][j]][j];
//					m3 += pose[(*bones)[faces[i + 2]][j]] * ibind[(*bones)[faces[i + 2]][j]] * (*weights)[(*bones)[faces[i + 2]][j]][j];
//				}
//
//				glm::vec3 p1 = glm::vec3(model * m1 * glm::vec4(vertices[faces[i]], 1.f));			//	vertex 1 of triangle
//				glm::vec3 v1 = glm::vec3(model * m2 * glm::vec4(vertices[faces[i + 1]], 1.f)) - p1;	//	vertex 2 of triangle - vertex 1 of triangle
//				glm::vec3 v2 = glm::vec3(model * m3 * glm::vec4(vertices[faces[i + 2]], 1.f)) - p1;	//	vertex 3 of triangle - vertex 1 of triangle
//				glm::vec3 normal = glm::cross(v1, v2);
//				if (normal == glm::vec3(0.f)) continue;
//				glm::normalize(normal);
//
//				//	compute intersection point
//				float proj = glm::dot(normal, camDirection);
//				if (proj == 0.f) continue;
//				if (proj > 0.f) normal *= -1.f;
//				float depth = glm::dot(normal, p1 - camPosition) / glm::dot(normal, camDirection);
//				if (depth > maxDistance || depth < 0.f) continue;
//				glm::vec3 intersection = camPosition + depth * camDirection - p1;
//
//				//	check if point is inside triangle
//				float dotv1v2 = glm::dot(v1, v2);
//				float magnitute = glm::dot(v2, v2) * glm::dot(v1, v1) - dotv1v2 * dotv1v2;
//				glm::vec2 barry;
//				barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - dotv1v2 * glm::dot(intersection, v2)) / magnitute;
//				barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - dotv1v2 * glm::dot(intersection, v1)) / magnitute;
//				if (barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f) continue;
//				else refinedList.push_back(std::pair<float, InstanceVirtual*>(depth, it->second));
//				break;
//			}
//		}
//		else
//		{
//			for (unsigned int i = 0; i < faces.size(); i += 3)
//			{
//				//	compute local base
//				glm::vec3 p1 = glm::vec3(model * glm::vec4(vertices[faces[i]], 1.f));				//	vertex 1 of triangle
//				glm::vec3 v1 = glm::vec3(model * glm::vec4(vertices[faces[i + 1]], 1.f)) - p1;		//	vertex 2 of triangle - vertex 1 of triangle
//				glm::vec3 v2 = glm::vec3(model * glm::vec4(vertices[faces[i + 2]], 1.f)) - p1;		//	vertex 3 of triangle - vertex 1 of triangle
//				glm::vec3 normal = glm::cross(v1, v2);
//				if (normal == glm::vec3(0.f)) continue;
//				glm::normalize(normal);
//
//				//	compute intersection point
//				float proj = glm::dot(normal, camDirection);
//				if (proj == 0.f) continue;
//				if (proj > 0.f) normal *= -1.f;
//				float depth = glm::dot(normal, p1 - camPosition) / glm::dot(normal, camDirection);
//				if (depth > maxDistance || depth < 0.f) continue;
//				glm::vec3 intersection = camPosition + depth * camDirection - p1;
//
//				//	check if point is inside triangle
//				float dotv1v2 = glm::dot(v1, v2);
//				float magnitute = glm::dot(v2, v2) * glm::dot(v1, v1) - dotv1v2 * dotv1v2;
//				glm::vec2 barry;
//				barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - dotv1v2 * glm::dot(intersection, v2)) / magnitute;
//				barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - dotv1v2 * glm::dot(intersection, v1)) / magnitute;
//				if (barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f) continue;
//				else refinedList.push_back(std::pair<float, InstanceVirtual*>(depth, it->second));
//				break;
//			}
//		}
//	}
//	list.swap(refinedList);
//}

