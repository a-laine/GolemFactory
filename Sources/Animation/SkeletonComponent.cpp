#include "SkeletonComponent.h"

#include <Resources/ResourceManager.h>
#include <Resources/Skeleton.h>
#include <Resources/Mesh.h>
#include <EntityComponent/Entity.hpp>
#include <Utiles/ConsoleColor.h>
#include <Utiles/Debug.h>
#include <functional>
#include <Renderer/DrawableComponent.h>


SkeletonComponent::SkeletonComponent() : m_skeleton(nullptr)
{
	skinnedBoundingRadius = -1.f;
}
SkeletonComponent::SkeletonComponent(const std::string& skeletonName)
{
	m_skeleton = ResourceManager::getInstance()->getResource<Skeleton>(skeletonName);
	if(m_skeleton)
		pose = m_skeleton->getBindPose();
}

SkeletonComponent::~SkeletonComponent()
{
	ResourceManager::getInstance()->release(m_skeleton);
}



bool SkeletonComponent::load(Variant& jsonObject, const std::string& objectName)
{
	if (jsonObject.getType() == Variant::MAP)
	{
		std::string skeletonName;
		auto it1 = jsonObject.getMap().find("skeletonName");
		if (it1 != jsonObject.getMap().end() && it1->second.getType() == Variant::STRING)
		{
			skeletonName = it1->second.toString();
		}

		if (!skeletonName.empty())
		{
			m_skeleton = ResourceManager::getInstance()->getResource<Skeleton>(skeletonName);
			if (m_skeleton)
				pose = m_skeleton->getBindPose();
			return true;
		}
		else
		{
			if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
			{
				if (skeletonName.empty())
				{
					std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : EntityFactory : " << objectName << " : SkeletonComponent loading : no skeleton name" << std::flush;
					std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
				}
			}
		}
	}
	return false;
}

void SkeletonComponent::save(Variant& jsonObject)
{

}


void SkeletonComponent::setSkeleton(std::string skeletonName)
{
	ResourceManager::getInstance()->release(m_skeleton);
	m_skeleton = ResourceManager::getInstance()->getResource<Skeleton>(skeletonName);

	locker.lock();
	pose.clear();
	locker.unlock();
}

void SkeletonComponent::setSkeleton(Skeleton* skeleton)
{
	ResourceManager::getInstance()->release(m_skeleton);
	if(skeleton) 
		m_skeleton = ResourceManager::getInstance()->getResource<Skeleton>(skeleton);
	else 
		m_skeleton = nullptr;

	locker.lock();
	pose.clear();
	locker.unlock();
}

Skeleton* SkeletonComponent::getSkeleton() const
{
	return m_skeleton;
}

unsigned int SkeletonComponent::getNbBones() const
{
    GF_ASSERT(isValid());
	return (unsigned int) m_skeleton->m_bones.size();
}

const std::vector<mat4f>& SkeletonComponent::getPose() const
{
	return pose;
}

void SkeletonComponent::setPose(const std::vector<mat4f>& _pose)
{
	pose = _pose;
}

void SkeletonComponent::swapPose(std::vector<mat4f>& _pose)
{
	pose = _pose;
}

const std::vector<mat4f>& SkeletonComponent::getInverseBindPose() const
{
    GF_ASSERT(isValid());
    return m_skeleton->getInverseBindPose();
}

vec4f SkeletonComponent::getBonePosition(const std::string& jointName)
{
    GF_ASSERT(isValid());
	vec4f result = vec4f::zero;

	locker.lock();
	for(unsigned int i = 0; i < m_skeleton->m_bones.size(); i++)
	{
		if(m_skeleton->m_bones[i].name == jointName)
		{
			//index = i;
			result = pose[i][3];
			break;
		}
	}
	locker.unlock();

	return result;
}

