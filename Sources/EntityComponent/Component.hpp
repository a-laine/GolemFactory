#pragma once



#define GF_DECLARE_COMPONENT_CLASS_GETSTATICCLASSID() static gf::ClassID getStaticClassID() { \
    static char GF_DECLARE_COMPONENT_CLASS_GETCLASSID_var; \
    return reinterpret_cast<gf::ClassID>(&GF_DECLARE_COMPONENT_CLASS_GETCLASSID_var); }

#define GF_DECLARE_COMPONENT_CLASS_GETCLASSID() virtual gf::ClassID getClassID() override { \
    return getStaticClassID(); }

#define GF_DECLARE_COMPONENT_CLASS() public: \
	GF_DECLARE_COMPONENT_CLASS_GETSTATICCLASSID() \
    GF_DECLARE_COMPONENT_CLASS_GETCLASSID() \
    private:


typedef intptr_t ClassID;


/* Exemple :
 * 
 * class MyComponent : public gf::Component
 * {
 * GF_DECLARE_COMPONENT_CLASS()  // REQUIRED
 * public:
 *     MyComponent() {}
 *     virtual ~MyComponent() override {}  // REQUIRED
 * }
 * 
 */
class Component
{
	public:
		static ClassID getStaticClassID()
		{
			return 0;
		}

		virtual ClassID getClassID()
		{
			return getStaticClassID();
		}

	protected:
		Component() = default;
		Component(const Component& other) = delete;
		Component(Component&& other) = delete;
		virtual ~Component() = default;
		Component& operator=(const Component& other) = delete;
};
