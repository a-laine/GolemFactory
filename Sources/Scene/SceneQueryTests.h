#pragma once

#include <map>
#include <glm/gtx/component_wise.hpp>

#include <EntityComponent/Entity.hpp>
#include <Scene/SceneManager.h>


class DefaultSceneManagerBoxTest
{
	public:
		DefaultSceneManagerBoxTest(const glm::vec3& cornerMin, const glm::vec3& cornerMax);

		SceneManager::CollisionType operator() (const NodeVirtual* node) const;
		void getChildren(NodeVirtual* node, std::vector<NodeVirtual::NodeRange>& path) const;

	private:
		glm::vec3 bbMin;
		glm::vec3 bbMax;
};
class DefaultBoxCollector
{
	public:
		DefaultBoxCollector();

		void operator() (NodeVirtual* node, Entity* object);
		std::vector<Entity*>& getObjectInBox();
		void clear();

	private:
		std::vector<Entity*> objectInBox;
};





class DefaultSceneManagerRayTest
{
	public:
		DefaultSceneManagerRayTest(const glm::vec3& pos, const glm::vec3& dir, float maxDist);
		SceneManager::CollisionType operator() (const NodeVirtual* node) const;

	private:
		glm::vec3 position;
		glm::vec3 direction;
		float distance;
};
class DefaultRayPickingCollector
{
	public:
		DefaultRayPickingCollector(const glm::vec3& pos, const glm::vec3& dir, float maxDist);

		void operator() (NodeVirtual* node, Entity* object);
		std::map<float, Entity*>& getObjects();
		Entity* getNearestObject() const;
		float getNearestDistance() const;

	private:
		std::map<float, Entity*> objectOnRay;
		glm::vec3 position;
		glm::vec3 direction;
		float distance;
};




class DefaultSceneManagerFrustrumTest
{
	public:
		DefaultSceneManagerFrustrumTest(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& verticalDir, const glm::vec3& leftDir, float verticalAngle, float horizontalAngle);
		SceneManager::CollisionType operator() (const NodeVirtual* node) const;

	private:
		glm::vec3 camP;
		glm::vec3 camD;
		glm::vec3 camV;
		glm::vec3 camL;
		float camVa;
		float camHa;
};