const AxisAlignedBox& SkeletonComponent::getBoundingBox() const
{
	return boundingBox;
}
void SkeletonComponent::recomputeBoundingBox()
{
	if (pose.empty())
	{
		boundingBox.min = boundingBox.max = vec4f(0.f, 0.f, 0.f, 1.f);
	}
	else
	{
		constexpr float maxValue = std::numeric_limits<float>::max();
		boundingBox.min = vec4f(maxValue);
		boundingBox.max = -boundingBox.min;

		for (int i = 0; i < pose.size(); i++)
		{
			vec4f v = pose[i][3];
			boundingBox.min = vec4f::min(boundingBox.min, v);
			boundingBox.max = vec4f::max(boundingBox.max, v);
		}
		boundingBox.min.w = boundingBox.max.w = 1.f;
	}

	if (skinnedBoundingRadius < 0.f && m_skeleton && getParentEntity())
	{
		DrawableComponent* drawable = getParentEntity()->getComponent<DrawableComponent>();
		if (drawable && drawable->getMesh() && drawable->getMesh()->hasSkeleton() && !pose.empty())
		{
			const auto& vertices = *drawable->getMesh()->getVertices();
			const auto& boneIds = *drawable->getMesh()->getBones();
			const auto& invBind = m_skeleton->getInverseBindPose();
			for (int i = 0; i < vertices.size(); i++)
			{
				vec4f p = invBind[boneIds[i].x] * vertices[i];
				vec4f v = pose[boneIds[i].x] * vec4f(p.x, p.y, p.z, 0);
				float d = v.getNorm2();

				if (boneIds[i].y >= 0)
				{
					p = invBind[boneIds[i].y] * vertices[i];
					v = pose[boneIds[i].y] * vec4f(p.x, p.y, p.z, 0);
					d = std::max(d, v.getNorm2());

					if (boneIds[i].z >= 0)
					{
						p = invBind[boneIds[i].z] * vertices[i];
						v = pose[boneIds[i].z] * vec4f(p.x, p.y, p.z, 0);
						d = std::max(d, v.getNorm2());

						if (boneIds[i].w >= 0)
						{
							p = invBind[boneIds[i].w] * vertices[i];
							v = pose[boneIds[i].w] * vec4f(p.x, p.y, p.z, 0);
							d = std::max(d, v.getNorm2());
						}
					}
				}

				skinnedBoundingRadius = std::max(d, skinnedBoundingRadius);
			}
			skinnedBoundingRadius = std::sqrt(skinnedBoundingRadius);
		}
	}

	float f = std::max(skinnedBoundingRadius, 0.3f);
	vec4f margin = vec4f(f, f, f, 0.f);
	boundingBox.min -= margin;
	boundingBox.max += margin;
}

bool SkeletonComponent::isValid() const
{
    return m_skeleton && m_skeleton->isValid();
}

void SkeletonComponent::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	entity->setFlags((uint64_t)Entity::Flags::Fl_Drawable | (uint64_t)Entity::Flags::Fl_Skinned);
}

static float frameLength = 4.f;
void SkeletonComponent::onDrawImGui()
{
#ifdef USE_IMGUI


	const ImVec4 componentColor = ImVec4(0.5, 0.5, 1, 1);
	std::ostringstream unicName;
	unicName << "Skeleton component##" << (uintptr_t)this;

	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(componentColor, "Skeleton");
		ImGui::Indent();
		ImGui::Text("name : %s", m_skeleton->name.c_str());
		ImGui::Text("bone count : %d", m_skeleton->m_bones.size());
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Gizmos");
		ImGui::Indent();
		ImGui::Checkbox("Draw skeleton", &m_drawSkeleton);
		ImGui::Checkbox("Draw boundingbox", &m_drawBoundingBox);
		ImGui::SliderFloat("Frame length", &frameLength, 0.f, 100.f, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::Unindent();

		ImGui::TreePop();
	}

	if (m_drawSkeleton && m_skeleton && pose.size() == getNbBones())
	{
		std::vector<Debug::Vertex> vertices;
		const std::vector<Skeleton::Bone>& joints = m_skeleton->m_bones;
		vertices.reserve(8 * m_skeleton->m_bones.size());
		vec4f color = vec4f(componentColor.x, componentColor.y, componentColor.z, 1.f);


		std::function<void(Skeleton::Bone*, mat4f)> RecursiveJointDraw = [&](Skeleton::Bone* bone, const mat4f& parentMatrix)
		{
			mat4f m = pose[bone->id];

			vertices.push_back({ parentMatrix[3], color });
			vertices.push_back({ m[3], color });
			vertices.push_back({ m[3], Debug::red });
			vertices.push_back({ m[3] + frameLength * m[0], Debug::red });
			vertices.push_back({ m[3], Debug::green });
			vertices.push_back({ m[3] + frameLength * m[1], Debug::green });
			vertices.push_back({ m[3], Debug::blue });
			vertices.push_back({ m[3] + frameLength * m[2], Debug::blue });

			for (int i = 0; i < bone->sons.size(); i++)
				RecursiveJointDraw(bone->sons[i], m);
		};

		const std::vector<Skeleton::Bone*>& roots = m_skeleton->getRoots();
		for (int j = 0; j < roots.size(); j++)
			RecursiveJointDraw(roots[j], mat4f::identity);

		Debug::setDepthTest(false);
		Debug::drawMultiplePrimitive(vertices.data(), vertices.size(), getParentEntity()->getWorldTransformMatrix(), GL_LINES);
		Debug::setDepthTest(true);
	}
	if (m_drawBoundingBox)
	{
		Debug::color = vec4f(0.6f * componentColor.x, 0.6f * componentColor.y, 0.6f * componentColor.z, 1.f);
		Debug::drawLineCube(getParentEntity()->getWorldTransformMatrix(), boundingBox.min, boundingBox.max);
	}
#endif // USE_IMGUI
}