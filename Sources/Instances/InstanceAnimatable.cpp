#include "InstanceAnimatable.h"

//  Default
InstanceAnimatable::InstanceAnimatable(std::string meshName, std::string shaderName) : InstanceDrawable(meshName, shaderName), skeleton(nullptr), animation(nullptr)
{
	type = InstanceVirtual::ANIMATABLE;

	mesh = ResourceManager::getInstance()->getMesh(meshName);
	shader = ResourceManager::getInstance()->getShader(shaderName);

	if (mesh->hasSkeleton()) skeleton = ResourceManager::getInstance()->getSkeleton(meshName); 
	if (mesh->isAnimable())  animation = ResourceManager::getInstance()->getAnimation(meshName);

	animationTime = 0.f;
	distortion = 1.f;

	startKeyFrame = 0;
	stopKeyFrame = (animation ? animation->timeLine.size() - 1 : startKeyFrame + 1);
	previousKeyFrame = startKeyFrame;
	nextKeyFrame = startKeyFrame + 1;

	animationConfiguration = PLAY | LOOPED;
}
InstanceAnimatable::~InstanceAnimatable()
{
	std::cout << "deleted instance" << std::endl;
	ResourceManager::getInstance()->release(mesh);
	ResourceManager::getInstance()->release(shader);
	ResourceManager::getInstance()->release(skeleton);
	ResourceManager::getInstance()->release(animation);
}
//

//	Public functions
void InstanceAnimatable::animate(float step)
{
	if (!skeleton) return;
	if (!(animationConfiguration & PLAY)) return;

	if (animation)
	{
		animationTime += distortion * step / 1000.f;
		std::pair<int, int> bound = animation->getBoundingKeyFrameIndex(animation->timeLine[previousKeyFrame].time + animationTime);
		if (bound.second != nextKeyFrame)
		{
			//std::cout << previousKeyFrame << ' ' << nextKeyFrame << ' ' << animation->timeLine.size() << std::endl;
			animationTime -= animation->timeLine[nextKeyFrame].time - animation->timeLine[previousKeyFrame].time;
			if (bound.first >= stopKeyFrame || bound.first < 0)
			{
				if (animationConfiguration&LOOPED)
					bound = animation->getBoundingKeyFrameIndex(animation->timeLine[startKeyFrame].time + animationTime);
				else
				{
					bound = std::pair<int, int>(startKeyFrame, startKeyFrame + 1);
					animationConfiguration &= ~PLAY;
				}
			}
		}
		previousKeyFrame = bound.first;
		nextKeyFrame = bound.second;
		if (!(animationConfiguration & PLAY)) return;

		std::vector<glm::mat4> newPose(animation->timeLine[0].poses.size(), glm::mat4(1.f));
		for (unsigned int i = 0; i < skeleton->roots.size(); i++)
			interpolatePose(previousKeyFrame, nextKeyFrame, animationTime / (animation->timeLine[nextKeyFrame].time - animation->timeLine[previousKeyFrame].time),
				newPose, glm::mat4(1.f), skeleton->roots[i], skeleton->joints);

		locker.lock();
		pose = newPose;
		locker.unlock();
	}
	else
	{
		locker.lock();
		pose = skeleton->getBindPose();
		locker.unlock();
	}
}
void InstanceAnimatable::launchAnimation(const std::string& labelName)
{
	if (!animation) return;
	std::map<std::string, KeyLabel>::iterator it = animation->labels.find(labelName);
	if (it != animation->labels.end())
	{
		animationTime = 0.f;
		distortion = it->second.distortion;

		startKeyFrame = it->second.start;
		stopKeyFrame = it->second.stop;
		previousKeyFrame = startKeyFrame;
		nextKeyFrame = previousKeyFrame + 1;

		animationConfiguration = PLAY;
		if (it->second.loop) animationConfiguration |= LOOPED;
	}
}
//

//	Set/get functions
void InstanceAnimatable::setAnimation(std::string animationName)
{
	ResourceManager::getInstance()->release(animation);
	animation = ResourceManager::getInstance()->getAnimation(animationName);

	startKeyFrame = 0;
	stopKeyFrame = (animation ? animation->timeLine.size() - 1 : startKeyFrame + 1);
}
void InstanceAnimatable::setAnimation(Animation* a)
{
	ResourceManager::getInstance()->release(animation);
	if (a) animation = ResourceManager::getInstance()->getAnimation(a->name);
	else animation = nullptr;
}

void InstanceAnimatable::setSkeleton(std::string skeletonName)
{
	ResourceManager::getInstance()->release(skeleton);
	skeleton = ResourceManager::getInstance()->getSkeleton(skeletonName);
}
void InstanceAnimatable::setSkeleton(Skeleton* s)
{
	ResourceManager::getInstance()->release(skeleton);
	if (s) skeleton = ResourceManager::getInstance()->getSkeleton(s->name);
	else skeleton = nullptr;
}

Skeleton* InstanceAnimatable::getSkeleton() const { return skeleton; }
Animation* InstanceAnimatable::getAnimation() const { return animation; }
std::vector<glm::mat4> InstanceAnimatable::getPose()
{
	std::vector<glm::mat4> p;
	locker.lock();
	p = pose;
	locker.unlock();
	return p;
}
//

//	Private functions
void InstanceAnimatable::interpolatePose(const int& key1, const int& key2,const float& p, std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy)
{
	glm::mat4 t = glm::translate(glm::mix(animation->timeLine[key1].poses[joint].position, animation->timeLine[key2].poses[joint].position, p));
	glm::mat4 s = glm::scale(glm::mix(animation->timeLine[key1].poses[joint].scale,    animation->timeLine[key2].poses[joint].scale,    p));
	glm::mat4 r = glm::toMat4(glm::slerp(animation->timeLine[key1].poses[joint].rotation,           animation->timeLine[key2].poses[joint].rotation,                 p));

	pose[joint] = parentPose * t * r * s;
	for (unsigned int i = 0; i < hierarchy[joint].sons.size(); i++)
		interpolatePose(key1, key2, p, pose, pose[joint], hierarchy[joint].sons[i], hierarchy);
}
//