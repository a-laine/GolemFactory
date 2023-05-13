#pragma once

#include <vector>

#include "Component.hpp"



class EntityBase
{
	public:
		//	Miscellaneous
		struct Element
		{
			Component* comp;
			ClassID type;
		};
		//
		//  Default
		EntityBase();
		~EntityBase();
		//

		unsigned short getNbComponents() const;
		ClassID getTypeID(unsigned short index) const;
		Component* getComponent(unsigned short index);
		const Component* getComponent(unsigned short index) const;
		Component* getComponent(ClassID type);
		const Component* getComponent(ClassID type) const;
		void getAllComponents(ClassID type, std::vector<Component*>& components);
		void getAllComponents(ClassID type, std::vector<const Component*>& components) const;

		template<typename T> T* getComponent() { return static_cast<T*>(getComponent(T::getStaticClassID())); }
		template<typename T> const T* getComponent() const { return static_cast<const T*>(getComponent(T::getStaticClassID())); }
		template<typename T> void getAllComponents(std::vector<Component*>& components) { return getAllComponents(T::getStaticClassID(), components); }

		template<class Visitor> void componentsVisitor(ClassID type, Visitor& visitor)
		{
			for (Element& elem : m_components)
				if (elem.type == type && visitor(elem.comp))
					break;
		}
		template<class Visitor> void allComponentsVisitor(Visitor& visitor)
		{
			for (Element& elem : m_components)
				if (visitor(elem))
					break;
		}

	protected:
		void addComponent(Component* component, ClassID type);
		template<typename T> void addComponent(T* component) { addComponent(component, T::getStaticClassID()); }
		void removeComponent(Component* component);

	private:
		// Attributes
		std::vector<Element> m_components;
		//
};
