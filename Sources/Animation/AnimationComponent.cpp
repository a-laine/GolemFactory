#include "AnimationComponent.h"

#include <Resources/ResourceManager.h>
#include <EntityComponent/Entity.hpp>
#include <Resources/Skeleton.h>
#include <Animation/SkeletonComponent.h>
#include <Resources/AnimationClip.h>
#include <Resources/Mesh.h>
#include <Utiles/ProfilerConfig.h>
#include <Scene/SceneManager.h>
#include <World/World.h>



std::vector<AnimationComponent*> g_allAnimations;


AnimationComponent::AnimationComponent()
{
	m_speed = 1.f;
	m_currentTime = 0.f;
	m_running = false;
	m_looped = true;
	m_animation = nullptr;
	m_skeleton = nullptr;
	m_skeletonComponent = nullptr;
}

AnimationComponent::~AnimationComponent()
{
	ResourceManager::getInstance()->release(m_animation);
	ResourceManager::getInstance()->release(m_skeleton);
}

bool AnimationComponent::load(Variant& jsonObject, const std::string& objectName)
{
	const auto TryLoadAsFloat = [](Variant& variant, const char* label, float& destination)
	{
		if (variant.getMap().find(label) != variant.getMap().end())
		{
			auto& v = variant[label];
			if (v.getType() == Variant::FLOAT)
				destination = v.toFloat();
			else if (v.getType() == Variant::DOUBLE)
				destination = v.toDouble();
			else if (v.getType() == Variant::INT)
				destination = v.toInt();
			else
				return false;
			return true;
		}
		return false;
	};

	if (jsonObject.getType() == Variant::MAP)
	{
		std::string animationName;
		auto it1 = jsonObject.getMap().find("animationName");
		if (it1 != jsonObject.getMap().end() && it1->second.getType() == Variant::STRING)
			animationName = it1->second.toString();

		if (!animationName.empty())
		{
			setAnimation(animationName);
			
			it1 = jsonObject.getMap().find("looped");
			if (it1 != jsonObject.getMap().end() && it1->second.getType() == Variant::BOOL)
				m_looped = it1->second.toBool();

			TryLoadAsFloat(jsonObject, "speed", m_speed);


			m_currentTime = 0.f;
			it1 = jsonObject.getMap().find("randomStart");
			if (it1 != jsonObject.getMap().end() && it1->second.getType() == Variant::BOOL && it1->second.toBool() && m_animation)
			{
				srand((uintptr_t)this);
				int r = rand() % 10000;
				m_currentTime = 0.0001f * r * m_animation->m_duration;
			}

			resumeAnimation();
			return true;
		}
	}
	return false;
}

void AnimationComponent::save(Variant& jsonObject)
{

}

void AnimationComponent::setAnimation(std::string animationName)
{
	m_states.clear();
	m_skeletonFinalPose.clear();

	ResourceManager::getInstance()->release(m_animation);
	m_animation = ResourceManager::getInstance()->getResource<AnimationClip>(animationName);

	for (int i = 0; i < m_animation->m_boneCurves.size(); i++)
	{
		BoneCurvesState state;
		state.m_boneName = m_animation->m_boneCurves[i].m_boneName;
		state.m_posKey = state.m_rotKey = state.m_scaleKey = 0;
		//state.m_posSubTime = state.m_rotSubTime = state.m_scaleSubTime = 0.f;

		state.m_position = m_animation->m_boneCurves[i].m_positionCurve[0].m_value;
		const vec4f v = m_animation->m_boneCurves[i].m_rotationCurve[0].m_value;
		state.m_rotation = quatf(v.w, v.x, v.y, v.z);
		state.m_scale = m_animation->m_boneCurves[i].m_scaleCurve[0].m_value;
		state.m_skeletonBoneIndex = m_skeleton ? (m_skeleton->getBoneId(state.m_boneName)) : -1;

		m_states.push_back(state);
	}
	TryInitSkeletonPose();
}

void AnimationComponent::setAnimation(AnimationClip* animation)
{
	if (animation)
		setAnimation(animation->name);
	else
	{
		stopAnimation();
		ResourceManager::getInstance()->release(m_animation);
		m_animation = nullptr;
	}
}

AnimationClip* AnimationComponent::getCurrentAnimation() const
{
	return m_animation;
}


void AnimationComponent::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	if (entity)
	{
		m_skeletonComponent = entity->getComponent<SkeletonComponent>();
		if (m_skeletonComponent)
			m_skeleton = m_skeletonComponent->getSkeleton();
		g_allAnimations.push_back(this);
		TryInitSkeletonPose();
	}
}

