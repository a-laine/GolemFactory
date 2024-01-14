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
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : loading skeleton : " << fileName << " : fail to open file" << std::endl;
        return false;
    }
    Variant& skelMap = *tmp;
    if (skelMap.getType() != Variant::MAP)
    {
        PrintError(fileName.c_str(), "wrong file formating");
        return false;
    }

    //	import data
    const auto TryLoadAsVec4f = [](Variant& variant, const char* label, vec4f& destination)
    {
        int sucessfullyParsed = 0;
        auto it0 = variant.getMap().find(label);
        if (it0 != variant.getMap().end() && it0->second.getType() == Variant::ARRAY)
        {
            auto varray = it0->second.getArray();
            vec4f parsed = destination;
            for (int i = 0; i < 4 && i < varray.size(); i++)
            {
                auto& element = varray[i];
                if (element.getType() == Variant::FLOAT)
                {
                    parsed[i] = element.toFloat();
                    sucessfullyParsed++;
                }
                else if (element.getType() == Variant::DOUBLE)
                {
                    parsed[i] = (float)element.toDouble();
                    sucessfullyParsed++;
                }
                else if (element.getType() == Variant::INT)
                {
                    parsed[i] = (float)element.toInt();
                    sucessfullyParsed++;
                }
            }
            destination = parsed;
        }
        return sucessfullyParsed;
    };
    const auto TryLoadAsFloat = [](Variant& variant, const char* label, float& destination)
    {
        auto it0 = variant.getMap().find(label);
        if (it0 != variant.getMap().end())
        {
            if (it0->second.getType() == Variant::FLOAT)
                destination = it0->second.toFloat();
            else if (it0->second.getType() == Variant::DOUBLE)
                destination = (float)it0->second.toDouble();
            else if (it0->second.getType() == Variant::INT)
                destination = (float)it0->second.toInt();
            else return false;
            return true;
        }
        return false;
    };

    bones.clear();
    auto it0 = skelMap.getMap().find("bones");
    if (it0 != skelMap.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        auto& vbones = it0->second.getArray();
        int boneCount = 0;
        for (int i = 0; i < vbones.size(); i++)
        {
            if (vbones[i].getType() != Variant::MAP)
                continue;
            boneCount++;
        }
        bones.assign(boneCount, Skeleton::Bone());

        int index = 0;
        for (int i = 0; i < vbones.size(); i++)
        {
            if (vbones[i].getType() != Variant::MAP)
                continue;

            auto& vbone = vbones[i];

            //bones.push_back(Skeleton::Bone());
            Skeleton::Bone& bone = bones[index];
            bone.id = index;

            auto it1 = vbone.getMap().find("boneName");
            if (it1 != vbone.getMap().end() && it1->second.getType() == Variant::STRING)
                bone.name = it1->second.toString();

            float scale = 1.f;
            vec4f position = vec4f(0, 0, 0, 1);
            vec4f rotation = vec4f(0, 0, 0, 1);

            TryLoadAsFloat(vbone, "scale", scale);
            TryLoadAsVec4f(vbone, "pos", position);
            TryLoadAsVec4f(vbone, "rot", rotation);
            bone.relativeBindTransform = mat4f::TRS(position, quatf(rotation.w, rotation.x, rotation.y, rotation.z), vec4f(scale));

            std::string parentName;
            it1 = vbone.getMap().find("parentName");
            if (it1 != vbone.getMap().end() && it1->second.getType() == Variant::STRING)
                parentName = it1->second.toString();
            if (!parentName.empty())
            {
                for (int j = 0; j < bones.size() - 1; j++)
                {
                    if (bones[j].name == parentName)
                    {
                        bone.parent = &bones[j];
                        bones[j].sons.push_back(&bone);
                        break;
                    }
                }
            }

            index++;
        }
    }

    return !bones.empty();
}

void SkeletonLoader::initialize(ResourceVirtual* resource)
{
    Skeleton* skeleton = static_cast<Skeleton*>(resource);
    skeleton->initialize(bones);
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

