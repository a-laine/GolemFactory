#include "AnimationSaver.h"
#include <Utiles/Parser/Writer.h>
#include <Utiles/ToolBox.h>

//	Public functions
/*void AnimationSaver::save(Animation* animation, const std::string& resourcesPath, std::string fileName)
{
	if (!animation) return;

	//	initialize fileName
	if (fileName.empty())
		fileName = animation->name;
	if (fileName.find_last_of('/') != std::string::npos)
		fileName = fileName.substr(fileName.find_last_of('/') + 1);
	if (fileName.find_first_of('.') != std::string::npos)
		fileName = fileName.substr(0, fileName.find_first_of('.'));

	//	clear buffers
	Variant rootVariant;
	rootVariant.createMap();

	//	export key frames array
	rootVariant.insert("keyFrameList", Variant::MapType());
	std::vector<KeyFrame> timeLine = animation->getTimeLine();
	for (unsigned int i = 0; i < timeLine.size(); i++)
	{
		//	create frame name
		std::string frameName = "key";
		{
			int maxDigit = (timeLine.size() > 0 ? (int)log10((double)timeLine.size()) + 1 : 1);
			int frameDigit = (i > 0 ? (int)log10((double)i) + 1 : 1);
			for (int j = 0; j < maxDigit - frameDigit; j++) frameName += '0';
			frameName += std::to_string(i);
		}

		//	create frame variant and export time
		rootVariant.getMap()["keyFrameList"].insert(frameName, Variant::MapType());
		Variant& frame = rootVariant.getMap()["keyFrameList"].getMap()[frameName];
		frame.insert("time", Variant(timeLine[i].time));

		//	export JointPose
		for (unsigned int j = 0; j < timeLine[i].poses.size(); j++)
		{
			//	create joint name
			std::string jointName = "jp";
			{
				int maxDigit = (timeLine[i].poses.size() > 0 ? (int)log10((double)timeLine[i].poses.size()) + 1 : 1);
				int jointDigit = (j > 0 ? (int)log10((double)j) + 1 : 1);
				for (int k = 0; k < maxDigit - jointDigit; k++) jointName += '0';
				jointName += std::to_string(j);
			}

			//	create frame variant and export time
			frame.insert(jointName, Variant::MapType());
			Variant& joint = frame.getMap()[jointName];
			int insertion = 0;

			//	save joint priority
			if (i == 0 || epsilon(timeLine[i].poses[j].priority, timeLine[i - 1].poses[j].priority, 0.001f))
			{
				insertion++;
				joint.insert("p", (int)timeLine[i].poses[j].priority);
			}

			//	save joint position
			if (i == 0 || epsilon(timeLine[i].poses[j].position, timeLine[i - 1].poses[j].position, 0.001f))
			{
				insertion++;
				joint.insert("t", ToolBox::getFromVec4f(timeLine[i].poses[j].position));
			}

			//	save joint scale
			if (i == 0 || epsilon(timeLine[i].poses[j].scale, timeLine[i - 1].poses[j].scale, 0.001f))
			{
				insertion++;
				joint.insert("s", ToolBox::getFromVec4f(timeLine[i].poses[j].scale));
			}

			//	save joint rotation
			if (i == 0 || epsilon(timeLine[i].poses[j].rotation, timeLine[i - 1].poses[j].rotation, 0.001f))
			{
				insertion++;
				joint.insert("r", ToolBox::getFromQuatf(timeLine[i].poses[j].rotation));
			}

			//
			if (!insertion) frame.getMap().erase(jointName);
		}
	}

	//	export labels
	rootVariant.insert("labelList", Variant::MapType());
	std::map<std::string, KeyLabel> labels = animation->getLabels();
	for (std::map<std::string, KeyLabel>::iterator it = labels.begin(); it != labels.end(); ++it)
	{
		//	create label variant
		rootVariant.getMap()["labelList"].insert(it->first, Variant::MapType());
		Variant& label = rootVariant.getMap()["labelList"].getMap()[it->first];

		//	save label attributes
		label.insert("start", Variant((int)it->second.start));
		label.insert("stop", Variant((int)it->second.stop));
		if (it->second.loop) label.insert("loop", Variant(it->second.loop));
		if (it->second.entry_key != it->second.start) label.insert("entry", Variant((int)it->second.entry_key));
		if (it->second.exit_key != it->second.stop) label.insert("exit", Variant((int)it->second.exit_key));
	}

	//	save into file
	std::ofstream file(resourcesPath + "Animations/" + fileName + Animation::extension, std::ofstream::out);
	Writer writer(&file);
	file << std::fixed;
	file.precision(5);
	writer.setInlineArray(true);
	writer.write(rootVariant);
}*/
//

//	Protected functions
bool AnimationSaver::epsilon(const float& a, const float& b, const float& e)
{
	return (a - b)*(a - b) > e;
}
bool AnimationSaver::epsilon(const vec4f& a, const vec4f& b, const float& e)
{
	return vec4f::dot(a - b, a - b) > e;
}
bool AnimationSaver::epsilon(const quatf& a, const quatf& b, const float& e)
{
	return epsilon(a.xyzw(), b.xyzw(), e);
}
//
