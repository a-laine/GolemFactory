#include "Animator.h"

#include <EntityComponent/Entity.hpp>
#include <Animation/AnimationComponent.h>
#include <Animation/SkeletonComponent.h>


void Animator::animate(Entity* object, float step)
{
	AnimationComponent* animationComp = object->getComponent<AnimationComponent>();
	SkeletonComponent* skeletonComp = object->getComponent<SkeletonComponent>();
	if(!animationComp || !skeletonComp)
		return;

	if(!animationComp->getAnimation())
	{
		skeletonComp->initToBindPose();
		return;
	}

	animationComp->updateAnimations(step);

	if(!animationComp->isAnimationRunning())
		return;

	std::vector<JointPose> blendPose(skeletonComp->getNbJoints());
	animationComp->blendAnimations(blendPose);
	animationComp->cleanAnimationTracks(step);

	skeletonComp->computePose(blendPose);
}

void Animator::launchAnimation(Entity* object, const std::string& labelName, bool flaged)
{
	AnimationComponent* animationComp = object->getComponent<AnimationComponent>();
	SkeletonComponent* skeletonComp = object->getComponent<SkeletonComponent>();
	if(animationComp && skeletonComp)
		animationComp->launchAnimation(labelName, skeletonComp->getNbJoints(), flaged);
}

void Animator::stopAnimation(Entity* object, const std::string& labelName)
{
	AnimationComponent* animationComp = object->getComponent<AnimationComponent>();
	if(animationComp)
		animationComp->stopAnimation(labelName);
}

bool Animator::isAnimationRunning(Entity* object, const std::string& animationName)
{
	AnimationComponent* animationComp = object->getComponent<AnimationComponent>();
	if(animationComp)
		return animationComp->isAnimationRunning(animationName);
	return false;
}

