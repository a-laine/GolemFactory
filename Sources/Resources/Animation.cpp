#include "Animation.h"
#include "Utiles/Parser/Reader.h"

//  Static attributes
std::string Animation::extension = ".animation";
//

//  Default
Animation::Animation(const std::string& animationName, const std::vector<KeyFrame>& animations)
	: ResourceVirtual(animationName, ResourceVirtual::ANIMATION), timeLine(animations)
{}

Animation::Animation(const std::string& path, const std::string& animationName) : ResourceVirtual(animationName, ResourceVirtual::ANIMATION)
{
	//	initialization
	Variant v; Variant* tmp = nullptr;
	try
	{
		Reader::parseFile(v, path + animationName + extension);
		tmp = &(v.getMap().begin()->second);
	}
	catch (std::exception&){ return; }
	Variant& animationMap = *tmp;

	//	import data
	KeyFrame keyframe;
	JointPose jointpose;
	std::vector<std::string> errors;
	errors.push_back("");
	try
	{
		//	load first key frame
		errors.back() = "loading first key frame";
		{
			//	load time and alias
			errors.push_back("key time");
			Variant& key0 = animationMap.getMap()["keyFrameList"].getMap().begin()->second;
			
			keyframe.time = (float)key0.getMap()["time"].toDouble();

			//	load joints poses attributes
			for (auto it2 = key0.getMap().begin(); it2 != key0.getMap().end(); ++it2)
			{
				if (it2->first.find("jp") == 0)
				{
					//	joint priority
					errors.back() = "joint priority";
					if (it2->second.getMap().find("p") != it2->second.getMap().end())
						jointpose.priority = it2->second.getMap()["p"].toInt();
					else throw std::logic_error("");

					//	joint position
					errors.back() = "joint position";
					jointpose.position.x = (float)it2->second.getMap()["t"].getArray()[0].toDouble();
					jointpose.position.y = (float)it2->second.getMap()["t"].getArray()[1].toDouble();
					jointpose.position.z = (float)it2->second.getMap()["t"].getArray()[2].toDouble();

					//	joint scaling
					errors.back() = "joint scale";
					jointpose.scale.x = (float)it2->second.getMap()["s"].getArray()[0].toDouble();
					jointpose.scale.y = (float)it2->second.getMap()["s"].getArray()[1].toDouble();
					jointpose.scale.z = (float)it2->second.getMap()["s"].getArray()[2].toDouble();

					//	joint rotation
					errors.back() = "joint rotation";
					jointpose.rotation.x = (float)it2->second.getMap()["r"].getArray()[0].toDouble();
					jointpose.rotation.y = (float)it2->second.getMap()["r"].getArray()[1].toDouble();
					jointpose.rotation.z = (float)it2->second.getMap()["r"].getArray()[2].toDouble();
					jointpose.rotation.w = (float)it2->second.getMap()["r"].getArray()[3].toDouble();

					//	store joinpose in keyframe
					keyframe.poses.push_back(jointpose);
				}
			}
			timeLine.push_back(keyframe);
			errors.pop_back();
		}
		
		//	load all others key frame
		errors.back() = "loading other frames : ";
		for (auto it = ++animationMap.getMap()["keyFrameList"].getMap().begin(); it != animationMap.getMap()["keyFrameList"].getMap().end(); ++it)
		{
			//	load time and alias
			if (it->first.find("key") == std::string::npos) continue;
			keyframe.time = (float)it->second.getMap()["time"].toDouble();

			//	load joint poses attributes
			for (auto it2 = it->second.getMap().begin(); it2 != it->second.getMap().end(); ++it2)
			{
				if (it2->first.find("jp") == 0)
				{
					int jp = std::stoi(it2->first.substr(it2->first.find("jp") + 2));

					//	joint priority
					if (it2->second.getMap().find("p") != it2->second.getMap().end())
						keyframe.poses[jp].priority = it2->second.getMap()["p"].toInt();

					//	joint position
					try
					{
						keyframe.poses[jp].position.x = (float)it2->second.getMap()["t"].getArray()[0].toDouble();
						keyframe.poses[jp].position.y = (float)it2->second.getMap()["t"].getArray()[1].toDouble();
						keyframe.poses[jp].position.z = (float)it2->second.getMap()["t"].getArray()[2].toDouble();
					}
					catch (std::exception&) {}

					//	joint scaling
					try
					{
						keyframe.poses[jp].scale.x = (float)it2->second.getMap()["s"].getArray()[0].toDouble();
						keyframe.poses[jp].scale.y = (float)it2->second.getMap()["s"].getArray()[1].toDouble();
						keyframe.poses[jp].scale.z = (float)it2->second.getMap()["s"].getArray()[2].toDouble();
					}
					catch (std::exception&) {}

					//	joint rotation
					try
					{
						keyframe.poses[jp].rotation.x = (float)it2->second.getMap()["r"].getArray()[0].toDouble();
						keyframe.poses[jp].rotation.y = (float)it2->second.getMap()["r"].getArray()[1].toDouble();
						keyframe.poses[jp].rotation.z = (float)it2->second.getMap()["r"].getArray()[2].toDouble();
						keyframe.poses[jp].rotation.w = (float)it2->second.getMap()["r"].getArray()[3].toDouble();
					}
					catch (std::exception&) {}
				}
			}
			timeLine.push_back(keyframe);
		}
		
		//	load labels
		errors.back() = "loading labels";
		try
		{
			KeyLabel label;
			for (auto it = animationMap.getMap()["labelList"].getMap().begin(); it != animationMap.getMap()["labelList"].getMap().end(); ++it)
			{
				label.start = (unsigned int)it->second.getMap()["start"].toInt();
				label.stop = (unsigned int)it->second.getMap()["stop"].toInt();

				if (it->second.getMap().find("distortion") != it->second.getMap().end())
					label.distortion = (float)it->second.getMap()["distortion"].toDouble();
				else label.distortion = 1.f;
				try{ label.loop = it->second.getMap()["loop"].toBool(); }
				catch (std::exception&) { label.loop = false; }

				labels[it->first] = label;
			}
		}
		catch (std::exception&) {}
	}
	catch (std::exception&)
	{
		std::cerr << "ERROR : loading animation : " << animationName << " : ";
		for (unsigned int i = 0; i < errors.size(); i++)
			std::cerr << errors[i] << (i == errors.size() - 1 ? "" : " : ");
		std::cerr << std::endl;
		return;
	}
}
Animation::~Animation() {}
//

//	Public functions
bool Animation::isValid() const
{
	return !timeLine.empty();
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
std::vector<KeyFrame> Animation::getTimeLine() const { return timeLine; }
std::map<std::string, KeyLabel> Animation::getLabels() const { return labels; }
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