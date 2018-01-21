#pragma once

#include <atomic>
#include "Component.hpp"
#include "EntityBase.hpp"
#include "Utiles/BoundingVolume.hpp"


namespace gf {


class Entity : public EntityBase
{
	friend class EntityHandler;

public:
	Entity();

	const glm::mat4& getMatrix() const;
	glm::vec3 getPosition() const;
	glm::vec3 getScale() const;
	glm::quat getOrientation() const;
	void setPosition(const glm::vec3& position);
	void setScale(const glm::vec3& scale);
	void setOrientation(const glm::quat& orientation);
	void setTransformation(const glm::vec3& position, const glm::vec3& scale, const glm::quat& orientation);

	const BoundingVolume& getBoundingVolume() const;
	BoundingVolume& getBoundingVolume();

private:
	// uint32_t flags;
	std::atomic<uint32_t> m_refCount;
	glm::vec3 m_scale;
	glm::quat m_rotation;
	glm::mat4 m_transform;
	BoundingVolume m_boundingVolume;
};


class EntityHandler
{
public:
	EntityHandler() : m_pointer(nullptr) {}
	EntityHandler(Entity* pointer) : m_pointer(pointer) { m_pointer->m_refCount++; }
	EntityHandler(const EntityHandler& other) : m_pointer(other.m_pointer) { m_pointer->m_refCount++; }
	EntityHandler(EntityHandler&& other) : m_pointer(other.m_pointer) { other.m_pointer = nullptr; }
	EntityHandler& operator=(const EntityHandler& other) { m_pointer = other.m_pointer; m_pointer->m_refCount++; }
	EntityHandler& operator=(EntityHandler&& other) { m_pointer = other.m_pointer; other.m_pointer = nullptr; }
	~EntityHandler() {
		m_pointer->m_refCount--;
		if (m_pointer->m_refCount <= 0)
			delete m_pointer; // TODO
	}
	Entity* getObject() { return m_pointer; }
	operator bool() { return m_pointer != nullptr; }

private:
	Entity* m_pointer;
};


} // namespace gf
