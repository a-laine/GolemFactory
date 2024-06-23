#include "SkeletonSaver.h"
#include <Utiles/Parser/Writer.h>
#include <Utiles/ToolBox.h>

//	Public functions
void SkeletonSaver::save(Skeleton* skeleton, const std::string& resourcesPath, std::string fileName)
{
	if (!skeleton) return;

	//	initialize fileName
	if (fileName.empty())
		fileName = skeleton->name;
	if (fileName.find_last_of('/') != std::string::npos)
		fileName = fileName.substr(fileName.find_last_of('/') + 1);
	if (fileName.find_first_of('.') != std::string::npos)
		fileName = fileName.substr(0, fileName.find_first_of('.'));

	//	clear buffers
	Variant rootVariant;   rootVariant.createMap();
	rootVariant.insert("jointList", Variant::MapType());
	rootVariant.insert("order", Variant::ArrayType());

	//	fill root variant structure
	const std::vector<Skeleton::Bone>& jointList = skeleton->getBones();
	for (unsigned int i = 0; i < jointList.size(); i++)
	{
		const Skeleton::Bone& bone = jointList[i];
		Variant& boneVariant = rootVariant.getMap()["boneList"].insert(bone.name, Variant::MapType());
		if (bone.parent)
			boneVariant.insert("parent", bone.parent->name);
		if (!bone.sons.empty())
		{
			auto& sons = boneVariant.insert("sons", Variant(Variant::ArrayType())).getArray();
			for (unsigned int j = 0; j < bone.sons.size(); j++)
				sons.push_back(Variant(bone.sons[j]->name));
			boneVariant.insert("relativeBindTransform", ToolBox::getFromMat4f(bone.relativeBindTransform));
		}
	}

	//	save into file
	std::ofstream file(resourcesPath + "Skeletons/" + fileName + Skeleton::extension, std::ofstream::out);
	Writer writer(&file);
	file << std::fixed;
	file.precision(5);
	writer.setInlineArray(false);
	writer.write(rootVariant);
}
//