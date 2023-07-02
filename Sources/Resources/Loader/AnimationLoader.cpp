#include "AnimationLoader.h"

#include <iostream>

#include <Utiles/Parser/Reader.h>
#include <Resources/AnimationClip.h>
#include <Utiles/ConsoleColor.h>



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

    Variant& animationMap = *tmp;
    if (animationMap.getType() != Variant::MAP)
    {
        PrintError(fileName.c_str(), "wrong file formating");
        return false;
    }

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
                    parsed[i] = element.toDouble();
                    sucessfullyParsed++;
                }
                else if (element.getType() == Variant::INT)
                {
                    parsed[i] = element.toInt();
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
                destination = it0->second.toDouble();
            else if (it0->second.getType() == Variant::INT)
                destination = it0->second.toInt();
            else return false;
            return true;
        }
        return false;
    };

    m_duration = -1.f;
    m_curves.clear();

    if (!TryLoadAsFloat(animationMap, "clipDuration", m_duration))
        PrintError(fileName.c_str(), "no clipDuration attribute found");

    auto it0 = animationMap.getMap().find("boneCurves");
    if (it0 != animationMap.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        auto& vcurvearray = it0->second.getArray();
        for (int i = 0; i < vcurvearray.size(); i++)
        {
            auto& vbonecurve = vcurvearray[i];
            if (vbonecurve.getType() == Variant::MAP)
            {
                AnimationClip::BoneCurves tmpCurve;
                tmpCurve.m_boneName.empty();

                auto it1 = vbonecurve.getMap().find("boneName");
                if (it1 != vbonecurve.getMap().end() && it1->second.getType() == Variant::STRING)
                    tmpCurve.m_boneName = it1->second.toString();

                it1 = vbonecurve.getMap().find("positions");
                if (it1 != vbonecurve.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto& vpositions = it1->second.getArray();
                    for (int j = 0; j < vpositions.size(); j++)
                    {
                        auto& element = vpositions[j];
                        if (element.getType() == Variant::MAP)
                        {
                            AnimationClip::Curve4DData data;
                            bool ok = TryLoadAsFloat(element, "t", data.m_time);
                            ok |= TryLoadAsVec4f(element, "pos", data.m_value);

                            if (ok)
                            {
                                data.m_value.w = 1.f;
                                tmpCurve.m_positionCurve.push_back(data);
                            }
                        }
                    }
                }

                it1 = vbonecurve.getMap().find("rotations");
                if (it1 != vbonecurve.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto& vrotations = it1->second.getArray();
                    for (int j = 0; j < vrotations.size(); j++)
                    {
                        auto& element = vrotations[j];
                        if (element.getType() == Variant::MAP)
                        {
                            AnimationClip::Curve4DData data;
                            bool ok = TryLoadAsFloat(element, "t", data.m_time);
                            ok |= TryLoadAsVec4f(element, "rot", data.m_value);

                            if (ok)
                                tmpCurve.m_rotationCurve.push_back(data);
                        }
                    }
                }

                it1 = vbonecurve.getMap().find("scales");
                if (it1 != vbonecurve.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto& vscales = it1->second.getArray();
                    for (int j = 0; j < vscales.size(); j++)
                    {
                        auto& element = vscales[j];
                        if (element.getType() == Variant::MAP)
                        {
                            AnimationClip::Curve1DData data;
                            bool ok = TryLoadAsFloat(element, "t", data.m_time);
                            ok |= TryLoadAsFloat(element, "scale", data.m_value);

                            if (ok)
                                tmpCurve.m_scaleCurve.push_back(data);
                        }
                    }
                }

                bool ok = true;
                if (tmpCurve.m_boneName.empty())
                {
                    ok = false;
                    PrintWarning(fileName.c_str(), "parsing didn't found a curve bone name");
                }
                if (tmpCurve.m_positionCurve.size() < 2)
                {
                    ok = false;
                    std::string msg = "positions curve not found (or has less than 2 data), for bone curve " + tmpCurve.m_boneName;
                    PrintWarning(fileName.c_str(), msg.c_str());
                }
                if (tmpCurve.m_rotationCurve.size() < 2)
                {
                    ok = false;
                    std::string msg = "rotations curve not found (or has less than 2 data), for bone curve " + tmpCurve.m_boneName;
                    PrintWarning(fileName.c_str(), msg.c_str());
                }
                if (tmpCurve.m_scaleCurve.size() < 2)
                {
                    ok = false;
                    std::string msg = "scales curve not found (or has less than 2 data), for bone curve " + tmpCurve.m_boneName;
                    PrintWarning(fileName.c_str(), msg.c_str());
                }

                if (ok)
                {
                    m_curves.push_back(tmpCurve);
                }
            }
        }
    }
    else PrintError(fileName.c_str(), "no boneCurves attribute found (or it's not an array)");

    bool valid = m_duration > 0.f && m_curves.size() > 0;
    if (!valid)
        PrintError(fileName.c_str(), "something went wrong parsing the file !");
    return valid;
}

void AnimationLoader::PrintError(const char* filename, const char* msg)
{
    if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
    {
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : AnimationLoader : " << filename << " : " << msg << std::flush;
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
    }
}

void AnimationLoader::PrintWarning(const char* filename, const char* msg)
{
    if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
    {
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << "WARNING : AnimationLoader : " << filename << " : " << msg << std::flush;
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
    }
}

void AnimationLoader::initialize(ResourceVirtual* resource)
{
    AnimationClip* animation = static_cast<AnimationClip*>(resource);
    animation->initialize(m_curves, m_duration);
}

void AnimationLoader::getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList)
{}

std::string AnimationLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += AnimationClip::directory;
    str += fileName;
    str += AnimationClip::extension;
    return str;
}

