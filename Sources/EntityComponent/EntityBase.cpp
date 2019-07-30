#include "EntityBase.hpp"

#include <algorithm>

//	Default
EntityBase::EntityBase() {}
EntityBase::~EntityBase()
{
	for(Element& elem : m_components)
	{
		delete elem.comp;
	}
}
//

//	Public functions
void EntityBase::addComponent(Component* component, ClassID type) { m_components.push_back({ component, type }); }
void EntityBase::removeComponent(Component* component)
{
	for (unsigned int i = 0; i < m_components.size(); i++)
	{
		if (m_components[i].comp == component)
			m_components.erase(m_components.begin() + i);
	}
}
//

//	Set/Get functions
unsigned short EntityBase::getNbComponents() const { return (unsigned short)m_components.size(); }
ClassID EntityBase::getTypeID(unsigned short index) const { return (index < m_components.size()) ? m_components[index].type : Component::getStaticClassID(); }
Component* EntityBase::getComponent(unsigned short index) { return (index < m_components.size()) ? m_components[index].comp : nullptr; }
const Component* EntityBase::getComponent(unsigned short index) const { return (index < m_components.size()) ? m_components[index].comp : nullptr; }
Component* EntityBase::getComponent(ClassID type)
{
	auto it = std::find_if(m_components.begin(), m_components.end(), [type](const Element& e) {return e.type == type; });
	if (it != m_components.end())
		return it->comp;
	else return nullptr;
}
const Component* EntityBase::getComponent(ClassID type) const
{
	auto it = std::find_if(m_components.begin(), m_components.end(), [type](const Element& e) {return e.type == type; });
	if (it != m_components.end())
		return it->comp;
	else return nullptr;
}
void EntityBase::getAllComponents(ClassID type, std::vector<Component*>& components)
{
	for (Element& elem : m_components)
	{
		if (elem.type == type)
			components.push_back(elem.comp);
	}
}
void EntityBase::getAllComponents(ClassID type, std::vector<const Component*>& components) const
{
	for (const Element& elem : m_components)
	{
		if (elem.type == type)
			components.push_back(elem.comp);
	}
}
//
