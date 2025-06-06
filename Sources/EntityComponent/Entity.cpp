#include "Entity.hpp"
#include "Utiles/Assert.hpp"
#include "Physics/Shapes/Collider.h"
#include "Renderer/DrawableComponent.h"
#include "Renderer/Lighting/LightComponent.h"
#include "Animation/SkeletonComponent.h"
#include "Terrain/TerrainAreaDrawableComponent.h"
#include "Terrain/TerrainDetailDrawableComponent.h"
#include "World/World.h"

#include <Utiles/Debug.h>


//	Default
Entity::Entity() : m_refCount(0), m_name("unknown"), m_parentWorld(nullptr), 
	m_worldPosition(0.f), m_localPosition(0.f), m_worldOrientation(quatf::identity), m_localOrientation(quatf::identity), 
	m_worldScale(1.f), m_localScale(1.f), m_transformIsDirty(false), m_transform(1.f), m_normalMatrix(1.f), m_invTransform(1.f), m_flags(0)
{
	m_parentEntity = nullptr;
}
Entity::~Entity()
{
	if (m_parentEntity)
		m_parentEntity->removeChild(this, true);
	if (m_parentWorld)
	{
		removeAllChild(true);
		m_parentWorld->removeFromScene(this);
	}
}
//


void Entity::addComponent(Component* component, ClassID type)
{
	GF_ASSERT(component->getParentEntity() == nullptr, "Bad m_parentEntity entity. A component can't have multiple m_parentEntity entities.");
	EntityBase::addComponent(component, type);
	component->onAddToEntity(this);
	GF_ASSERT(component->getParentEntity() == this, "Bad m_parentEntity entity. You should call Component::onAddToEntity when reimplementing the method.");
}

void Entity::removeComponent(Component* component)
{
	GF_ASSERT(component->getParentEntity() == this, "Bad m_parentEntity entity. The component has a different m_parentEntity entity than the one he's beeing removed.");
	EntityBase::removeComponent(component);
	component->onRemoveFromEntity(this);
	GF_ASSERT(component->getParentEntity() == nullptr, "Bad m_parentEntity entity. You should call Component::onRemoveFromEntity when reimplementing the method.");
}


