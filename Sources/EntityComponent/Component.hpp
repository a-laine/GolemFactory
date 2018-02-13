#pragma once

typedef intptr_t ClassID;

#define GF_DECLARE_COMPONENT_CLASS_GETSTATICCLASSID() static ClassID getStaticClassID() { \
    static char GF_DECLARE_COMPONENT_CLASS_GETCLASSID_var; \
    return reinterpret_cast<ClassID>(&GF_DECLARE_COMPONENT_CLASS_GETCLASSID_var); }

#define GF_DECLARE_COMPONENT_CLASS_GETCLASSID() virtual ClassID getClassID() override { \
    return getStaticClassID(); }

#define GF_DECLARE_COMPONENT_CLASS() public: \
	GF_DECLARE_COMPONENT_CLASS_GETSTATICCLASSID() \
    GF_DECLARE_COMPONENT_CLASS_GETCLASSID() \
    private:


/* Exemple :
 * class MyComponent : public Component
 * {
 *    GF_DECLARE_COMPONENT_CLASS()  // REQUIRED
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

	protected:
		Component() = default;
		Component(const Component& other) = delete;
		Component(Component&& other) = delete;
		virtual ~Component() = default;
		Component& operator=(const Component& other) = delete;
};
