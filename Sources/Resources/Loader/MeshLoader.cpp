#include "MeshLoader.h"

//  Default
MeshLoader::MeshLoader() {}
MeshLoader::~MeshLoader() { clear(); }
//

//  Public functions
int MeshLoader::loadMesh(std::string file)
{
	//	clear all buffers
	clear();

	//	import mesh using assimp or GF loader
	if (file.substr(file.find_last_of(".") + 1) == "gfmesh")
	{
		std::cerr << "ERROR : loading mesh : " << file << "\ngfmesh format loading not yet implemented" << std::endl;
		return -1;
	}
	else
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
		if (!scene) { std::cerr << "ERROR : loading mesh : " << file << "\ncould not open file" << std::endl; return 1; }

		//	usefull parameters for next
		unsigned int facesOffset = 0;
		glm::vec3 meshColor;
		bool hasSkeleton = false;
		std::vector<int> meshVertexOffset;
		
		//	Load mesh
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			//	import material and pack into vertex color
			aiMesh* mesh = scene->mMeshes[i];
			if (scene->HasMaterials())
			{
				aiMaterial* m = scene->mMaterials[mesh->mMaterialIndex];
				aiString nameStr;
				m->Get(AI_MATKEY_NAME, nameStr);
				std::string name(nameStr.C_Str());
				if (name.find("JoinedMaterial") != std::string::npos)
					meshColor = glm::vec3(0.5f, 0.5f, 0.5f);
				else
				{
					aiColor3D color(0.f, 0.f, 0.f);
					m->Get(AI_MATKEY_COLOR_DIFFUSE, color);
					meshColor = glm::vec3(color.r, color.g, color.b);
				}
			}
			else meshColor = glm::vec3(0.5f, 0.5f, 0.5f);

			//	import faces index
			for (unsigned int j = 0; j < mesh->mNumFaces; j++)
				for (int k = 0; k < 3; k++)
					faces.push_back(mesh->mFaces[j].mIndices[k] + facesOffset);
			facesOffset += mesh->mNumVertices;

			//	import vertex attributes
			meshVertexOffset.push_back(vertices.size());
			for (unsigned int j = 0; j < mesh->mNumVertices; j++)
			{
				aiVector3D pos = mesh->mVertices[j];
				vertices.push_back(glm::vec3(pos.x,pos.y,pos.z));

				aiVector3D normal = (mesh->HasNormals() ? mesh->mNormals[j] : aiVector3D(0.0f, 0.0f, 0.0f));
				normales.push_back(glm::vec3(normal.x, normal.y, normal.z));

				colors.push_back(meshColor);
			}

			//	demande a second pass to parse bone and skeleton
			if (mesh->HasBones()) hasSkeleton = true;
		}

		//	Load skeleton
		if (hasSkeleton)
		{
			//	create bone map
			for (unsigned int i = 0; i < scene->mNumMeshes; i++)
				for (unsigned int j = 0; j < scene->mMeshes[i]->mNumBones; j++)
				{
					std::string boneName(scene->mMeshes[i]->mBones[j]->mName.C_Str());
					if (boneMap.find(boneName) == boneMap.end())
					{
						boneMap[boneName] = boneMap.size();
						joints.push_back(Joint(boneName));
					}
				}

						

			//	import bone index & weight for each vertex
			bones.assign(vertices.size(), glm::ivec3(-1, -1, -1));
			weights.assign(vertices.size(), glm::vec3(0.f, 0.f, 0.f));
			for (unsigned int i = 0; i < scene->mNumMeshes; i++)
			{
				aiMesh* mesh = scene->mMeshes[i];
				for (unsigned int j = 0; j < mesh->mNumBones; j++)
				{
					for (unsigned int k = 0; k < mesh->mBones[j]->mNumWeights; k++)
					{
						int vertexIndex = meshVertexOffset[i] + mesh->mBones[j]->mWeights[k].mVertexId;
						float vertexWeight = mesh->mBones[j]->mWeights[k].mWeight;
						std::string boneName(mesh->mBones[j]->mName.C_Str());

						if (bones[vertexIndex].x < 0)
						{
							bones[vertexIndex].x = boneMap[boneName];
							weights[vertexIndex].x = vertexWeight;
						}
						else if (bones[vertexIndex].y < 0)
						{
							bones[vertexIndex].y = boneMap[boneName];
							weights[vertexIndex].y = vertexWeight;
						}
						else if (bones[vertexIndex].z < 0)
						{
							bones[vertexIndex].z = boneMap[boneName];
							weights[vertexIndex].z = vertexWeight;
						}
						else
						{
							std::cerr << "ERROR : loading mesh : vertex at position " << vertices[vertexIndex].x <<' ' << vertices[vertexIndex].y << ' ' << vertices[vertexIndex].z;
							std::cerr << ": more than 3 bones weights defined" << std::endl;
						}
					}
				}
			}

			//	read scene hierarchy for importing skeleton
			if(scene->mRootNode) readSceneHierarchy(scene->mRootNode);
			for (unsigned int i = 0; i < joints.size(); i++)
				if (joints[i].parent == (unsigned int)-1) roots.push_back(i);
		}

		//	Load animation
		for (unsigned int i = 0; i < scene->mNumAnimations; i++)
		{
			for (unsigned int j = 0; j < scene->mAnimations[i]->mNumChannels; j++)
			{
				std::string channel(scene->mAnimations[i]->mChannels[j]->mNodeName.C_Str());
				glm::vec3 p, s;
				glm::fquat q;

				for (unsigned int k = 0; k < scene->mAnimations[i]->mChannels[j]->mNumPositionKeys; k++)
				{
					p = glm::vec3(	scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.x,
									scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.y,
									scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.z);
					updateKeyFramePosition((float) scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mTime, boneMap[channel], p);
				}
				for (unsigned int k = 0; k < scene->mAnimations[i]->mChannels[j]->mNumRotationKeys; k++)
				{
					q = glm::fquat( scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.w,
									scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.x,
									scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.y,
									scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.z);
					updateKeyFrameOrientation((float) scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mTime, boneMap[channel], q);
				}
				for (unsigned int k = 0; k < scene->mAnimations[i]->mChannels[j]->mNumRotationKeys; k++)
				{
					s = glm::vec3(	scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.x,
									scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.y,
									scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.z);
					updateKeyFrameScale((float) scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mTime, boneMap[channel], s);
				}
			}
		}

		///	Debug
		/*for (unsigned int i = 0; i < joints.size(); i++)
		{
			std::cout << "joint : " << i << std::endl;
			std::cout << "   p : " << joints[i].offsetPosition.x << ' ' << joints[i].offsetPosition.y << ' ' << joints[i].offsetPosition.z << std::endl;
			std::cout << "   q : " << joints[i].offsetOrientation.w << ' ' << joints[i].offsetOrientation.x << ' ' << joints[i].offsetOrientation.y << ' ' << joints[i].offsetOrientation.z << std::endl;
			std::cout << "   s : " << joints[i].offsetScale.x << ' ' << joints[i].offsetScale.y << ' ' << joints[i].offsetScale.z << std::endl;
		}*/
		for (unsigned int i = 0; i < animations.size(); i++)
		{
			std::cout << "time : " << animations[i].time << std::endl;
			for (unsigned int j = 0; j < animations[i].poses.size(); j++)
			{
				//std::cout << "   bone name : " << joints[j].name << std::endl;
				JointPose jp = animations[i].poses[j];
				if(i<5) std::cout << "      position : " << jp.position.x << ' ' << jp.position.y << ' ' << jp.position.z << std::endl;
				//std::cout << "      orientation : " << jp.orientation.w << ' ' << jp.orientation.x << ' ' << jp.orientation.y << ' ' << jp.orientation.z << std::endl;
				//std::cout << "      scale : " << jp.scale.x << ' ' << jp.scale.y << ' ' << jp.scale.z << std::endl;
			}
		}

		importer.FreeScene();
		return 0;
	}
}
//

