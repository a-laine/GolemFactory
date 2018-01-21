#include "EntityBase.hpp"


namespace gf {

EntityBase::EntityBase()
{
}

EntityBase::~EntityBase()
{
}

EntityBase::SizeType EntityBase::getNbComponents() const
{
	return (SizeType)m_components.size();
}

ClassID EntityBase::getTypeID(SizeType index) const
{
	return (index < m_components.size()) ? m_components[index].type : Component::getStaticClassID();
}

Component* EntityBase::getComponent(SizeType index)
{
	return (index < m_components.size()) ? m_components[index].comp : nullptr;
}

const Component* EntityBase::getComponent(SizeType index) const
{
	return (index < m_components.size()) ? m_components[index].comp : nullptr;
}

Component* EntityBase::getComponent(ClassID type)
{
	for (auto& elem : m_components)
	{
		if (elem.type == type)
			return elem.comp;
	}
	return nullptr;
}

const Component* EntityBase::getComponent(ClassID type) const
{
	for (const auto& elem : m_components)
	{
		if (elem.type == type)
			return elem.comp;
	}
	return nullptr;
}

void EntityBase::getAllComponents(ClassID type, std::vector<Component*> components)
{
	for (auto& elem : m_components)
	{
		if (elem.type == type)
			components.push_back(elem.comp);
	}
}

void EntityBase::getAllComponents(ClassID type, std::vector<const Component*> components) const
{
	for (const auto& elem : m_components)
	{
		if (elem.type == type)
			components.push_back(elem.comp);
	}
}

void EntityBase::addComponent(Component* component, ClassID type)
{
	m_components.push_back({ component, type });
}

void EntityBase::removeComponent(Component* component)
{
	for (int i = 0; i < m_components.size(); i++)
	{
		if (m_components[i].comp == component)
		{
			m_components.erase(m_components.begin() + i);
		}
	}
}


} // namespace gf
