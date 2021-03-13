#include "AnimationLoader.h"

#include <iostream>

#include <Utiles/Parser/Reader.h>
#include <Resources/Animation.h>



bool AnimationLoader::load(const std::string& resourceDirectory, const std::string& fileName)
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
            std::cerr << "ERROR : loading animation : " << fileName << " : fail to open or parse file" << std::endl;
        return false;
    }

    //	import data
    KeyFrame keyframe;
    JointPose jointpose;
    std::vector<std::string> errors;
    errors.push_back("");
    try
    {
		Variant& animationMap = *tmp;
        //	load first key frame
        errors.back() = "loading first key frame";
        {
            //	load time and alias
            errors.push_back("key time");
            Variant& key0 = animationMap.getMap()["keyFrameList"].getMap().begin()->second;

            keyframe.time = (float) key0.getMap()["time"].toDouble();

            //	load joints poses attributes
            for(auto it2 = key0.getMap().begin(); it2 != key0.getMap().end(); ++it2)
            {
                if(it2->first.find("jp") == 0)
                {
                    //	joint priority
                    errors.back() = "joint priority";
                    if(it2->second.getMap().find("p") != it2->second.getMap().end())
                        jointpose.priority = (float) it2->second.getMap()["p"].toInt();
                    else throw std::logic_error("");

                    //	joint position
                    errors.back() = "joint position";
                    jointpose.position.x = (float) it2->second.getMap()["t"].getArray()[0].toDouble();
                    jointpose.position.y = (float) it2->second.getMap()["t"].getArray()[1].toDouble();
                    jointpose.position.z = (float) it2->second.getMap()["t"].getArray()[2].toDouble();

                    //	joint scaling
                    errors.back() = "joint scale";
                    jointpose.scale.x = (float) it2->second.getMap()["s"].getArray()[0].toDouble();
                    jointpose.scale.y = (float) it2->second.getMap()["s"].getArray()[1].toDouble();
                    jointpose.scale.z = (float) it2->second.getMap()["s"].getArray()[2].toDouble();

                    //	joint rotation
                    errors.back() = "joint rotation";
                    jointpose.rotation.x = (float) it2->second.getMap()["r"].getArray()[0].toDouble();
                    jointpose.rotation.y = (float) it2->second.getMap()["r"].getArray()[1].toDouble();
                    jointpose.rotation.z = (float) it2->second.getMap()["r"].getArray()[2].toDouble();
                    jointpose.rotation.w = (float) it2->second.getMap()["r"].getArray()[3].toDouble();

                    //	store joinpose in keyframe
                    keyframe.poses.push_back(jointpose);
                }
            }
            timeLine.push_back(keyframe);
            errors.pop_back();
        }

        //	load all others key frame
        errors.back() = "loading other frames : ";
        for(auto it = ++animationMap.getMap()["keyFrameList"].getMap().begin(); it != animationMap.getMap()["keyFrameList"].getMap().end(); ++it)
        {
            //	load time and alias
            if(it->first.find("key") == std::string::npos) continue;
            keyframe.time = (float) it->second.getMap()["time"].toDouble();

            //	load joint poses attributes
            for(auto it2 = it->second.getMap().begin(); it2 != it->second.getMap().end(); ++it2)
            {
                if(it2->first.find("jp") == 0)
                {
                    int jp = std::stoi(it2->first.substr(it2->first.find("jp") + 2));

                    //	joint priority
                    if(it2->second.getMap().find("p") != it2->second.getMap().end())
                        keyframe.poses[jp].priority = (float) it2->second.getMap()["p"].toInt();

                    //	joint position
                    try
                    {
                        keyframe.poses[jp].position.x = (float) it2->second.getMap()["t"].getArray()[0].toDouble();
                        keyframe.poses[jp].position.y = (float) it2->second.getMap()["t"].getArray()[1].toDouble();
                        keyframe.poses[jp].position.z = (float) it2->second.getMap()["t"].getArray()[2].toDouble();
                    }
                    catch(std::exception&) {}

                    //	joint scaling
                    try
                    {
                        keyframe.poses[jp].scale.x = (float) it2->second.getMap()["s"].getArray()[0].toDouble();
                        keyframe.poses[jp].scale.y = (float) it2->second.getMap()["s"].getArray()[1].toDouble();
                        keyframe.poses[jp].scale.z = (float) it2->second.getMap()["s"].getArray()[2].toDouble();
                    }
                    catch(std::exception&) {}

                    //	joint rotation
                    try
                    {
                        keyframe.poses[jp].rotation.x = (float) it2->second.getMap()["r"].getArray()[0].toDouble();
                        keyframe.poses[jp].rotation.y = (float) it2->second.getMap()["r"].getArray()[1].toDouble();
                        keyframe.poses[jp].rotation.z = (float) it2->second.getMap()["r"].getArray()[2].toDouble();
                        keyframe.poses[jp].rotation.w = (float) it2->second.getMap()["r"].getArray()[3].toDouble();
                    }
                    catch(std::exception&) {}
                }
            }
            timeLine.push_back(keyframe);
        }

        //	load labels
        try
        {
            KeyLabel label;
            for(auto it = animationMap.getMap()["labelList"].getMap().begin(); it != animationMap.getMap()["labelList"].getMap().end(); ++it)
            {
                label.start = (unsigned int) it->second.getMap()["start"].toInt();
                label.stop = (unsigned int) it->second.getMap()["stop"].toInt();

                if(it->second.getMap().find("entry") != it->second.getMap().end())
                    label.entry_key = it->second.getMap()["entry"].toInt();
                else label.entry_key = label.start;

                if(it->second.getMap().find("exit") != it->second.getMap().end())
                    label.exit_key = it->second.getMap()["exit"].toInt();
                else label.exit_key = label.stop;

                if(it->second.getMap().find("loop") != it->second.getMap().end())
                    label.loop = it->second.getMap()["loop"].toBool();
                else label.loop = false;

                labels[it->first] = label;
            }
        }
        catch(std::exception&) {}
    }
    catch(std::exception&)
    {
        //	Print errors in Log file or output
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
        {
            std::cerr << "ERROR : loading animation : " << fileName << " : ";
            for(unsigned int i = 0; i < errors.size(); i++)
                std::cerr << errors[i] << (i == errors.size() - 1 ? "" : " : ");
            std::cerr << std::endl;
        }
    }

    if(timeLine.empty())
    {
        labels.clear();
        return false;
    }
    return true;
}

void AnimationLoader::initialize(ResourceVirtual* resource)
{
    Animation* animation = static_cast<Animation*>(resource);
    animation->initialize(std::move(timeLine), std::move(labels));
}

void AnimationLoader::getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList)
{}

std::string AnimationLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += Animation::directory;
    str += fileName;
    str += Animation::extension;
    return str;
}

