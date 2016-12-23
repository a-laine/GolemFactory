#include "Skeleton.h"
#include "Utiles/Parser/Reader.h"

//  Static attributes
std::string Skeleton::extension = ".skeleton";
//

//	Default
Skeleton::Skeleton(std::string path, std::string skeletonName) : ResourceVirtual(skeletonName, ResourceVirtual::SKELETON), configuration(0x00)
{
	Variant v; Variant* tmp = NULL;
	std::string tmpName;

	try
	{
		Reader::parseFile(v, path + skeletonName + extension);
		tmp = &(v.getMap().begin()->second);
	}
	catch (std::exception&) {return;}
	Variant& skeletonMap = *tmp;

	//	first pass : create joint list
	for (auto it = skeletonMap.getMap().begin(); it != skeletonMap.getMap().end(); it++)
	{
		Joint j;
		j.name = it->first;

		//	import joint position
		try
		{
			Variant pos = it->second.getMap()["position"];
			j.position.x = (float) pos.getArray()[0].toDouble();
			j.position.y = (float) pos.getArray()[1].toDouble();
			j.position.z = (float) pos.getArray()[2].toDouble();
		}
		catch (std::exception&) { j.position = glm::vec3(0,0,0); }

		//	import joint orientation
		try
		{
			Variant ori = it->second.getMap()["orientation"];
			j.orientation.x = (float) ori.getArray()[0].toDouble();
			j.orientation.y = (float) ori.getArray()[1].toDouble();
			j.orientation.z = (float) ori.getArray()[2].toDouble();
			j.orientation.w = (float) ori.getArray()[3].toDouble();
		}
		catch (std::exception&) { j.orientation = glm::fquat(0,0,0,0); }

		//	add joint to joint list
		jointList.push_back(j);
	}

	//	second pass : construct hierarchy
	root = jointList.size() + 1;
	unsigned int i = 0;
	for (auto it = skeletonMap.getMap().begin(); it != skeletonMap.getMap().end(); it++, i++)
	{
		//	parse sons list
		try
		{
			Variant sons = it->second.getMap()["sons"];
			if (sons.getType() == Variant::ARRAY)
			{
				for (auto it2 = sons.getArray().begin(); it2 != sons.getArray().end(); it2++)
				{
					for (unsigned int k = 0; k < jointList.size(); k++)
					{
						if (jointList[k].name == (*it2).toString())
						{
							jointList[k].parent = i;
							jointList[i].sons.push_back(k);
							break;
						}
					}
				}
			}
			else
			{
				for (unsigned int k = 0; k < jointList.size(); k++)
				{
					if (jointList[k].name == sons.toString())
					{
						jointList[k].parent = i;
						jointList[i].sons.push_back(k);
						break;
					}
				}
			}
			jointList[i].sons.shrink_to_fit();
		}
		catch (std::exception&) { }

		//	identify root
		if(it->second.getMap()["parent"].isNull())
		{
			if (root > jointList.size())
				root = i;
			else return;
		}
	}

	if (root < jointList.size())
	{
		configuration |= VALID;
	}
}

Skeleton::~Skeleton()
{

}
//

//	Public functions
bool Skeleton::isValid() const
{
	return (configuration & VALID);
}
void Skeleton::debug()
{
	if(isValid()) printJoint(root, 0);
}
//

//	Protected functions
void Skeleton::printJoint(unsigned int joint, int depth)
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << jointList[joint].name << std::endl;

	for (unsigned int i = 0; i < jointList[joint].sons.size(); i++)
		printJoint(jointList[joint].sons[i],depth+1);
}
//