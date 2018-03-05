#pragma once

#include <glm/gtx/component_wise.hpp>
#include "SceneManager.h"


//	coefficient for intersection test computation (to avoid artefacts)
#define RAY_COEFF				0.2f
#define FRUSTRUM_COEFF			2.f


class DefaultSceneManagerBoxTest
{
	public:
		DefaultSceneManagerBoxTest(const glm::vec3& bbMin, const glm::vec3& bbMax)
			: center((bbMin + bbMax) * 0.5f)
			, halfSize((bbMax - bbMin) * 0.5f)
		{}

		SceneManager::CollisionType operator() (const NodeVirtual* node) const
		{
			glm::vec3 nodeHalfSize = node->getSize() * 0.5f;
			glm::vec3 allowance(node->allowanceSize);
			const glm::vec3 p = glm::max(glm::abs(node->getCenter() - center) - allowance, 0.f);
			if(p.x >  halfSize.x + nodeHalfSize.x || p.y >  halfSize.y + nodeHalfSize.y || p.z >  halfSize.z + nodeHalfSize.z) return SceneManager::NONE;
			if(p.x <= halfSize.x - nodeHalfSize.x && p.y <= halfSize.y - nodeHalfSize.y && p.z <= halfSize.z - nodeHalfSize.z) return SceneManager::INSIDE;
			else return SceneManager::OVERLAP;
		}

		void getChildren(NodeVirtual* node, std::vector<NodeVirtual::NodeRange>& path) const
		{
			node->getChildrenInBox(path, center - halfSize, center + halfSize);
		}

	private:
		glm::vec3 center;
		glm::vec3 halfSize;
};

class DefaultSceneManagerRayTest
{
	public:
		DefaultSceneManagerRayTest(const glm::vec3& pos, const glm::vec3& dir, float maxDist)
			: position(pos), direction(dir), distance(maxDist)
		{}

		SceneManager::CollisionType operator() (const NodeVirtual* node) const
		{
			const glm::vec3 t1 = (node->getBBMin() - position) / direction;
			const glm::vec3 t2 = (node->getBBMax() - position) / direction;
			float tnear = glm::compMax(glm::min(t1, t2));
			float tfar = glm::compMin(glm::max(t1, t2));
			return (tfar >= tnear && tfar >= 0 && tnear <= distance) ? SceneManager::OVERLAP : SceneManager::NONE;
		}

	private:
		glm::vec3 position;
		glm::vec3 direction;
		float distance;
};

class DefaultSceneManagerFrustrumTest
{
	public:
		DefaultSceneManagerFrustrumTest(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& verticalDir, const glm::vec3& leftDir, float verticalAngle, float horizontalAngle)
			: camP(position)
			, camD(direction)
			, camV(verticalDir)
			, camL(leftDir)
			, camVa(verticalAngle)
			, camHa(horizontalAngle)
		{}

		SceneManager::CollisionType operator() (const NodeVirtual* node) const
		{
			//	test if in front of camera
			const glm::vec3 p = node->getCenter() - camP;
			const glm::vec3 size = node->getSize();
			float forwardFloat = glm::dot(p, camD) + FRUSTRUM_COEFF * (abs(size.x * camD.x) + abs(size.y * camD.y) + abs(size.z * camD.z));
			if(forwardFloat < 0.f)
				return SceneManager::NONE;

			//	out of horizontal range
			float maxAbsoluteDimension = (std::max)(size.x, (std::max)(size.y, size.z)) / 2.f;
			float maxTangentDimension = abs(size.x * camL.x) / 2.f + abs(size.y * camL.y) / 2.f + abs(size.z * camL.z) / 2.f;
			if(abs(glm::dot(p, camL)) - maxTangentDimension > std::abs(forwardFloat) * tan(glm::radians(camHa)) + FRUSTRUM_COEFF * maxAbsoluteDimension)
				return SceneManager::NONE;

			//	out of vertical range
			maxTangentDimension = abs(size.x * camV.x) / 2.f + abs(size.y * camV.y) / 2.f + abs(size.z * camV.z) / 2.f;
			if(abs(glm::dot(p, camV)) - maxTangentDimension > abs(forwardFloat) * tan(glm::radians(camVa)) + FRUSTRUM_COEFF * maxAbsoluteDimension)
				return SceneManager::NONE;

			//	return distance to camera in int
			//return (int) glm::length(p);
			return SceneManager::OVERLAP;
		}

	private:
		glm::vec3 camP;
		glm::vec3 camD;
		glm::vec3 camV;
		glm::vec3 camL;
		float camVa;
		float camHa;
};

class DefaultRayPickingCollector
{
	public:
		DefaultRayPickingCollector() : nearestObject(nullptr), distance(std::numeric_limits<float>::max()) {}

		void operator() (const NodeVirtual* node, InstanceVirtual* object)
		{
			// TODO raycast object more precisely (BB, physics shape, mesh)
		}

		InstanceVirtual* getObject() const { return nearestObject; }
		float getDistance() const { return distance; }

	private:
		InstanceVirtual* nearestObject;
		float distance;
};




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