bool AnimationComponent::isValid() const
{
	return m_animation && m_animation->isValid();
}

void AnimationComponent::startAnimation(float speed, bool loop) 
{
	m_speed = speed;
	m_looped = loop;
	m_currentTime = 0.f;
	resumeAnimation();
}

void AnimationComponent::resumeAnimation()
{
	if (m_animation && !m_states.empty())
		m_running = true;
}

void AnimationComponent::stopAnimation() 
{
	m_running = false;
}

bool AnimationComponent::isAnimationRunning()
{
	return m_running;
}
bool AnimationComponent::hasSkeletonAnimation() const
{
	return m_skeleton && !m_skeletonFinalPose.empty();
}
const std::vector<mat4f>& AnimationComponent::getSkeletonPose() const
{
	return m_skeletonFinalPose;
}








void AnimationComponent::update(float elapsedTime)
{
	SCOPED_CPU_MARKER("AnimationComponent");
	if (!m_running)
		return;

	float dt = m_speed * elapsedTime;
	m_currentTime += dt;
	const float duration = m_animation->m_duration;
	if (m_currentTime >= duration)
	{
		if (!m_looped)
		{
			m_currentTime = duration;
			return;
		}

		while (m_currentTime >= duration)
			m_currentTime -= duration;

		for (int i = 0; i < m_states.size(); i++)
		{
			BoneCurvesState& state = m_states[i];
			const AnimationClip::BoneCurves& curve = m_animation->m_boneCurves[i];

			for (int j = 1; j < curve.m_positionCurve.size(); j++)
			{
				if (curve.m_positionCurve[j].m_time > m_currentTime)
				{
					state.m_posKey = j - 1;
					break;
				}
			}
			for (int j = 1; j < curve.m_scaleCurve.size(); j++)
			{
				if (curve.m_scaleCurve[j].m_time > m_currentTime)
				{
					state.m_scaleKey = j - 1;
					break;
				}
			}
			for (int j = 1; j < curve.m_rotationCurve.size(); j++)
			{
				if (curve.m_rotationCurve[j].m_time > m_currentTime)
				{
					state.m_rotKey = j - 1;
					break;
				}
			}
		}
	}

	for (int i = 0; i < m_states.size(); i++)
	{
		BoneCurvesState& state = m_states[i];
		const AnimationClip::BoneCurves& curve = m_animation->m_boneCurves[i];
		
		// change curve keyframes if needed
		/*for (int k = 1; k < curve.m_positionCurve.size(); k++)
		{
			if (curve.m_positionCurve[k].m_time > m_currentTime)
			{
				state.m_posKey = k - 1;
				break;
			}
		}
		for (int k = 1; k < curve.m_scaleCurve.size(); k++)
		{
			if (curve.m_scaleCurve[k].m_time > m_currentTime)
			{
				state.m_scaleKey = k - 1;
				break;
			}
		}
		for (int k = 1; k < curve.m_rotationCurve.size(); k++)
		{
			if (curve.m_rotationCurve[k].m_time > m_currentTime)
			{
				state.m_rotKey = k - 1;
				break;
			}
		}*/
		if (m_currentTime > curve.m_positionCurve[state.m_posKey + 1].m_time)
		 {
			for (int j = state.m_posKey + 2; j < curve.m_positionCurve.size(); j++)
			{
				if (curve.m_positionCurve[j].m_time > m_currentTime)
				{
					state.m_posKey = j - 1;
					break;
				}
			}
		}
		if (m_currentTime > curve.m_rotationCurve[state.m_rotKey + 1].m_time)
		{
			for (int j = state.m_rotKey + 2; j < curve.m_rotationCurve.size(); j++)
			{
				if (curve.m_rotationCurve[j].m_time > m_currentTime)
				{
					state.m_rotKey = j - 1;
					break;
				}
			}
		}
		if (m_currentTime > curve.m_scaleCurve[state.m_scaleKey + 1].m_time)
		{
			for (int j = state.m_scaleKey + 2; j < curve.m_scaleCurve.size(); j++)
			{
				if (curve.m_scaleCurve[j].m_time > m_currentTime)
				{
					state.m_scaleKey = j - 1;
					break;
				}
			}
		}

		// evaluate segments
		float t = (m_currentTime - curve.m_positionCurve[state.m_posKey].m_time) / (curve.m_positionCurve[state.m_posKey + 1].m_time - curve.m_positionCurve[state.m_posKey].m_time);
		state.m_position = vec4f::lerp(curve.m_positionCurve[state.m_posKey].m_value, curve.m_positionCurve[state.m_posKey + 1].m_value, t);

		t = (m_currentTime - curve.m_rotationCurve[state.m_rotKey].m_time) / (curve.m_rotationCurve[state.m_rotKey + 1].m_time - curve.m_rotationCurve[state.m_rotKey].m_time);
		const quatf& v0 = *(const quatf*)&curve.m_rotationCurve[state.m_rotKey].m_value;
		const quatf& v1 = *(const quatf*)&curve.m_rotationCurve[state.m_rotKey + 1].m_value;
		state.m_rotation = quatf::slerp(v0, v1, t);

		t = (m_currentTime - curve.m_scaleCurve[state.m_scaleKey].m_time) / (curve.m_scaleCurve[state.m_scaleKey + 1].m_time - curve.m_scaleCurve[state.m_scaleKey].m_time);
		state.m_scale = lerp(curve.m_scaleCurve[state.m_scaleKey].m_value, curve.m_scaleCurve[state.m_scaleKey + 1].m_value, t);
	}

	computePoseMatrices();
	m_skeletonComponent->setPose(m_skeletonFinalPose);
	m_skeletonComponent->recomputeBoundingBox();
	if (getParentEntity())
	{
		getParentEntity()->recomputeBoundingBox();
		getParentEntity()->getParentWorld()->getSceneManager().updateObject(getParentEntity());
	}
}

