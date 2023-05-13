#pragma once

#include <atomic>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Physics/BoundingVolume.h>
#include <EntityComponent/Component.hpp>
#include <EntityComponent/EntityBase.hpp>


class World;
class Swept;
class Entity : public EntityBase
{
	friend class EntityManager;
	friend class Physics;
	friend class SceneManager;

	public:
		//
		enum class Flags : uint64_t
		{
			#define FLAG_MACRO(name,value) Fl_##name = value,
			#include "EntityFlags.h"
			#undef FLAG_MACRO
		};
		//

		//  Default
		Entity();
		//

		// public
		void addComponent(Component* component, ClassID type);
		template<typename T> void addComponent(T* component) { addComponent(component, T::getStaticClassID()); }
		void removeComponent(Component* component);

		void recomputeBoundingBox();

		static std::string getFlagName(uint64_t flag);
		//

		//	Set/Get functions
		void setWorldPosition(const vec4f& position);
		void setWorldScale(const float& scale);
		void setWorldOrientation(const quatf& orientation);
		void setWorldTransformation(const vec4f& position, const float& scale, const quatf& orientation);
		void setLocalPosition(const vec4f& position);
		void setLocalScale(const float& scale);
		void setLocalOrientation(const quatf& orientation);
		void setLocalTransformation(const vec4f& position, const float& scale, const quatf& orientation);
		void touchTransform();
		void setParentWorld(World* parentWorld);
		void setName(const std::string& _name);
		Entity* getParent();
		std::vector<Entity*>& getChilds();
		void setFlags(uint64_t _f);
		void clearFlags(uint64_t _f);


        uint64_t getId() const;
		const mat4f& getWorldTransformMatrix();
		const mat4f& getInverseWorldTransformMatrix();
		vec4f getWorldPosition() const;
		float getWorldScale() const;
		quatf getWorldOrientation() const;
		vec4f getLocalPosition() const;
		float getLocalScale() const;
		quatf getLocalOrientation() const;
		World* getParentWorld() const;
		std::string getName() const;
		AxisAlignedBox getBoundingBox() const;
		uint64_t getFlags() const;
		//

		//	Hierarchy
		void addChild(Entity* child);

		template<typename Visitor>
		bool recursiveChildVisitor(Visitor&& visitor)
		{
			for (Entity* child : m_childs)
			{
				if ((visitor)(child) || child->recursiveChildVisitor(visitor))
					return true;
			}
			return false;
		}
		//

		//	Debug
		bool drawImGui(World& world);
		//

	private:
		//	Helpers
		void recomputeWorldBoundingBox();
		void recomputeWorldChildTransforms();
		//

		//	Attributes
		World* m_parentWorld;
		std::atomic<uint32_t> m_refCount;
		vec4f m_worldPosition, m_localPosition;
		quatf m_worldOrientation, m_localOrientation;
		float m_worldScale, m_localScale;
		bool m_transformIsDirty;
		mat4f m_transform;
		mat4f m_invTransform;
		std::string m_name;
		uint64_t m_flags;

		AxisAlignedBox m_localBoundingBox;
		AxisAlignedBox m_worldBoundingBox;
		
		Entity* m_parentEntity;
		std::vector<Entity*> m_childs;
		//

#ifdef USE_IMGUI
		bool m_isDebugSelected = false;
		bool m_showAllFlags = false;
		bool m_showTransform = false;
		bool m_showHierarchyTransform = false;
		bool m_drawBoundingBox = false;
#endif // USE_IMGUI
};


