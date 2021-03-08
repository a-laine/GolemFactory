#include "Animation.h"
#include "Loader/AnimationLoader.h"

#include <Utiles/Assert.hpp>

//  Static attributes
char const * const Animation::directory = "Animations/";
char const * const Animation::extension = ".animation";
std::string Animation::defaultName;
//

//  Default
Animation::Animation(const std::string& animationName) : ResourceVirtual(animationName) {}
Animation::~Animation() {}
//

//	Public functions
void Animation::initialize(const std::vector<KeyFrame>& animations)
{
    GF_ASSERT(state == INVALID);
    if(!animations.empty())
    {
        //state = LOADING;
        timeLine = animations;
        state = VALID;
    }
}

void Animation::initialize(std::vector<KeyFrame>&& animations)
{
    GF_ASSERT(state == INVALID);
    if(!animations.empty())
    {
        //state = LOADING;
        timeLine = std::move(animations);
        state = VALID;
    }
}

void Animation::initialize(const std::vector<KeyFrame>& animations, const std::map<std::string, KeyLabel>& names)
{
    GF_ASSERT(state == INVALID);
    if(!animations.empty())
    {
        //state = LOADING;
        timeLine = animations;
        labels = names;
        state = VALID;
    }
}

void Animation::initialize(std::vector<KeyFrame>&& animations, std::map<std::string, KeyLabel>&& names)
{
    GF_ASSERT(state == INVALID);
    if(!animations.empty())
    {
        //state = LOADING;
        timeLine = std::move(animations);
        labels = std::move(names);
        state = VALID;
    }
}

void Animation::clear()
{
    if(state == VALID)
    {
        //state = LOADING;
        labels.clear();
        timeLine.clear();
        state = INVALID;
    }
}

std::vector<glm::mat4> Animation::getKeyPose(const unsigned int& keyFrame, const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const
{
	std::vector<glm::mat4> pose(timeLine[0].poses.size(), glm::mat4(1.f));
	for (unsigned int i = 0; i < roots.size() && keyFrame < timeLine.size(); i++)
		computePose(keyFrame, pose, glm::mat4(1.f), roots[i], hierarchy);
	return pose;
}
std::pair<int, int> Animation::getBoundingKeyFrameIndex(float time) const
{
	int previous = -1;
	int next = -1;
	for (unsigned int i = 0; i < timeLine.size(); i++)
	{
		if (timeLine[i].time >= time)
		{
			previous = i - 1;
			next = i;
			break;
		}
	}
	return std::pair<int, int>(previous, next);
}
const std::vector<KeyFrame>& Animation::getTimeLine() const { return timeLine; }
const std::map<std::string, KeyLabel>& Animation::getLabels() const { return labels; }

std::string Animation::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}

std::string Animation::getIdentifier() const
{
    return getIdentifier(name);
}

std::string Animation::getLoaderId(const std::string& resourceName) const
{
    return extension;
}

const std::string& Animation::getDefaultName() { return defaultName; }
void Animation::setDefaultName(const std::string& name) { defaultName = name; }
//

//	Protected functions
void Animation::computePose(const unsigned int& keyFrame, std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy) const
{
	glm::mat4 t = glm::translate(timeLine[keyFrame].poses[joint].position);
	glm::mat4 r = glm::toMat4(timeLine[keyFrame].poses[joint].rotation);
	glm::mat4 s = glm::scale(timeLine[keyFrame].poses[joint].scale);

	pose[joint] = parentPose *t * r * s;
	for (unsigned int i = 0; i < hierarchy[joint].sons.size(); i++)
		computePose(keyFrame, pose, pose[joint], hierarchy[joint].sons[i], hierarchy);
}
//