#pragma once

#include <cstdint>


typedef intptr_t ClassID;

#define GF_DECLARE_COMPONENT_CLASS_GETSTATICCLASSID() static ClassID getStaticClassID() { \
    static char GF_DECLARE_COMPONENT_CLASS_GETCLASSID_var; \
    return reinterpret_cast<ClassID>(&GF_DECLARE_COMPONENT_CLASS_GETCLASSID_var); }

#define GF_DECLARE_COMPONENT_CLASS_GETCLASSID() virtual ClassID getClassID() override { \
    return getStaticClassID(); }

#define GF_DECLARE_COMPONENT_CLASS_ISIDINHIERARCHY(ParentClassName) virtual bool isIdInHierarchy(ClassID id) override { \
	return id == getStaticClassID() || ParentClassName::isIdInHierarchy(id); }

#define GF_DECLARE_COMPONENT_CLASS(ClassName, ParentClassName) public: \
	GF_DECLARE_COMPONENT_CLASS_GETSTATICCLASSID() \
    GF_DECLARE_COMPONENT_CLASS_GETCLASSID() \
	GF_DECLARE_COMPONENT_CLASS_ISIDINHIERARCHY(ParentClassName) \
    private:


class Entity;


/* Exemple :
 * class MyComponent : public Component
 * {
 *    GF_DECLARE_COMPONENT_CLASS(MyComponent, Component)  // REQUIRED
 *    public:
 *       MyComponent() {}
 *       virtual ~MyComponent() override {}  // REQUIRED
 * }
 */
class Component
{
	public:
		static ClassID getStaticClassID() { return 0; }


		virtual ~Component() = default;

		virtual ClassID getClassID() { return getStaticClassID(); }
		virtual bool isIdInHierarchy(ClassID id) { return id == getStaticClassID(); }

		template<typename T>
		bool isOfType()
		{
			return isIdInHierarchy(T::getStaticClassID());
		}

		template<typename T>
		bool isExactlyOfType()
		{
			return T::getStaticClassID() == getClassID();
		}

		Entity* getParentEntity() const { return m_parent; }
		virtual void onAddToEntity(Entity* entity) { m_parent = entity; }
		virtual void onRemoveFromEntity(Entity* entity) { m_parent = nullptr; }

	protected:
		Component() : m_parent(nullptr) {};
		Component(const Component& other) = delete;
		Component(Component&& other) = delete;
		Component& operator=(const Component& other) = delete;
		Component& operator=(Component&& other) = delete;

	private:
		Entity* m_parent;
};
