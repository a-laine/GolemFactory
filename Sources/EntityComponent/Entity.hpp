#pragma once

#include <atomic>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Physics/Shape.h>
#include <EntityComponent/Component.hpp>
#include <EntityComponent/EntityBase.hpp>

//#include "Utiles/BoundingVolume.h"


class World;

class Entity : public EntityBase
{
	friend class EntityManager;

	public:
		//  Default
		Entity();
		//

		//	Set/Get functions
		void setPosition(const glm::vec3& position);
		void setScale(const glm::vec3& scale);
		void setOrientation(const glm::quat& orientation);
		void setTransformation(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation);
		void setParentWorld(World* parentWorld);
        void setBoundingVolume(const OrientedBox& bbox);


        uint64_t getId() const;
		const glm::mat4& getMatrix() const;
		glm::vec3 getPosition() const;
		glm::vec3 getScale() const;
		glm::fquat getOrientation() const;
		World* getParentWorld() const;
		const OrientedBox& getBoundingVolume() const;
		//

		//const BoundingVolume& getBoundingVolume() const;
		//BoundingVolume& getBoundingVolume();

	private:
		//	Attributes
		// uint32_t flags;
		World* m_parentWorld;
		std::atomic<uint32_t> m_refCount;
		glm::vec3 m_scale;
		glm::fquat m_rotation;
		glm::mat4 m_transform;
        OrientedBox m_boundingVolume;
		//
};


