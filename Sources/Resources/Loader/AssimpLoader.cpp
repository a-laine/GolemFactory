#include "AssimpLoader.h"

#include <iostream>
#include <limits>

#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#pragma warning(pop)

#include <Utiles/Assert.hpp>
#include <Resources/Mesh.h>
#include <Resources/Skeleton.h>
//#include <Resources/AnimationClip.h>
#include <Utiles/ConsoleColor.h>


//  Default
AssimpLoader::AssimpLoader(ResourceType resourceToLoad) : firstResource(resourceToLoad)
{}
void AssimpLoader::PrintError(const char* filename, const char* msg)
{
    std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) <<    "ERROR   : AssimpLoader : " << filename << " : " << msg << std::flush;
    std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
}
void AssimpLoader::PrintWarning(const char* filename, const char* msg)
{
    std::cout << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << "WARNING : AssimpLoader : " << filename << " : " << msg << std::flush;
    std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
}
//

//  Public functions
bool AssimpLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
    clear();
    resourceName = fileName;

    //	import mesh using assimp
    std::string file = getFileName(resourceDirectory, fileName);
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
    if(!scene)
    {
        if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            PrintError(fileName.c_str(), "could not open file");
            std::cerr << "        : AssimpImporter log : " << importer.GetErrorString() << std::endl;
        return false;
    }

    auto mat = scene->mRootNode->mTransformation.Inverse();
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            globalMatrix[i][j] = mat[j][i];
    globalMatrix = mat4f(1.f);

    //
    unsigned int totalVertices = 0;
    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
        totalVertices += scene->mMeshes[i]->mNumVertices;
    if(totalVertices > std::numeric_limits<unsigned short>::max())
    {
        PrintError(fileName.c_str(), "too much vertex in file (not supported by engine)");
        return false;
    }


    //	usefull parameters for next
    unsigned int facesOffset = 0;
    vec4f meshColor;
    bool hasSkeleton = false;
    std::vector<int> meshVertexOffset;
    bool hasPrintedNotTriangle = false;
    bool hasPrintedMeshTooLarge = false;
    bool hasPrintedFaceIndexOutOfBound = false;
    bool hasPrintedNoUV = false;

    //if (fileName.find("Characters/Character_Skeleton_Soldier_02") != std::string::npos) { int toto = 0; }


    //	Load mesh
    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        //	import faces index
        aiMesh* mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            if (mesh->mFaces[j].mNumIndices != 3)
            {
                if (!hasPrintedNotTriangle)
                    PrintWarning(fileName.c_str(), "has not triangle faces (discarded)");
                hasPrintedNotTriangle = true;
                continue;
            }

            for(int k = 0; k < 3; k++)
                faces.push_back(mesh->mFaces[j].mIndices[k] + facesOffset);
        }
        facesOffset += mesh->mNumVertices;

        //	import vertex attributes
        meshVertexOffset.push_back((int)vertices.size());
        for(unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            aiVector3D pos = mesh->mVertices[j];
            vertices.push_back(vec4f(-pos.x, pos.y, pos.z, 1.f));
            //vertices.push_back(vec4f(pos.x, pos.y, pos.z, 1.f));
            if (std::abs(pos.x) > 10000.f || std::abs(pos.y) > 10000.f || std::abs(pos.z) > 10000.f)
            {
                if (!hasPrintedMeshTooLarge)
                    PrintWarning(fileName.c_str(), "at least one vertice is VERY large");
                hasPrintedMeshTooLarge = true;
            }

            aiVector3D normal = (mesh->HasNormals() ? mesh->mNormals[j] : aiVector3D(0.0f, 0.0f, 0.0f));
            normales.push_back(vec4f(-normal.x, normal.y, normal.z, 0.f));
            //normales.push_back(vec4f(normal.x, normal.y, normal.z, 0.f));

            aiVector3D uv = (mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][j] : aiVector3D(0.0f, 0.0f, 0.0f));
            uvs.push_back(vec4f(uv.x, 1 - uv.y, uv.z, 0.f));

            if (!hasPrintedNoUV && !mesh->HasTextureCoords(0))
            {
                hasPrintedNoUV = true;
                PrintWarning(fileName.c_str(), "No valid UV (maybe your mesh doesn't have materials)");
            }
        }

        //	demande a second pass to parse bone and skeleton
        if(mesh->HasBones()) hasSkeleton = true;
    }

    if (faces.empty())
    {
        PrintError(fileName.c_str(), "has no valid triangle");
        return false;
    }
    //  check that every faces are ok
    for (int i = 0; i < faces.size(); i++)
    {
        if (faces[i] < 0 || faces[i] >= vertices.size())
        {
            if (!hasPrintedFaceIndexOutOfBound)
                PrintWarning(fileName.c_str(), "face index out of bound");
            hasPrintedFaceIndexOutOfBound = true;
        }
    }

    //	Load skeleton
    if(hasSkeleton)
    {
        //	create bone map
        for(unsigned int i = 0; i < scene->mNumMeshes; i++)
            for(unsigned int j = 0; j < scene->mMeshes[i]->mNumBones; j++)
            {
                std::string boneName(scene->mMeshes[i]->mBones[j]->mName.C_Str());
                if(boneMap.find(boneName) == boneMap.end())
                {
                    joints.push_back(Skeleton::Bone());
                    joints.back().name = boneName;
                    joints.back().id = (int)joints.size() - 1;
                    joints.back().parent = nullptr;
                    joints.back().relativeBindTransform = mat4f::identity;
					boneMap[boneName] = (int)boneMap.size();
                }
            }

        //	import bone index & weight for each vertex
        bones.assign(vertices.size(), vec4i(-1, -1, -1, -1));
        weights.assign(vertices.size(), vec4f(0.f, 0.f, 0.f, 0.f));
        for(unsigned int i = 0; i < scene->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[i];
            for(unsigned int j = 0; j < mesh->mNumBones; j++)
            {
                for(unsigned int k = 0; k < mesh->mBones[j]->mNumWeights; k++)
                {
                    int vertexIndex = meshVertexOffset[i] + mesh->mBones[j]->mWeights[k].mVertexId;
                    float vertexWeight = mesh->mBones[j]->mWeights[k].mWeight;
                    std::string boneName(mesh->mBones[j]->mName.C_Str());

                    if(bones[vertexIndex].x < 0)
                    {
                        bones[vertexIndex].x = boneMap[boneName];
                        weights[vertexIndex].x = vertexWeight;
                    }
                    else if(bones[vertexIndex].y < 0)
                    {
                        bones[vertexIndex].y = boneMap[boneName];
                        weights[vertexIndex].y = vertexWeight;
                    }
                    else if(bones[vertexIndex].z < 0)
                    {
                        bones[vertexIndex].z = boneMap[boneName];
                        weights[vertexIndex].z = vertexWeight;
                    }
                    else if(bones[vertexIndex].w < 0)
                    {
                        bones[vertexIndex].w = boneMap[boneName];
                        weights[vertexIndex].w = vertexWeight;
                    }
                    else if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                    {
                        std::cerr << "ERROR : AssimpLoader : vertex at position " << vertices[vertexIndex].x << ' ' << vertices[vertexIndex].y << ' ' << vertices[vertexIndex].z;
                        std::cerr << ": more than 4 bones weights defined" << std::endl;
                    }
                }
            }
        }

        // replace negative value by 0 (for shader simplifaction)
        for(unsigned int i = 0; i < bones.size(); i++)
        {
            if (bones[i].x < 0)
            {
                std::cerr << "ERROR : AssimpLoader : a vertex has no bone id at index X" << std::endl;
            }
            if (weights[i].x <= 0.f)
            {
                std::cerr << "ERROR : AssimpLoader : a vertex has a weight of zero on bone X" << std::endl;
            }


            float sum = 0.f;
            for (int k = 0; k < 4; k++)
            {
                if (weights[i][k] > 0.f)
                    sum += weights[i][k];
            }
            if (sum > 0.f)
                weights[i] /= sum;

            //if(bones[i].x < 0) bones[i].x = 0;
            //if(bones[i].y < 0) bones[i].y = 0;
            //if(bones[i].z < 0) bones[i].z = 0;
            //if(bones[i].w < 0) bones[i].w = 0;
        }

        //	read scene hierarchy for importing skeleton
        if(scene->mRootNode) 
            readSceneHierarchy(scene->mRootNode);
    }

    //	Load animation
    for(unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
        // init
        BidirectionnalVectorMap positionBM;
        BidirectionnalVectorMap scaleBM;
        BidirectionnalQuaternionMap rotationBM;
        vec4f position, scale;
        quatf rotation;

        // extract array from assimp
        for(unsigned int j = 0; j < scene->mAnimations[i]->mNumChannels; j++)
        {
            aiNodeAnim* currentChannel = scene->mAnimations[i]->mChannels[j];
            std::string channelName(currentChannel->mNodeName.C_Str());

            // import translation vector
            for(unsigned int k = 0; k < currentChannel->mNumPositionKeys; k++)
            {
                position = vec4f(currentChannel->mPositionKeys[k].mValue.x,
                    currentChannel->mPositionKeys[k].mValue.y,
                    currentChannel->mPositionKeys[k].mValue.z, 1.f);
                positionBM.push_back(std::make_tuple((float) currentChannel->mPositionKeys[k].mTime, channelName, position));
            }

            // import rotation quaternion
            for(unsigned int k = 0; k < currentChannel->mNumRotationKeys; k++)
            {
                rotation = quatf(currentChannel->mRotationKeys[k].mValue.w,
                    currentChannel->mRotationKeys[k].mValue.x,
                    currentChannel->mRotationKeys[k].mValue.y,
                    currentChannel->mRotationKeys[k].mValue.z);
                rotationBM.push_back(std::make_tuple((float) currentChannel->mRotationKeys[k].mTime, channelName, rotation));
            }

            // import scaling vector
            for(unsigned int k = 0; k < currentChannel->mNumScalingKeys; k++)
            {
                scale = vec4f(currentChannel->mScalingKeys[k].mValue.x,
                    currentChannel->mScalingKeys[k].mValue.y,
                    currentChannel->mScalingKeys[k].mValue.z, 1.f);
                scaleBM.push_back(std::make_tuple((float) currentChannel->mScalingKeys[k].mTime, channelName, scale));
            }
        }

        // sort lists in time
        positionBM.sort([](tupleVec3 a, tupleVec3 b) { return std::get<float>(a) < std::get<float>(b); });
        scaleBM.sort([](tupleVec3 a, tupleVec3 b) { return std::get<float>(a) < std::get<float>(b); });
        rotationBM.sort([](tupleQuat a, tupleQuat b) { return std::get<float>(a) < std::get<float>(b); });

        // read lists in time and create keyframes
        /*KeyFrame currentKeyFrame;
        currentKeyFrame.time = 0.f;
        currentKeyFrame.poses.assign(joints.size(), JointPose());
        BidirectionnalVectorMap::iterator itPos = positionBM.begin();
        BidirectionnalVectorMap::iterator itSca = scaleBM.begin();
        BidirectionnalQuaternionMap::iterator itRot = rotationBM.begin();

        while(itPos != positionBM.end() || itSca != scaleBM.end() || itRot != rotationBM.end())
        {
            // counter for no more information for current time
            int increment = 0;

            // update current keyframe pose for skeleton (here position)
            if(itPos != positionBM.end() && std::get<float>(*itPos) <= currentKeyFrame.time)
            {
                currentKeyFrame.poses[boneMap[std::get<std::string>(*itPos)]].position = std::get<vec4f>(*itPos);
                ++itPos;
            }
            else increment++;

            // update current keyframe pose for skeleton (here rotation)
            if(itRot != rotationBM.end() && std::get<float>(*itRot) <= currentKeyFrame.time)
            {
                currentKeyFrame.poses[boneMap[std::get<std::string>(*itRot)]].rotation = std::get<quatf>(*itRot);

                ++itRot;
            }
            else increment++;

            // update current keyframe pose for skeleton (here scaling)
            if(itSca != scaleBM.end() && std::get<float>(*itSca) <= currentKeyFrame.time)
            {
                currentKeyFrame.poses[boneMap[std::get<std::string>(*itSca)]].scale = std::get<vec4f>(*itSca);
                ++itSca;
            }
            else increment++;

            if(increment == 3)
            {
                // push keyframe and increment currentKeyFrame time
                animations.push_back(currentKeyFrame);

                float t1 = FLT_MAX; if(itPos != positionBM.end()) t1 = std::get<float>(*itPos);
                float t2 = FLT_MAX; if(itRot != rotationBM.end()) t1 = std::get<float>(*itRot);
                float t3 = FLT_MAX; if(itSca != scaleBM.end())    t1 = std::get<float>(*itSca);

                currentKeyFrame.time = std::min(std::min(t1, t2), t3);
                if(currentKeyFrame.time == FLT_MAX) break;
            }
        }
        animations.push_back(currentKeyFrame);*/

        /*	Warnning :
        if multiple animation are attached to mesh they will be all stacked into "animations" attributes
        (time is discontinue when changing from one to another).
        maybe add some label to diferenciate them
        */
    }
    importer.FreeScene();
    return true;
}