std::string Entity::getFlagName(uint64_t flag)
{
	bool first = true;
	std::string result = "";

#ifdef USE_IMGUI
#define FLAG_MACRO(name,value)    \
	if (flag&(##value)){            \
		if (!first) result += '+';\
		first = false;			  \
		result += "Fl_"#name;	  \
	}\

#include "EntityFlags.h"
#undef FLAG_MACRO
#endif
	return result;
}


//	Set/Get functions
void Entity::setWorldPosition(const vec4f& position)
{
	m_worldPosition = position;
	m_worldPosition.w = 1.f;
	if (m_parentEntity)
	{
		m_localPosition = (vec4f::one / m_parentEntity->m_worldScale) * (conjugate(m_parentEntity->m_worldOrientation) * (position - m_parentEntity->m_worldPosition));
		m_localPosition.w = 1.f;
	}
	else
		m_localPosition = m_worldPosition;

	m_transformIsDirty = true;
	recomputeWorldBoundingBox();
	recomputeWorldChildTransforms();
}
void Entity::setWorldScale(const vec4f& scale)
{
	m_worldScale = scale;
	if (m_parentEntity)
		m_localScale = scale / m_parentEntity->m_worldScale;

	m_transformIsDirty = true;
	recomputeWorldBoundingBox();
	recomputeWorldChildTransforms();
}
void Entity::setWorldOrientation(const quatf& orientation)
{
	m_worldOrientation = orientation;
	m_worldOrientation.normalize();
	if (m_parentEntity)
	{
		m_localOrientation = conjugate(m_parentEntity->m_worldOrientation) * orientation;
		m_localOrientation.normalize();
	}
	else
		m_localOrientation = m_worldOrientation;

	m_transformIsDirty = true;
	recomputeWorldBoundingBox();
	recomputeWorldChildTransforms();
}
void Entity::setWorldTransformation(const vec4f& position, const vec4f& scale, const quatf& orientation)
{
	m_worldPosition = position;
	m_worldOrientation = orientation;
	m_worldScale = scale;
	m_worldPosition.w = 1.f;
	m_worldOrientation.normalize();

	if (m_parentEntity)
	{
		vec4f invScale = vec4f::one / m_parentEntity->m_worldScale;
		m_localPosition = invScale * (conjugate(m_parentEntity->m_worldOrientation) * (position - m_parentEntity->m_worldPosition));
		m_localPosition.w = 1.f;
		m_localOrientation = conjugate(m_parentEntity->m_worldOrientation) * orientation;
		m_localOrientation.normalize();
		m_localScale = scale * invScale;
	}

	m_transformIsDirty = true;
	recomputeWorldBoundingBox();
	recomputeWorldChildTransforms();
}
void Entity::setLocalPosition(const vec4f& position)
{
	m_localPosition = position;
	m_localPosition.w = 1.f;
	if (m_parentEntity)
		m_worldPosition = m_parentEntity->m_worldPosition + m_parentEntity->m_worldScale * (m_parentEntity->m_worldOrientation * position);
	else
		m_worldPosition = position;
	m_worldPosition.w = 1.f;

	m_transformIsDirty = true;
	recomputeWorldBoundingBox();
	recomputeWorldChildTransforms();
}
void Entity::setLocalScale(const vec4f& scale)
{
	m_localScale = scale;
	if (m_parentEntity)
		m_worldScale = m_parentEntity->m_worldScale * scale;
	else
		m_worldScale = scale;

	m_transformIsDirty = true;
	recomputeWorldBoundingBox();
	recomputeWorldChildTransforms();
}
void Entity::setLocalOrientation(const quatf& orientation)
{
	m_localOrientation = orientation;
	m_localOrientation.normalize();
	if (m_parentEntity)
		m_worldOrientation = m_parentEntity->m_worldOrientation * orientation;
	else
		m_worldOrientation = orientation;
	m_worldOrientation.normalize();

	m_transformIsDirty = true;
	recomputeWorldBoundingBox();
	recomputeWorldChildTransforms();
}
void Entity::setLocalTransformation(const vec4f& position, const vec4f& scale, const quatf& orientation)
{
	m_localPosition = position;
	m_localOrientation = orientation;
	m_localScale = scale;
	m_localPosition.w = 1.f;
	m_localOrientation.normalize();

	if (m_parentEntity)
	{
		m_worldPosition = m_parentEntity->m_worldPosition + m_parentEntity->m_worldScale * (m_parentEntity->m_worldOrientation * position);
		m_worldPosition.w = 1.f;
		m_worldOrientation = m_parentEntity->m_worldOrientation * orientation;
		m_worldScale = m_parentEntity->m_worldScale * scale;
	}
	else
	{
		m_worldPosition = position;
		m_worldPosition.w = 1.f;
		m_worldOrientation = orientation;
		m_worldScale = scale;
	}

	m_transformIsDirty = true; 
	recomputeWorldBoundingBox();
	recomputeWorldChildTransforms();
}
void Entity::touchTransform()
{
	m_transformIsDirty = true;
}
void Entity::setParentWorld(World* parentWorld)
{
	m_parentWorld = parentWorld;
}
void Entity::recomputeBoundingBox()
{
	bool firstshape = true;
	const auto AddBox = [&](const AxisAlignedBox& box)
	{
		if (firstshape)
			m_localBoundingBox = box;
		else m_localBoundingBox.add(box);
		firstshape = false;
	};
	const auto visitor = [&](EntityBase::Element& element)
	{
		if (element.type == Collider::getStaticClassID())
		{
			const Collider* collider = static_cast<const Collider*>(element.comp);
			AddBox(collider->m_shape->toAxisAlignedBox());
		}
		else if (element.type == SkeletonComponent::getStaticClassID())
		{
			const SkeletonComponent* skeleton = static_cast<const SkeletonComponent*>(element.comp);
			AddBox(skeleton->getBoundingBox());
		}
		else if (element.type == TerrainAreaDrawableComponent::getStaticClassID())
		{
			const TerrainAreaDrawableComponent* area = static_cast<const TerrainAreaDrawableComponent*>(element.comp);
			AddBox(area->getBoundingBox());
		}
		else if (element.type == TerrainDetailDrawableComponent::getStaticClassID())
		{
			const TerrainDetailDrawableComponent* area = static_cast<const TerrainDetailDrawableComponent*>(element.comp);
			AddBox(area->getBoundingBox());
		}
		else if (element.comp->isIdInHierarchy(DrawableComponent::getStaticClassID()))
		{
			const DrawableComponent* drawable = static_cast<const DrawableComponent*>(element.comp);
			AddBox(drawable->getMesh()->getBoundingBox());
		}
		else if (element.type == LightComponent::getStaticClassID())
		{
			const LightComponent* light = static_cast<const LightComponent*>(element.comp);
			AddBox(light->getBoundingBox());
		}
		return false;
	};
	allComponentsVisitor(visitor);

	if (firstshape)
		m_localBoundingBox.min = m_localBoundingBox.max = vec4f(0, 0, 0, 1);
	m_localBoundingBox.min.w = 1.f;
	m_localBoundingBox.max.w = 1.f;

	recomputeWorldBoundingBox();
}
void Entity::setName(const std::string& _name) { m_name = _name; }
Entity* Entity::getParent() { return m_parentEntity; }
std::vector<Entity*>& Entity::getChilds() { return m_childs; }


uint64_t Entity::getId() const { return reinterpret_cast<uintptr_t>(this); }
const mat4f& Entity::getWorldTransformMatrix()
{ 
	if (m_transformIsDirty)
	{
		m_transform = mat4f::TRS(m_worldPosition, m_worldOrientation, m_worldScale);
		m_invTransform = mat4f::inverse(m_transform);
		bool cw = m_worldScale.x * m_worldScale.y * m_worldScale.z >= 0.f;

		m_normalMatrix = m_invTransform;
		m_normalMatrix[3] = vec4f(0, 0, 0, 1);
		m_normalMatrix = mat4f::transpose(m_normalMatrix);

		/*mat4f adj(1.f);
		adj[0] = vec4f::cross(m_transform[1], m_transform[2]);
		adj[1] = vec4f::cross(m_transform[2], m_transform[0]);
		adj[2] = vec4f::cross(m_transform[0], m_transform[1]);
		m_normalMatrix = (cw ? 1.f : -1.f) * adj;*/

		const auto visitor = [&](EntityBase::Element& element)
		{
			if (element.comp->isIdInHierarchy(DrawableComponent::getStaticClassID()))
			{
				DrawableComponent* drawable = static_cast<DrawableComponent*>(element.comp);
				drawable->setClockWise(cw);
			}
			return false;
		};
		allComponentsVisitor(visitor);

		m_transformIsDirty = false;

		for (int i = 0; i < m_childs.size(); i++)
		{
			m_childs[i]->touchTransform();
			m_childs[i]->getWorldTransformMatrix();
		}
	}

	return m_transform; 
}
const mat4f& Entity::getNormalMatrix()
{
	if (m_transformIsDirty)
		getWorldTransformMatrix();
	return m_normalMatrix;
}
const mat4f& Entity::getInverseWorldTransformMatrix()
{
	if (m_transformIsDirty)
		getWorldTransformMatrix();
	return m_invTransform;
}
vec4f Entity::getWorldPosition() const
{
	return m_worldPosition;
}
vec4f Entity::getWorldScale() const
{
	return m_worldScale;
}
quatf Entity::getWorldOrientation() const
{
	return m_worldOrientation;
}
vec4f Entity::getLocalPosition() const
{
	return m_localPosition;
}
vec4f Entity::getLocalScale() const
{
	return m_localScale;
}
quatf Entity::getLocalOrientation() const
{
	return m_localOrientation;
}
World* Entity::getParentWorld() const { return m_parentWorld; }
std::string Entity::getName() const { return m_name; }
AxisAlignedBox Entity::getBoundingBox() const { return m_worldBoundingBox; }


void Entity::setFlags(uint64_t _f) { m_flags |= _f; }
void Entity::clearFlags(uint64_t _f) { m_flags &= ~_f; }
uint64_t Entity::getFlags() const { return m_flags; }
//

// Hierarchy
void Entity::addChild(Entity* child, bool incOwnership)
{
	if (!child)
		return;
	m_childs.push_back(child);
	child->m_parentEntity = this;
	if (incOwnership && m_parentWorld)
		m_parentWorld->getOwnership(child);

}
void Entity::removeChild(Entity* child, bool removeFromSceneToo)
{
	int index = -1;
	for (int i = 0; i < m_childs.size(); i++)
	{
		if (m_childs[i] == child)
		{
			index = i;
			break;
		}
	}
	if (index >= 0)
	{
		if (m_parentWorld)
		{
			if (removeFromSceneToo)
				m_parentWorld->removeFromScene(child);
			m_parentWorld->releaseOwnership(child);
		}
		child->m_parentEntity = nullptr;
		int last = (int)m_childs.size() - 1;
		if (index != last)
			m_childs[index] = m_childs[last];
		m_childs.pop_back();
	}
}
void Entity::removeAllChild(bool removeFromSceneToo)
{
	if (m_parentWorld)
	{
		for (Entity* child : m_childs)
		{
			if (removeFromSceneToo)
				m_parentWorld->removeFromScene(child);
			m_parentWorld->releaseOwnership(child);
		}
	}
	m_childs.clear();
}
void Entity::recursiveHierarchyCollect(std::vector<Entity*>& collection)
{
	collection.push_back(this);
	for (auto& child : m_childs)
		child->recursiveHierarchyCollect(collection);
}
//

// Helpers
void Entity::recomputeWorldBoundingBox()
{
	m_worldBoundingBox = m_localBoundingBox;
	m_worldBoundingBox.transform(m_worldPosition, m_worldScale, m_worldOrientation);
}
void Entity::recomputeWorldChildTransforms()
{
	for (int i = 0; i < m_childs.size(); i++)
	{
		m_childs[i]->m_worldPosition = m_worldPosition + m_worldScale * (m_worldOrientation * m_childs[i]->m_localPosition);
		m_childs[i]->m_worldPosition.w = 1.f;
		m_childs[i]->m_worldOrientation = m_worldOrientation * m_childs[i]->m_localOrientation;
		m_childs[i]->m_worldOrientation.normalize();
		m_childs[i]->m_worldScale = m_worldScale * m_childs[i]->m_localScale;

		m_childs[i]->m_transformIsDirty = true;
		m_childs[i]->recomputeWorldBoundingBox();
		m_childs[i]->recomputeWorldChildTransforms();
	}
}
//

// Debug
bool Entity::drawImGui(World& world, bool inChildWindow)
{
#ifdef USE_IMGUI
	// new window
	ImGui::PushID(this);
	std::ostringstream unicName;
	unicName << m_name << "##" << (uintptr_t)this;
	if (inChildWindow)
	{
		ImGui::BeginChild(unicName.str().c_str());
		//ImGui::SeparatorText("Child windows");
	}
	else
	{
		
		ImGui::Begin(unicName.str().c_str(), &m_isDebugSelected);
	}

	// base
	ImGui::Text("Name : %s", m_name.c_str());
	ImGui::TextDisabled("Reference count : %d", m_refCount.load());
	ImGui::Checkbox("Draw boundingBox", &m_drawBoundingBox);

	// transform
	const ImVec4 sectionColor = ImVec4(1.f, 0.9f, 0.5f, 1.f);
	const auto ColoredTreeNode = [](ImVec4 color, const char* label, void* id, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		bool open = ImGui::TreeNodeEx(id, flags, label);
		ImGui::PopStyleColor();
		return open;
	};

	ImGui::Spacing();
	ImGui::Spacing();
	if (ColoredTreeNode(sectionColor, "Transform", (void*)1, ImGuiTreeNodeFlags_DefaultOpen))
	{
		constexpr float floatMax = std::numeric_limits<float>::max();
		constexpr float epsilon = std::numeric_limits<float>::epsilon();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "World");
		if (ImGui::DragFloat3("World position", &m_worldPosition[0], 0.1f, -floatMax, floatMax, "%.3f"))
			setWorldPosition(m_worldPosition);
		if (ImGui::DragFloat3("World scale", &m_worldScale[0], 0.1f, epsilon, floatMax, "%.3f"))
			setWorldScale(m_worldScale);
		/*vec3f euler = (float)RAD2DEG * glm::eulerAngles(m_worldOrientation);
		if (ImGui::DragFloat3("World orientation", &euler[0], 0.1f, -360.f, 360.f, "%.3f"))
		setWorldOrientation(quatf((float)DEG2RAD * euler));*/
		if (ImGui::DragFloat4("World orientation", &m_worldOrientation[0], 0.1f, -1.f, 1.f, "%.3f"))
			setWorldOrientation(m_worldOrientation);

		if (m_parentEntity)
		{
			ImGui::Spacing();
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Local");
			if (ImGui::DragFloat3("Local position", &m_localPosition[0], 0.1f, -floatMax, floatMax, "%.3f"))
				setLocalPosition(m_localPosition);
			if (ImGui::DragFloat3("Local scale", &m_localScale[0], 0.1f, epsilon, floatMax, "%.3f"))
				setLocalScale(m_localScale);
			/*euler = (float)RAD2DEG * glm::eulerAngles(m_localOrientation);
			if (ImGui::DragFloat3("Local orientation", &euler[0], 0.1f, -360.f, 360.f, "%.3f"))
				setLocalOrientation(quatf((float)DEG2RAD * euler));*/
			if (ImGui::DragFloat4("Local orientation", &m_localOrientation[0], 0.1f, -1.f, 1.f, "%.3f"))
				setLocalOrientation(m_localOrientation);
		}

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Gizmos");
		ImGui::Text("Distance to camera : %.2f", (Debug::view[3] - m_worldPosition).getNorm());
		ImGui::Checkbox("Show transform", &m_showTransform);
		ImGui::Checkbox("Show all hierarchy", &m_showHierarchyTransform);

		ImGui::TreePop();
	}

	// flags
	if (ColoredTreeNode(sectionColor, "Flags", (void*)2))
	{
		ImGui::Checkbox("Show all flags", &m_showAllFlags);

		if (m_showAllFlags)
		{
			bool boolean;
			#define FLAG_MACRO(name,value)                   \
				boolean = m_flags&(##value);	             \
				if (ImGui::Checkbox("Fl_"#name , &boolean)) {\
					if (boolean) m_flags |= (##value);       \
					else  m_flags &= ~(##value);             \
				}\

			#include "EntityFlags.h"
			#undef FLAG_MACRO
		}
		else
		{
			if (m_flags)
			{
				for (int i = 0; i < 64; i++)
				{
					uint64_t f = ((uint64_t)1 << i);
					if (m_flags & f)
						ImGui::Text(Entity::getFlagName(f).c_str());
				}
			}
			else
			{
				ImGui::Text("(none)");
			}
		}

		ImGui::TreePop();
	}

	// hierarchy
	if (!m_childs.empty())
	{
		if (ColoredTreeNode(sectionColor, "Childs", (void*)3))
		{
			for (const Entity* child : m_childs)
			{
				ImGui::Text(child->m_name.c_str());
			}
			ImGui::TreePop();
		}
	}

	// components
	if (getNbComponents() && ColoredTreeNode(sectionColor, "Components", (void*)4))
	{
		const auto DrawComp = [](EntityBase::Element& elem) { elem.comp->onDrawImGui(); return false; };
		allComponentsVisitor(DrawComp);
		ImGui::TreePop();
	}

	// end window
	if (inChildWindow)
	{
		ImGui::EndChild();
	}
	else
	{
		ImGui::End();
	}
	ImGui::PopID();

	const auto DrawTransform = [](vec4f p, quatf o)
	{
		Debug::setDepthTest(false);
		Debug::color = Debug::red;
		Debug::drawLine(p, p + o * vec4f(1, 0, 0, 0));
		Debug::color = Debug::green;
		Debug::drawLine(p, p + o * vec4f(0, 1, 0, 0));
		Debug::color = Debug::blue;
		Debug::drawLine(p, p + o * vec4f(0, 0, 1, 0));
		Debug::setDepthTest(true);
	};

	if (m_drawBoundingBox)
	{
		Debug::color = Debug::magenta;
		Debug::drawLineCube(mat4f::identity, m_worldBoundingBox.min, m_worldBoundingBox.max);
		Debug::color = Debug::orange;
		Debug::drawLineCube(getWorldTransformMatrix(), m_localBoundingBox.min, m_localBoundingBox.max);
	}
	if (m_showTransform)
	{
		DrawTransform(m_worldPosition, m_worldOrientation);
	}
	if (m_showHierarchyTransform)
	{
		Entity* prev = this;
		Entity* e = m_parentEntity;
		while (e)
		{
			DrawTransform(e->m_worldPosition, e->m_worldOrientation);
			Debug::color = Debug::magenta;
			Debug::drawLine(e->m_worldPosition, prev->m_worldPosition);

			prev = e;
			e = e->m_parentEntity;
		}

		DrawTransform(vec4f(0, 0, 0, 1), quatf(1, 0, 0, 0));
		Debug::color = Debug::magenta;
		Debug::drawLine(vec4f(0, 0, 0, 1), prev->m_worldPosition);
	}

	return !m_isDebugSelected;
#endif // USE_IMGUI
}
//