#include "SkeletonLoader.h"

#include <iostream>

#include <Utiles/Parser/Reader.h>
#include <Resources/Skeleton.h>



bool SkeletonLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
    //	initialization
    Variant v; Variant* tmp = nullptr;
    try
    {
        Reader::parseFile(v, getFileName(resourceDirectory, fileName));
        tmp = &(v.getMap().begin()->second);
    }
    catch(std::exception&)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
            std::cerr << "ERROR : loading skeleton : " << fileName << " : fail to open file" << std::endl;
        return false;
    }

    //	import data
    Joint joint;
    try
    {
		Variant& skeletonMap = *tmp;

        //	Prevent parsing errors
        if(skeletonMap.getMap().find("order") == skeletonMap.getMap().end())
            throw std::runtime_error("no order array defined");
        if(skeletonMap.getMap().find("jointList") == skeletonMap.getMap().end())
            throw std::runtime_error("no joint array defined");

        for(unsigned int i = 0; i < skeletonMap.getMap()["order"].getArray().size(); i++)
        {
            //	extract name & currentvariant alias
            joint.sons.clear();
            joint.name = skeletonMap.getMap()["order"].getArray()[i].toString();
            Variant& jointVariant = skeletonMap.getMap()["jointList"].getMap()[joint.name];

            //	extract relative bind position
            if(jointVariant.getMap().find("relativeBindTransform") == jointVariant.getMap().end())
                throw std::runtime_error("no relative transform defined for at least one joint");
            for(int j = 0; j < 4; j++)
                for(int k = 0; k < 4; k++)
                    joint.relativeBindTransform[j][k] = (float) jointVariant.getMap()["relativeBindTransform"].getArray()[4 * j + k].toDouble();

            //	search and extract parent index
            try
            {
                std::string parentName = jointVariant.getMap()["parent"].toString();
                for(unsigned int j = 0; j < skeletonMap.getMap()["order"].getArray().size(); j++)
                    if(skeletonMap.getMap()["order"].getArray()[j].toString() == parentName)
                    {
                        joint.parent = (int) j;
                        break;
                    }
            }
            catch(std::exception&)
            {
                roots.push_back(i);
                joint.parent = -1;
            }

            //	search and extract all sons index
            try
            {
                for(unsigned int j = 0; j < jointVariant.getMap()["sons"].getArray().size(); j++)
                {
                    std::string sonName = jointVariant.getMap()["sons"].getArray()[j].toString();
                    for(unsigned int k = 0; k < skeletonMap.getMap()["order"].getArray().size(); k++)
                        if(skeletonMap.getMap()["order"].getArray()[k].toString() == sonName)
                        {
                            joint.sons.push_back(k);
                            break;
                        }
                }
            }
            catch(std::exception&) {}

            //	add successfully parsed joint to list
            joints.push_back(joint);
        }
    }
    catch(std::exception& e)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
            std::cerr << "ERROR : loading shader : " << fileName << " : " << e.what() << std::endl;
        roots.clear();
        joints.clear();
        return false;
    }
    if(roots.empty() || joints.empty())
    {
        roots.clear();
        joints.clear();
        return false;
    }

    return true;
}

void SkeletonLoader::initialize(ResourceVirtual* resource)
{
    Skeleton* skeleton = static_cast<Skeleton*>(resource);
    skeleton->initialize(std::move(roots), std::move(joints));
}

void SkeletonLoader::getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList)
{}

std::string SkeletonLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += Skeleton::directory;
    str += fileName;
    str += Skeleton::extension;
    return str;
}

