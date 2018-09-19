#include "AnimationEngine.h"


//  Default
AnimationEngine::AnimationEngine(const std::string& skeletonName, const std::string& animationName) : skeleton(nullptr), animation(nullptr)
{
	skeleton = ResourceManager::getInstance()->getSkeleton(skeletonName);
	animation = ResourceManager::getInstance()->getAnimation(animationName);
}
AnimationEngine::~AnimationEngine()
{
	ResourceManager::getInstance()->release(skeleton);
	ResourceManager::getInstance()->release(animation);
}/**/
//

//	Public functions
void AnimationEngine::animate(float step)
{
	if (!skeleton) return;
	if (animation)
	{
		//	update all current animations
		for (std::list<AnimationTrack>::iterator it = currentAnimations.begin(); it != currentAnimations.end();)
		{
			it->jointCounter = 0;
			if (it->animate(step, this))
			{
				if (it->flag) std::cout << "end animation " << it->name << std::endl;
				it = currentAnimations.erase(it);
			}
			else ++it;
		}
		if (currentAnimations.empty()) return;

		//	blend all animations
		std::vector<JointPose> blendPose(skeleton->joints.size());
		for (unsigned int i = 0; i < blendPose.size(); i++)
		{
			//	check for highest priority animation track
			float pl = -1.f; float ph = -1.f;
			std::list<AnimationTrack>::iterator pl_it = currentAnimations.begin();
			std::list<AnimationTrack>::iterator ph_it = currentAnimations.begin();
			for (std::list<AnimationTrack>::iterator it = currentAnimations.begin(); it != currentAnimations.end(); ++it)
			{
				if (it->pose[i].priority > ph)
				{
					pl = ph;
					ph = it->pose[i].priority;
					pl_it = ph_it;
					ph_it = it;
				}
			}

			//	interpolate between the two highest priority animation track or just get the highest one
			if (ph - (int)ph > 0.f && pl > 0.f && ph - pl < 1.f)
			{
				blendPose[i].position = glm::mix(pl_it->pose[i].position, ph_it->pose[i].position, ph - pl);
				blendPose[i].rotation = glm::slerp(pl_it->pose[i].rotation, ph_it->pose[i].rotation, ph - pl);
				blendPose[i].scale = glm::mix(pl_it->pose[i].scale, ph_it->pose[i].scale, ph - pl);
				pl_it->jointCounter++;
				ph_it->jointCounter++;
				pl_it->uselessTime = 0.f;
				ph_it->uselessTime = 0.f;
			}
			else if (ph_it != currentAnimations.end())
			{
				blendPose[i].position = ph_it->pose[i].position;
				blendPose[i].scale = ph_it->pose[i].scale;
				blendPose[i].rotation = ph_it->pose[i].rotation;
				ph_it->jointCounter++;
				ph_it->uselessTime = 0.f;
			}
		}

		//	remove useless animation track
		for (std::list<AnimationTrack>::iterator it = currentAnimations.begin(); it != currentAnimations.end();)
		{
			if (it->jointCounter == 0 && it->uselessTime >= 1.f) it = currentAnimations.erase(it);
			else if (it->jointCounter == 0)
			{
				it->uselessTime += step / 1000.f;
				++it;
			}
			else ++it;
		}

		//	compute matrix list pose
		std::vector<glm::mat4> blendMatrix(blendPose.size(), glm::mat4(1.f));
		for (unsigned int i = 0; i < skeleton->roots.size(); i++)
			computePose(blendMatrix, blendPose, glm::mat4(1.f), skeleton->roots[i]);

		//swap
		pose = blendMatrix;
	}
	else pose = skeleton->getBindPose();
}
void AnimationEngine::launchAnimation(const std::string& labelName, const bool& flaged)
{
	if (!animation || !skeleton) return;

	//	create and add a new track to current animation
	std::map<std::string, KeyLabel>::iterator it = animation->labels.find(labelName);
	if (it != animation->labels.end())
	{
		AnimationTrack at((unsigned int)skeleton->joints.size(), labelName);
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
void AnimationEngine::stopAnimation(const std::string& labelName)
{
	for (std::list<AnimationTrack>::iterator it = currentAnimations.begin(); it != currentAnimations.end(); ++it)
		if (it->name == labelName)
			it->loop = false;
}

void AnimationEngine::computeCapsules(const Mesh* mesh)
{
	if (!capsules.empty()) capsules.clear();
	if (!mesh || !skeleton || !mesh->getBones() || !mesh->getWeights()) return;

	//	get all lists
	std::vector<glm::mat4> ibind = skeleton->getInverseBindPose();
	const std::vector<glm::vec3>& vertices = *mesh->getVertices();
	const std::vector<glm::ivec3>* bones = mesh->getBones();
	const std::vector<glm::vec3>* weights = mesh->getWeights();
	std::vector<Joint> joints = skeleton->getJoints();
	if (ibind.empty() || pose.empty() || !bones || !weights || joints.empty()) return;;
	capsules.assign(skeleton->getJoints().size(), ShortCapsule());

	//	inflating bones
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		float maxDistance = 0.f;
		for (int j = 0; j < 3; j++)
		{
			int bone = (*bones)[i][j];
			glm::vec3 v = glm::vec3(ibind[bone] * glm::vec4(vertices[i], 1.f));
			if (joints[bone].sons.size() == 1)
			{
				glm::vec3 end = glm::vec3(joints[joints[bone].sons[0]].relativeBindTransform[3]);
				float d = glm::dot(v, end) / glm::length(end);
				if (d < 0.f) maxDistance += (*weights)[i][j] * glm::length(v);
				else if (d > 1.f) maxDistance += (*weights)[i][j] * glm::length(v - end);
				else maxDistance += (*weights)[i][j] * glm::length(v - d * glm::normalize(end));
			}
			else maxDistance += (*weights)[i][j] * glm::length(v);
		}
		for (int j = 0; j < 3; j++)
			capsules[(*bones)[i][j]].radius = std::max(capsules[(*bones)[i][j]].radius, maxDistance);
	}

	//	contruct capsule drawing buffers
	for (unsigned int i = 0; i < joints.size(); i++)
	{
		if (joints[i].sons.empty())
		{
			capsules[i].index1 = i;
			capsules[i].index2 = i;
		}
		else
		{
			for (unsigned int j = 0; j < joints[i].sons.size(); j++)
			{
				capsules[i].index1 = i;
				capsules[i].index2 = joints[i].sons[j];
			}
		}
	}
}
//

//	Set/get functions
bool AnimationEngine::isAnimationRunning(const std::string& animationName)
{
	for (std::list<AnimationTrack>::iterator it = currentAnimations.begin(); it != currentAnimations.end(); ++it)
		if (it->name == animationName && it->uselessTime <= 0.f) return true;
	return false;
}


void AnimationEngine::setSkeleton(std::string skeletonName)
{
	ResourceManager::getInstance()->release(skeleton);
	skeleton = ResourceManager::getInstance()->getSkeleton(skeletonName);

	if (animation && skeleton && pose.empty())
		pose = animation->getKeyPose(0, skeleton->roots, skeleton->joints);
}
void AnimationEngine::setSkeleton(Skeleton* s) { if (s) setSkeleton(s->name); }
void AnimationEngine::setAnimation(std::string animationName)
{
	currentAnimations.clear();
	ResourceManager::getInstance()->release(animation);
	animation = ResourceManager::getInstance()->getAnimation(animationName);

	if (animation && skeleton && pose.empty())
		pose = animation->getKeyPose(0, skeleton->roots, skeleton->joints);
}
void AnimationEngine::setAnimation(Animation* a) { if (a) setAnimation(a->name); };


Skeleton* AnimationEngine::getSkeleton() const { return skeleton; }
Animation* AnimationEngine::getAnimation() const { return animation; }
std::vector<glm::mat4> AnimationEngine::getPose() const { return pose; }
glm::vec3 AnimationEngine::getJointPosition(const std::string& jointName)
{
	if (!skeleton || pose.empty()) return  glm::vec3(0.f);
	int index = -1;
	for (unsigned int i = 0; i < skeleton->joints.size(); i++)
	{
		if (skeleton->joints[i].name == jointName)
		{
			index = i;
			break;
		}
	}
	if (index < 0) return  glm::vec3(0.f);
	return glm::vec3(pose[index][3]);
}
std::vector<AnimationEngine::ShortCapsule> AnimationEngine::getCapsules() const { return capsules; }
//

//	Protected functions
void AnimationEngine::computePose(std::vector<glm::mat4>& result, const std::vector<JointPose>& input, const glm::mat4& parentPose, unsigned int joint)
{
	result[joint] = parentPose * glm::translate(input[joint].position) * glm::toMat4(input[joint].rotation) * glm::scale(input[joint].scale);
	for (unsigned int i = 0; i < skeleton->joints[joint].sons.size(); i++)
		computePose(result, input, result[joint], skeleton->joints[joint].sons[i]);
}
//



//	Miscellaneous
AnimationEngine::AnimationTrack::AnimationTrack(const unsigned int& poseSize, const std::string& n)
	: name(n), start(0), stop(0), exit(0), previous(0), next(0), time(0.f), uselessTime(0.f), loop(false), flag(false), jointCounter(0)
{
	pose.assign(poseSize, JointPose());
}
bool AnimationEngine::AnimationTrack::animate(const float& step, const AnimationEngine* const parent)
{
	//	Increment time and create aliases
	time += step / 1000.f;
	const std::vector<KeyFrame>& animationSet = parent->animation->timeLine;
	float dt = animationSet[next].time - animationSet[previous].time;

	//	end of keyframes interpolation
	if (time > dt)
	{
		std::pair<int, int> bound = parent->animation->getBoundingKeyFrameIndex(animationSet[previous].time + time);
		time -= dt;
		if (loop && (bound.first >= stop || bound.first < 0))
		{
			if (loop) bound = parent->animation->getBoundingKeyFrameIndex(animationSet[start].time + time);
			else return true;
		}
		else if (!loop && (bound.first >= exit || bound.first < 0)) return true;
		previous = bound.first;
		next = bound.second;
	}

	//	interpolate joint parameters
	float t = time / (animationSet[next].time - animationSet[previous].time);
	for (unsigned int i = 0; i < pose.size(); i++)
	{
		pose[i].priority = glm::mix(animationSet[previous].poses[i].priority, animationSet[next].poses[i].priority, t);
		pose[i].position = glm::mix(animationSet[previous].poses[i].position, animationSet[next].poses[i].position, t);
		pose[i].rotation = glm::slerp(animationSet[previous].poses[i].rotation, animationSet[next].poses[i].rotation, t);
		pose[i].scale = glm::mix(animationSet[previous].poses[i].scale, animationSet[next].poses[i].scale, t);
	}
	return false;
}
//
