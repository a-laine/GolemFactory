#include "Skeleton.h"
#include "Utiles/Parser/Reader.h"

//  Static attributes
std::string Skeleton::extension = ".skeleton";
//

//	Default
Skeleton::Skeleton(const std::string& skeletonName, const std::vector<unsigned int>& rootsList, const std::vector<Joint>& jointsList)
	: ResourceVirtual(skeletonName, ResourceVirtual::SKELETON), roots(rootsList), joints(jointsList)
{
	//	compute bind pose and inverse bind pose matrix lists
	inverseBindPose.assign(joints.size(), glm::mat4(1.f));
	bindPose.assign(joints.size(), glm::mat4(1.f));
	for (unsigned int i = 0; i < roots.size(); i++)
		computeBindPose(glm::mat4(1.f), roots[i]);
}
Skeleton::Skeleton(const std::string& path, const std::string& skeletonName) : ResourceVirtual(skeletonName, ResourceVirtual::SKELETON)
{
	//	initialization
	Variant v; Variant* tmp = nullptr;
	try
	{
		Reader::parseFile(v, path + skeletonName + extension);
		tmp = &(v.getMap().begin()->second);
	}
	catch (std::exception&)
	{
		if (logVerboseLevel > 0)
			std::cerr << "ERROR : loading skeleton : " << skeletonName << " : fail to open file" << std::endl;
		return;
	}
	Variant& skeletonMap = *tmp;

	//	import data
	Joint joint;
	try
	{
		//	Prevent parsing errors
		if (skeletonMap.getMap().find("order") == skeletonMap.getMap().end())
			throw std::runtime_error("no order array defined");
		if (skeletonMap.getMap().find("jointList") == skeletonMap.getMap().end())
			throw std::runtime_error("no joint array defined");

		for (unsigned int i = 0; i < skeletonMap.getMap()["order"].getArray().size(); i++)
		{
			//	extract name & currentvariant alias
			joint.sons.clear();
			joint.name = skeletonMap.getMap()["order"].getArray()[i].toString();
			Variant& jointVariant = skeletonMap.getMap()["jointList"].getMap()[joint.name];

			//	extract relative bind position
			if (jointVariant.getMap().find("relativeBindTransform") == jointVariant.getMap().end())
				throw std::runtime_error("no relative transform defined for at least one joint");
			for (int j = 0; j < 4; j++)
				for (int k = 0; k < 4; k++)
					joint.relativeBindTransform[j][k] = (float)jointVariant.getMap()["relativeBindTransform"].getArray()[4 * j + k].toDouble();

			//	search and extract parent index
			try
			{
				std::string parentName = jointVariant.getMap()["parent"].toString();
				for (unsigned int j = 0; j < skeletonMap.getMap()["order"].getArray().size(); j++)
					if (skeletonMap.getMap()["order"].getArray()[j].toString() == parentName)
					{
						joint.parent = (int)j;
						break;
					}
			}
			catch (std::exception&)
			{
				roots.push_back(i);
				joint.parent = -1;
			}

			//	search and extract all sons index
			try
			{
				for (unsigned int j = 0; j < jointVariant.getMap()["sons"].getArray().size(); j++)
				{
					std::string sonName = jointVariant.getMap()["sons"].getArray()[j].toString();
					for (unsigned int k = 0; k < skeletonMap.getMap()["order"].getArray().size(); k++)
						if (skeletonMap.getMap()["order"].getArray()[k].toString() == sonName)
						{
							joint.sons.push_back(k);
							break;
						}
				}
			}
			catch (std::exception&) {}

			//	add successfully parsed joint to list
			joints.push_back(joint);
		}
	}
	catch (std::exception& e)
	{
		//std::cerr << "ERROR : loading skeleton : " << skeletonName << " : parser fail, check file manually to correct format" << std::endl;
		if (logVerboseLevel > 0)
			std::cerr << "ERROR : loading shader : " << skeletonName << " : " << e.what() << std::endl;
		roots.clear();
		joints.clear();
		return;
	}

	//	compute bind pose and inverse bind pose matrix lists
	inverseBindPose.assign(joints.size(), glm::mat4(1.f));
	bindPose.assign(joints.size(), glm::mat4(1.f));

	for (unsigned int i = 0; i < roots.size(); i++)
		computeBindPose(glm::mat4(1.f), roots[i]);
}

Skeleton::~Skeleton() {}
//

//	Public functions
bool Skeleton::isValid() const { return !(roots.empty() || joints.empty()); }

std::vector<glm::mat4> Skeleton::getInverseBindPose() const { return inverseBindPose; }
std::vector<glm::mat4> Skeleton::getBindPose() const { return bindPose; }
std::vector<Joint> Skeleton::getJoints() const { return joints; }
//

//	Private functions
void Skeleton::computeBindPose(const glm::mat4& parentPose, unsigned int joint)
{
	bindPose[joint] = parentPose * joints[joint].relativeBindTransform;
	inverseBindPose[joint] = glm::inverse(bindPose[joint]);
	
	for (unsigned int i = 0; i < joints[joint].sons.size(); i++)
		computeBindPose(bindPose[joint], joints[joint].sons[i]);
}
//