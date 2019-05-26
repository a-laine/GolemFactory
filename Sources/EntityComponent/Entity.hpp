#pragma once

#include <atomic>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Physics/BoundingVolume.h>
#include <EntityComponent/Component.hpp>
#include <EntityComponent/EntityBase.hpp>


class World;
class Entity : public EntityBase
{
	friend class EntityManager;

	public:
		//  Default
		Entity();
		//

		void addComponent(Component* component, ClassID type);
		template<typename T> void addComponent(T* component) { addComponent(component, T::getStaticClassID()); }
		void removeComponent(Component* component);

		//	Set/Get functions
		void setPosition(const glm::vec3& position);
		void setScale(const glm::vec3& scale);
		void setOrientation(const glm::quat& orientation);
		void setTransformation(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation);
		void setParentWorld(World* parentWorld);
        void setShape(Shape* Shape);


        uint64_t getId() const;
		const glm::mat4& getMatrix() const;
		glm::vec3 getPosition() const;
		glm::vec3 getScale() const;
		glm::fquat getOrientation() const;
		World* getParentWorld() const;
		const Shape& getShape() const;
		//

	private:
		//	Attributes
		World* m_parentWorld;
		std::atomic<uint32_t> m_refCount;
		glm::vec3 m_scale;
		glm::fquat m_rotation;
		glm::mat4 m_transform;
        Shape* m_boundingShapeVanilla;
		Shape* m_boundingShapeResult;
		//
};


