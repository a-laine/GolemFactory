#include "AnimationSaver.h"
#include "Utiles/Parser/Writer.h"

//	Public functions
void AnimationSaver::save(Animation* animation, const std::string& resourcesPath, std::string fileName)
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
			std::string jointName = "joint";
			{
				int maxDigit = (timeLine[i].poses.size() > 0 ? (int)log10((double)timeLine[i].poses.size()) + 1 : 1);
				int jointDigit = (j > 0 ? (int)log10((double)j) + 1 : 1);
				for (int k = 0; k < maxDigit - jointDigit; k++) jointName += '0';
				jointName += std::to_string(j);
			}

			//	create frame variant and export time
			frame.insert(jointName, Variant::MapType());
			Variant& joint = frame.getMap()[jointName];

			//	save joint position
			if (i == 0 || timeLine[i].poses[j].position != timeLine[i-1].poses[j].position)
			{
				joint.insert("p", Variant::ArrayType());
				for (int k = 0; k < 3; k++)
					joint["p"].getArray().push_back(Variant(timeLine[i].poses[j].position[k]));
			}

			//	save joint scale
			if (i == 0 || timeLine[i].poses[j].scale != timeLine[i - 1].poses[j].scale)
			{
				joint.insert("s", Variant::ArrayType());
				for (int k = 0; k < 3; k++)
					joint["s"].getArray().push_back(Variant(timeLine[i].poses[j].scale[k]));
			}

			//	save joint rotation
			if (i == 0 || timeLine[i].poses[j].rotation != timeLine[i - 1].poses[j].rotation)
			{
				joint.insert("r", Variant::ArrayType());
				for (int k = 0; k < 4; k++)
					joint["r"].getArray().push_back(Variant(timeLine[i].poses[j].rotation[k]));
			}
		}
	}

	//	save into file
	std::ofstream file(resourcesPath + "Animations/" + fileName + Animation::extension, std::ofstream::out);
	Writer writer(&file);
	file << std::fixed;
	file.precision(5);
	writer.setInlineArray(true);
	writer.setInlineEmptyMap(true);
	writer.write(rootVariant);
}
//