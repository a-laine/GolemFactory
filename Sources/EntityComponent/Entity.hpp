#pragma once

#include <atomic>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Physics/BoundingVolume.h>
#include <EntityComponent/Component.hpp>
#include <EntityComponent/EntityBase.hpp>


class World;
class Swept;
class Entity : public EntityBase
{
	friend class EntityManager;
	friend class Physics;
	friend class SceneManager;

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
		void setName(const std::string& _name);


        uint64_t getId() const;
		const glm::mat4& getTransformMatrix() const;
		const glm::mat4& getInverseTransformMatrix() const;
		glm::vec3 getPosition() const;
		glm::vec3 getScale() const;
		glm::fquat getOrientation() const;
		World* getParentWorld() const;
		const Shape* getLocalBoundingShape() const;
		const Shape* getGlobalBoundingShape() const;
		std::string getName() const;
		//

	private:
		//	Attributes
		World* m_parentWorld;
		std::atomic<uint32_t> m_refCount;
		glm::mat4 m_transform;
		glm::mat4 m_inverseTransform;
		std::string name;
        Shape* m_localBoundingShape;
		Shape* m_globalBoundingShape;

		Swept* swept;
		//
};