//	Private functions
void MeshLoader::clear()
{
	vertices.clear();
	normales.clear();
	colors.clear();
	bones.clear();
	weights.clear();
	faces.clear();

	boneMap.clear();
	roots.clear();
	joints.clear();
	
	animations.clear();
}
void MeshLoader::readSceneHierarchy(const aiNode* node, int depth)
{
	int parentIndex = -1;
	Joint* joint = nullptr;
	for (unsigned int j = 0; j < joints.size(); j++)
		if(joints[j].name == std::string(node->mName.C_Str()))
		{
			joint = &joints[j];
			parentIndex = j;

			for (int l = 0; l < 4; l++)
				for (int m = 0; m < 4; m++)
					joint->offsetMatrix[l][m] = node->mTransformation[l][m];
			break;
		}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		for (unsigned int j = 0; joint && j < joints.size(); j++)
			if (joints[j].name == std::string(node->mChildren[i]->mName.C_Str()))
			{
				joint->sons.push_back(j);
				joints[j].parent = parentIndex;
				break;
			}
		readSceneHierarchy(node->mChildren[i], depth + 1);
	}
}
int MeshLoader::searchKeyFrameIndex(const float& keyTime)
{
	//	Search keyFrame
	for (unsigned int i = 0; i < animations.size(); i++)
		if (animations[i].time == keyTime) return i;

	//	keyFrame not found
	int keyFrameIndex = animations.size();
	KeyFrame kf;
		kf.time = keyTime;
		kf.poses.assign(joints.size(), JointPose());
		animations.push_back(kf);
	return keyFrameIndex;
}
void MeshLoader::updateKeyFramePosition(const float& keyTime, const int& joint, const glm::vec3& p)
{
	int keyFrameIndex = searchKeyFrameIndex(keyTime);
	animations[keyFrameIndex].poses[joint].position = p;
}
void MeshLoader::updateKeyFrameOrientation(const float& keyTime, const int& joint, const glm::fquat& q)
{
	int keyFrameIndex = searchKeyFrameIndex(keyTime);
	animations[keyFrameIndex].poses[joint].orientation = q;
}
void MeshLoader::updateKeyFrameScale(const float& keyTime, const int& joint, const glm::vec3& s)
{
	int keyFrameIndex = searchKeyFrameIndex(keyTime);
	animations[keyFrameIndex].poses[joint].scale = s;
}
//

//
void MeshLoader::printJoint(unsigned int joint, int depth)
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << joints[joint].name << std::endl;

	for (unsigned int i = 0; i < joints[joint].sons.size(); i++)
		printJoint(joints[joint].sons[i], depth + 1);
}
//