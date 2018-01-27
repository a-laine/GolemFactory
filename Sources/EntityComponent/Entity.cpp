#include "Entity.hpp"


Entity::Entity()
	: m_refCount(0)
	, m_scale(1.f)
	, m_rotation()
	, m_boundingVolume()
{
}

const glm::mat4& Entity::getMatrix() const
{
	return m_transform;
}

glm::vec3 Entity::getPosition() const
{
	return glm::vec3(m_transform[4]);
}

glm::vec3 Entity::getScale() const
{
	return m_scale;
}

glm::quat Entity::getOrientation() const
{
	return m_rotation;
}

void Entity::setPosition(const glm::vec3& position)
{
	m_transform[4] = glm::vec4(position, 1);
}

void Entity::setScale(const glm::vec3& scale)
{
	setTransformation(getPosition(), scale, getOrientation());
}

void Entity::setOrientation(const glm::quat& orientation)
{
	setTransformation(getPosition(), getScale(), orientation);
}

void Entity::setTransformation(const glm::vec3& position, const glm::vec3& scale, const glm::quat& orientation)
{
	m_scale = scale;
	m_rotation = orientation;
	m_transform = glm::mat4(scale[0], 0, 0, 0,
		0, scale[1], 0, 0,
		0, 0, scale[2], 0,
		0, 0, 0, 1);
	m_transform = glm::rotate(m_transform, glm::angle(orientation), glm::axis(orientation));
	m_transform[4] = glm::vec4(position, 1);
}

const BoundingVolume& Entity::getBoundingVolume() const
{
	return m_boundingVolume;
}

BoundingVolume& Entity::getBoundingVolume()
{
	return m_boundingVolume;
}

