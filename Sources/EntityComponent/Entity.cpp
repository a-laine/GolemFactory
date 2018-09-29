#include "Entity.hpp"

//	Default
Entity::Entity() : m_refCount(0), m_scale(1.f), m_rotation() //, m_boundingVolume()
{}
//

//	Set/Get functions
void Entity::setPosition(const glm::vec3& position) { m_transform[3] = glm::vec4(position, 1); }
void Entity::setScale(const glm::vec3& scale)
{
	setTransformation(getPosition(), scale, m_rotation);
}
void Entity::setOrientation(const glm::quat& orientation)
{
	setTransformation(getPosition(), m_scale, orientation);
}
void Entity::setTransformation(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	m_scale = scale;
	m_rotation = orientation;

	m_transform = glm::translate(glm::mat4(1.0), position);
	m_transform = m_transform * glm::toMat4(orientation);
	m_transform = glm::scale(m_transform, scale);
}
void Entity::setParentWorld(World* parentWorld)
{
	m_parentWorld = parentWorld;
}
void Entity::setBoundingVolume(const OrientedBox& bbox)
{
	m_boundingVolume = bbox;
}

uint64_t Entity::getId() const { return reinterpret_cast<uintptr_t>(this); }
const glm::mat4& Entity::getMatrix() const { return m_transform; }
glm::vec3 Entity::getPosition() const { return glm::vec3(m_transform[3]); }
glm::vec3 Entity::getScale() const { return m_scale; }
glm::fquat Entity::getOrientation() const { return m_rotation; }
World* Entity::getParentWorld() const { return m_parentWorld; }
const OrientedBox& Entity::getBoundingVolume() const { return m_boundingVolume; }
//
