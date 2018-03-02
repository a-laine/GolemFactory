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

