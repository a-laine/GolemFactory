#pragma once

#include <vector>

#include "Component.hpp"



class EntityBase
{
	public:
		//  Miscellaneous
		typedef uint16_t SizeType;
		//

		EntityBase();
		~EntityBase();

		SizeType getNbComponents() const;
		ClassID getTypeID(SizeType index) const;
		Component* getComponent(SizeType index);
		const Component* getComponent(SizeType index) const;
		Component* getComponent(ClassID type);
		const Component* getComponent(ClassID type) const;
		void getAllComponents(ClassID type, std::vector<Component*> components);
		void getAllComponents(ClassID type, std::vector<const Component*> components) const;
		void addComponent(Component* component, ClassID type);
		void removeComponent(Component* component);

		template<typename T> T* getComponent() { return static_cast<T*>(getComponent(T::getStaticClassID())); }
		template<typename T> const T* getComponent() const { return static_cast<const T*>(getComponent(T::getStaticClassID())); }
		template<typename T> void getAllComponents(std::vector<Component*> components) { return getAllComponents(T::getStaticClassID(), components); }
		template<typename T> void addComponent(T* component) { addComponent(component, T::getStaticClassID()); }

	private:
		struct Element
		{
			Component* comp;
			ClassID type;
		};

		std::vector<Element> m_components;
};
