#pragma once

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

	protected:
		Component() = default;
		Component(const Component& other) = delete;
		Component(Component&& other) = delete;
		virtual ~Component() = default;
		Component& operator=(const Component& other) = delete;
};
