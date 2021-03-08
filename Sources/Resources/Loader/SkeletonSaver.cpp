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
	std::vector<Joint> jointList = skeleton->getJoints();
	for (unsigned int i = 0; i < jointList.size(); i++)
	{
		//	save skeleton joint vector order (!important)
		rootVariant.getMap()["order"].getArray().push_back(Variant(jointList[i].name));

		//	create joint variant
		rootVariant.getMap()["jointList"].insert(jointList[i].name, Variant::MapType());
		Variant& jointVariant = rootVariant.getMap()["jointList"].getMap()[jointList[i].name];

		//	create parent attribute
		if (jointList[i].parent >= 0)
			jointVariant.insert("parent", Variant(jointList[jointList[i].parent].name));

		//	create sons array attributes
		if (!jointList[i].sons.empty())
			jointVariant.insert("sons", Variant(Variant::ArrayType()));
		for (unsigned int j = 0; j < jointList[i].sons.size(); j++)
			jointVariant["sons"].getArray().push_back(Variant(jointList[jointList[i].sons[j]].name));

		//	create bind matrix array
		jointVariant.insert("relativeBindTransform", ToolBox::getFromMat4(jointList[i].relativeBindTransform));
	}

	//	save into file
	std::ofstream file(resourcesPath + "Skeletons/" + fileName + Skeleton::extension, std::ofstream::out);
	Writer writer(&file);
	file << std::fixed;
	file.precision(5);
	writer.setInlineArray(true);
	writer.write(rootVariant);
}
//