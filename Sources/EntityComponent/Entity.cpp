#include "Entity.hpp"
#include <Renderer/DrawableComponent.h>
#include <Utiles/Assert.hpp>

//#include <glm/gtc/quaternion.hpp>


//	Default
Entity::Entity() : m_refCount(0), m_parentWorld(nullptr), m_localBoundingShape(nullptr), m_globalBoundingShape(nullptr), swept(nullptr)
{}
//


void Entity::addComponent(Component* component, ClassID type)
{
	GF_ASSERT(component->getParentEntity() == nullptr, "Bad parent entity. A component can't have multiple parent entities.");
	EntityBase::addComponent(component, type);
	component->onAddToEntity(this);
	GF_ASSERT(component->getParentEntity() == this, "Bad parent entity. You should call Component::onAddToEntity when reimplementing the method.");
}

void Entity::removeComponent(Component* component)
{
	GF_ASSERT(component->getParentEntity() == this, "Bad parent entity. The component has a different parent entity than the one he's beeing removed.");
	EntityBase::removeComponent(component);
	component->onRemoveFromEntity(this);
	GF_ASSERT(component->getParentEntity() == nullptr, "Bad parent entity. You should call Component::onRemoveFromEntity when reimplementing the method.");
}


//	Set/Get functions
void Entity::setPosition(const glm::vec3& position)
{
	setTransformation(position, getScale(), getOrientation());
}
void Entity::setScale(const glm::vec3& scale)
{
	setTransformation(getPosition(), scale, getOrientation());
}
void Entity::setOrientation(const glm::quat& orientation)
{
	setTransformation(getPosition(), getScale(), orientation);
}
void Entity::setTransformation(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	m_transform = glm::translate(glm::mat4(1.0), position);
	m_transform = m_transform * glm::toMat4(orientation);
	m_transform = glm::scale(m_transform, scale);

	if (m_globalBoundingShape)
	{
		*m_globalBoundingShape = *m_localBoundingShape;
		m_globalBoundingShape->transform(position, scale, orientation);
	}
}
void Entity::setParentWorld(World* parentWorld)
{
	m_parentWorld = parentWorld;
}
void Entity::setShape(Shape* Shape)
{
	if (m_localBoundingShape)
		delete m_localBoundingShape;
	m_localBoundingShape = Shape;
	if (m_globalBoundingShape)
		delete m_globalBoundingShape;
	m_globalBoundingShape = m_localBoundingShape->duplicate();
	m_globalBoundingShape->transform(glm::vec3(m_transform[3]), getScale(), getOrientation());
}


uint64_t Entity::getId() const { return reinterpret_cast<uintptr_t>(this); }
const glm::mat4& Entity::getMatrix() const { return m_transform; }
glm::vec3 Entity::getPosition() const
{
	return glm::vec3(m_transform[3]);
}
glm::vec3 Entity::getScale() const
{
	return glm::vec3(glm::length(m_transform[0]), glm::length(m_transform[1]), glm::length(m_transform[2]));
}
glm::fquat Entity::getOrientation() const
{
	glm::vec3 s = getScale();
	const float m00 = m_transform[0][0] / s.x;
	const float m01 = m_transform[0][1] / s.y;
	const float m02 = m_transform[0][2] / s.z;
	const float m10 = m_transform[1][0] / s.x;
	const float m11 = m_transform[1][1] / s.y;
	const float m12 = m_transform[1][2] / s.z;
	const float m20 = m_transform[2][0] / s.x;
	const float m21 = m_transform[2][1] / s.y;
	const float m22 = m_transform[2][2] / s.z;
	return glm::quat_cast(glm::mat3(m00, m01, m02, m10, m11, m12, m20, m21, m22));
}
World* Entity::getParentWorld() const { return m_parentWorld; }

const Shape* Entity::getLocalBoundingShape() const
{
	return m_localBoundingShape;
}

const Shape* Entity::getGlobalBoundingShape() const
{
	return m_globalBoundingShape;
}
//