void AssimpLoader::initialize(ResourceVirtual* resource)
{
    switch(firstResource)
    {
        case ResourceType::MESH:
        {
            Mesh* mesh = static_cast<Mesh*>(resource);
            mesh->initialize(vertices, normales, uvs, faces, bones, weights);
            if (!bones.empty() || !weights.empty())
            {
                std::vector<std::string> boneNames;
                for (int i = 0; i < joints.size(); i++)
                    boneNames.push_back(joints[i].name);
                mesh->setBoneNames(boneNames);
            }
            break;
        }
        case ResourceType::SKELETON:
        {
            Skeleton* skeleton = static_cast<Skeleton*>(resource);
            skeleton->initialize(joints);
            break;
        }
        /*case ResourceType::ANIMATION:
        {
            Animation* animation = static_cast<Animation*>(resource);
            animation->initialize(animations);
            break;
        }*/
        default:
            GF_ASSERT(0);
    }
}

std::string AssimpLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += Mesh::directory;
    str += fileName;
    return str;
}

bool AssimpLoader::isAssimpFileMesh(const std::string& fileName)
{
    size_t ext = fileName.find_last_of('.');
    return ext != std::string::npos && fileName.substr(ext) != Mesh::extension;
}
//

//	Private functions
void AssimpLoader::clear()
{
    vertices.clear();
    normales.clear();
    uvs.clear();
    bones.clear();
    weights.clear();
    faces.clear();

    boneMap.clear();
    roots.clear();
    joints.clear();

    //animations.clear();
}
void AssimpLoader::readSceneHierarchy(const aiNode* node, int depth)
{
    //int parentIndex = -1;
    Skeleton::Bone* joint = nullptr;
    for(unsigned int j = 0; j < joints.size(); j++)
        if(joints[j].name == std::string(node->mName.C_Str()))
        {
            joint = &joints[j];
            //parentIndex = j;

            for(int l = 0; l < 4; l++)
                for(int m = 0; m < 4; m++)
                    joint->relativeBindTransform[l][m] = node->mTransformation[m][l];
            break;
        }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        for(unsigned int j = 0; joint && j < joints.size(); j++)
            if(joints[j].name == std::string(node->mChildren[i]->mName.C_Str()))
            {
                joint->sons.push_back(&joints[j]);
                joints[j].parent = joint;
                break;
            }
        readSceneHierarchy(node->mChildren[i], depth + 1);
    }
}
//

//
void AssimpLoader::printJoint(Skeleton::Bone* bone, int depth)
{
    for(int i = 0; i < depth; i++)
        std::cout << "  ";
    std::cout << bone->name << std::endl;

    for(unsigned int i = 0; i < bone->sons.size(); i++)
        printJoint(bone->sons[i], depth + 1);
}
//