void AnimationComponent::TryInitSkeletonPose()
{
	m_bones2state.clear();
	m_skeletonFinalPose.clear();
	if (!m_skeleton || m_states.empty())
		return;


	for (int i = 0; i < m_states.size(); i++)
	{
		m_states[i].m_skeletonBoneIndex = m_skeleton ? (m_skeleton->getBoneId(m_states[i].m_boneName)) : -1;
	}

	const int boneCount = m_skeleton->getBones().size();
	for (int i = 0; i < boneCount; i++)
	{
		int index = -1;
		for (int j = 0; j < m_states.size(); j++)
		{
			if (m_states[j].m_skeletonBoneIndex == i)
			{
				index = j;
				break;
			}
		}
		m_bones2state.push_back(index);
		m_skeletonFinalPose.push_back(mat4f::identity);
	}
	computePoseMatrices();
}

void AnimationComponent::computePoseMatrices()
{
	const std::vector<Skeleton::Bone>& bones = m_skeleton->m_bones;
	for (int i = 0; i < bones.size(); i++)
	{
		const mat4f& parent = bones[i].parent ? m_skeletonFinalPose[bones[i].parent->id] : mat4f::identity;
		mat4f trs;
		if (m_bones2state[i] >= 0)
		{
			const BoneCurvesState& state = m_states[m_bones2state[i]];
			trs = mat4f::TRS(state.m_position, state.m_rotation, vec4f(state.m_scale));
		}
		else 
			trs = bones[i].relativeBindTransform;
		m_skeletonFinalPose[i] = parent * trs;
	}
}

void AnimationComponent::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0.7, 0.7, 0.5, 1);
	std::ostringstream unicName;
	unicName << "Animation component##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(componentColor, "Animation");
		ImGui::Indent();
		ImGui::Text("Animation name : %s", m_animation->name.c_str());
		ImGui::Text("Animation curves count : %d", m_states.size());
		ImGui::DragFloat("Speed", &m_speed, 0.01f, 0.f, 100.f, "%.2f");
		if (ImGui::SliderFloat("Time", &m_currentTime, 0.f, m_animation->m_duration, "%.3f", ImGuiSliderFlags_AlwaysClamp))
		{
			for (int i = 0; i < m_states.size(); i++)
			{
				BoneCurvesState& state = m_states[i];
				const AnimationClip::BoneCurves& curve = m_animation->m_boneCurves[i];

				for (int j = 1; j < curve.m_positionCurve.size(); j++)
				{
					if (curve.m_positionCurve[j].m_time > m_currentTime)
					{
						state.m_posKey = j - 1;
						break;
					}
				}
				for (int j = 1; j < curve.m_scaleCurve.size(); j++)
				{
					if (curve.m_scaleCurve[j].m_time > m_currentTime)
					{
						state.m_scaleKey = j - 1;
						break;
					}
				}
				for (int j = 1; j < curve.m_rotationCurve.size(); j++)
				{
					if (curve.m_rotationCurve[j].m_time > m_currentTime)
					{
						state.m_rotKey = j - 1;
						break;
					}
				}
			}
		}
		ImGui::Checkbox("Running", &m_running);
		ImGui::Checkbox("Looped", &m_looped);
		ImGui::Unindent();

		ImGui::TreePop();
	}
#endif // USE_IMGUI
}
