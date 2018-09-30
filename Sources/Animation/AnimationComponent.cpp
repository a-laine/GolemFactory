#include "AnimationComponent.h"

#include <Resources/ResourceManager.h>
#include <Resources/Skeleton.h>
#include <Resources/Animation.h>
#include <Resources/Mesh.h>



AnimationComponent::AnimationComponent(const std::string& animationName)
{
	m_animation = ResourceManager::getInstance()->getResource<Animation>(animationName);
}

AnimationComponent::~AnimationComponent()
{
	ResourceManager::getInstance()->release(m_animation);
}

void AnimationComponent::setAnimation(std::string animationName)
{
	currentAnimations.clear();
	ResourceManager::getInstance()->release(m_animation);
	m_animation = ResourceManager::getInstance()->getResource<Animation>(animationName);
}

void AnimationComponent::setAnimation(Animation* animation)
{
	currentAnimations.clear();
	ResourceManager::getInstance()->release(m_animation);
	if(animation)
		m_animation = ResourceManager::getInstance()->getResource<Animation>(animation);
	else
		m_animation = nullptr;
}

Animation* AnimationComponent::getAnimation() const
{
	return m_animation;
}

bool AnimationComponent::isValid() const
{
    return m_animation && m_animation->isValid();
}

void AnimationComponent::launchAnimation(const std::string& labelName, unsigned int nbJoints, const bool& flaged)
{
	GF_ASSERT(isValid());

	//	create and add a new track to current animation
	std::map<std::string, KeyLabel>::iterator it = m_animation->labels.find(labelName);
	if(it != m_animation->labels.end())
	{
		AnimationTrack at(nbJoints, labelName);
		at.start = it->second.start;
		at.stop = it->second.stop;
		at.exit = it->second.exit_key;
		at.loop = it->second.loop;
		at.flag = flaged;
		at.previous = it->second.entry_key;
		at.next = at.previous + 1;
		currentAnimations.insert(currentAnimations.end(), at);
	}
}

void AnimationComponent::stopAnimation(const std::string& labelName)
{
	for(std::list<AnimationTrack>::iterator it = currentAnimations.begin(); it != currentAnimations.end(); ++it)
		if(it->animationName == labelName)
			it->loop = false;
}

bool AnimationComponent::isAnimationRunning()
{
	return !currentAnimations.empty();
}

bool AnimationComponent::isAnimationRunning(const std::string& animationName)
{
	for(std::list<AnimationTrack>::iterator it = currentAnimations.begin(); it != currentAnimations.end(); ++it)
		if(it->animationName == animationName && it->uselessTime <= 0.f)
			return true;
	return false;
}

void AnimationComponent::updateAnimations(float step)
{
    GF_ASSERT(isValid());
	for(std::list<AnimationTrack>::iterator it = currentAnimations.begin(); it != currentAnimations.end();)
	{
		it->jointCounter = 0;
		if(it->animate(step, this))
		{
			if(it->flag) std::cout << "end animation " << it->animationName << std::endl;
			it = currentAnimations.erase(it);
		}
		else ++it;
	}
}

void AnimationComponent::blendAnimations(std::vector<JointPose>& result)
{
	for(unsigned int i = 0; i < result.size(); i++)
	{
		//	check for highest priority animation track
		float pl = -1.f; float ph = -1.f;
		std::list<AnimationTrack>::iterator pl_it = currentAnimations.begin();
		std::list<AnimationTrack>::iterator ph_it = currentAnimations.begin();
		for(std::list<AnimationTrack>::iterator it = currentAnimations.begin(); it != currentAnimations.end(); ++it)
		{
			if(it->pose[i].priority > ph)
			{
				pl = ph;
				ph = it->pose[i].priority;
				pl_it = ph_it;
				ph_it = it;
			}
		}

		//	interpolate between the two highest priority animation track or just get the highest one
		if(ph - (int) ph > 0.f && pl > 0.f && ph - pl < 1.f)
		{
			result[i].position = glm::mix(pl_it->pose[i].position, ph_it->pose[i].position, ph - pl);
			result[i].rotation = glm::slerp(pl_it->pose[i].rotation, ph_it->pose[i].rotation, ph - pl);
			result[i].scale = glm::mix(pl_it->pose[i].scale, ph_it->pose[i].scale, ph - pl);
			pl_it->jointCounter++;
			ph_it->jointCounter++;
			pl_it->uselessTime = 0.f;
			ph_it->uselessTime = 0.f;
		}
		else if(ph_it != currentAnimations.end())
		{
			result[i].position = ph_it->pose[i].position;
			result[i].scale = ph_it->pose[i].scale;
			result[i].rotation = ph_it->pose[i].rotation;
			ph_it->jointCounter++;
			ph_it->uselessTime = 0.f;
		}
	}
}

void AnimationComponent::cleanAnimationTracks(float step)
{
	for(std::list<AnimationTrack>::iterator it = currentAnimations.begin(); it != currentAnimations.end();)
	{
		if(it->jointCounter == 0 && it->uselessTime >= 1.f) it = currentAnimations.erase(it);
		else if(it->jointCounter == 0)
		{
			it->uselessTime += step/1000.f;
			++it;
		}
		else ++it;
	}
}



AnimationComponent::AnimationTrack::AnimationTrack(const unsigned int& poseSize, const std::string& animation)
	: animationName(animation)
	, start(0), stop(0), exit(0), previous(0), next(0)
	, time(0.f), uselessTime(0.f)
	, loop(false)
	, flag(false)
	, jointCounter(0)
{
	pose.assign(poseSize, JointPose());
}

bool AnimationComponent::AnimationTrack::animate(const float& step, const AnimationComponent* const parent)
{
	//	Increment time and create aliases
	time += step / 1000.f;
	const std::vector<KeyFrame>& animationSet = parent->m_animation->timeLine;
	float dt = animationSet[next].time - animationSet[previous].time;

	//	end of keyframes interpolation
	if(time > dt)
	{
		std::pair<int, int> bound = parent->m_animation->getBoundingKeyFrameIndex(animationSet[previous].time + time);
		time -= dt;
		if(loop && (bound.first >= stop || bound.first < 0))
		{
			if(loop)
				bound = parent->m_animation->getBoundingKeyFrameIndex(animationSet[start].time + time);
			else
				return true;
		}
		else if(!loop && (bound.first >= exit || bound.first < 0))
			return true;
		previous = bound.first;
		next = bound.second;
	}

	//	interpolate joint parameters
	float t = time / (animationSet[next].time - animationSet[previous].time);
	for(unsigned int i = 0; i < pose.size(); i++)
	{
		pose[i].priority = glm::mix(animationSet[previous].poses[i].priority, animationSet[next].poses[i].priority, t);
		pose[i].position = glm::mix(animationSet[previous].poses[i].position, animationSet[next].poses[i].position, t);
		pose[i].rotation = glm::slerp(animationSet[previous].poses[i].rotation, animationSet[next].poses[i].rotation, t);
		pose[i].scale = glm::mix(animationSet[previous].poses[i].scale, animationSet[next].poses[i].scale, t);
	}
	return false;
}

