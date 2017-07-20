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

		auto m = scene->mRootNode->mTransformation.Inverse();
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				globalMatrix[i][j] = m[j][i];
		globalMatrix = glm::mat4(1.f);

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

						glm::mat4 offset;
						for (int l = 0; l < 4; l++)
							for (int m = 0; m < 4; m++)
								offset[l][m] = scene->mMeshes[i]->mBones[j]->mOffsetMatrix[m][l];

						joints.push_back(Joint(boneName, offset));
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
			for (unsigned int i = 0; i < bones.size(); i++)
			{
				if (bones[i].x < 0) bones[i].x = 0;
				if (bones[i].y < 0) bones[i].y = 0;
				if (bones[i].z < 0) bones[i].z = 0;
			}

			//	read scene hierarchy for importing skeleton
			if (scene->mRootNode) readSceneHierarchy(scene->mRootNode);
			for (unsigned int i = 0; i < joints.size(); i++)
				if (joints[i].parent == (unsigned int)-1) roots.push_back(i);
		}

		//	Load animation
		/*	Debug
			if (scene->mNumAnimations > 0)
			{
				std::cout << "mesh : " << file.substr(file.find_last_of("/") + 1) << std::endl;
				std::cout << "mNumAnimations : " << scene->mNumAnimations << std::endl;
			}
		*/
		for (unsigned int i = 0; i < scene->mNumAnimations; i++)
		{
			// init
			/*	Debug
				std::cout << "   annimation " << i << ", mNumChannels : " << scene->mAnimations[i]->mNumChannels << std::endl;
			*/

			BidirectionnalVectorMap positionBM;
			BidirectionnalVectorMap scaleBM;
			BidirectionnalQuaternionMap rotationBM;
			glm::vec3 position, scale;
			glm::fquat rotation;

			// extract array from assimp
			for (unsigned int j = 0; j < scene->mAnimations[i]->mNumChannels; j++)
			{
				aiNodeAnim* currentChannel = scene->mAnimations[i]->mChannels[j];
				std::string channelName(currentChannel->mNodeName.C_Str());
				
				/*	Debug
					std::cout << "      channel " << channelName << std::endl;
					std::cout << "        mNumPositionKeys : " << currentChannel->mNumPositionKeys << std::endl;
					std::cout << "        mNumRotationKeys : " << currentChannel->mNumRotationKeys << std::endl;
					std::cout << "        mNumScalingKeys : "  << currentChannel->mNumScalingKeys << std::endl;
				*/

				// import translation vector
				for (unsigned int k = 0; k < scene->mAnimations[i]->mChannels[j]->mNumPositionKeys; k++)
				{
					position = glm::vec3(scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.x,
										 scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.y,
										 scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.z);
					positionBM.push_back(std::make_tuple((float)currentChannel->mPositionKeys[k].mTime, channelName, position));
				}

				// import rotation quaternion
				for (unsigned int k = 0; k < scene->mAnimations[i]->mChannels[j]->mNumRotationKeys; k++)
				{
					rotation = glm::fquat(scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.w,
										  scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.x,
										  scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.y,
										  scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.z);
					rotationBM.push_back(std::make_tuple((float)currentChannel->mRotationKeys[k].mTime, channelName, rotation));
				}

				// import scaling vector
				for (unsigned int k = 0; k < scene->mAnimations[i]->mChannels[j]->mNumScalingKeys; k++)
				{
					scale = glm::vec3(scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.x,
									  scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.y,
									  scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.z);
					scaleBM.push_back(std::make_tuple((float)currentChannel->mScalingKeys[k].mTime, channelName, scale));
				}
			}

			// sort lists in time
			positionBM.sort([](tupleVec3 a, tupleVec3 b) { return std::get<float>(a) < std::get<float>(b); });
			scaleBM.sort([](tupleVec3 a, tupleVec3 b) { return std::get<float>(a) < std::get<float>(b); });
			rotationBM.sort([](tupleQuat a, tupleQuat b) { return std::get<float>(a) < std::get<float>(b); });

			// read lists in time and create keyframes
			KeyFrame currentKeyFrame;
				currentKeyFrame.time = 0.f;
				currentKeyFrame.poses.assign(joints.size(), JointPose());
			BidirectionnalVectorMap::iterator itPos = positionBM.begin();
			BidirectionnalVectorMap::iterator itSca = scaleBM.begin();
			BidirectionnalQuaternionMap::iterator itRot = rotationBM.begin();

			while (itPos != positionBM.end() || itSca != scaleBM.end() || itRot != rotationBM.end())
			{
				// counter for no more information for current time
				int increment = 0;

				// update current keyframe pose for skeleton (here position)
				if (itPos != positionBM.end() && std::get<float>(*itPos) <= currentKeyFrame.time)
				{
					currentKeyFrame.poses[boneMap[std::get<std::string>(*itPos)]].position = std::get<glm::vec3>(*itPos);
					itPos++;
				}
				else increment++;

				// update current keyframe pose for skeleton (here rotation)
				if (itRot != rotationBM.end() && std::get<float>(*itRot) <= currentKeyFrame.time)
				{
					currentKeyFrame.poses[boneMap[std::get<std::string>(*itRot)]].orientation = std::get<glm::fquat>(*itRot);
					itRot++;
				}
				else increment++;

				// update current keyframe pose for skeleton (here scaling)
				if (itSca != scaleBM.end() && std::get<float>(*itSca) <= currentKeyFrame.time)
				{
					currentKeyFrame.poses[boneMap[std::get<std::string>(*itSca)]].scale = std::get<glm::vec3>(*itSca);
					itSca++;
				}
				else increment++;

				if (increment >= 3)
				{
					// push keyframe and increment currentKeyFrame time
					animations.push_back(currentKeyFrame);

					float t1 = FLT_MAX; if (itPos != positionBM.end()) t1 = std::get<float>(*itPos);
					float t2 = FLT_MAX; if (itRot != rotationBM.end()) t1 = std::get<float>(*itRot);
					float t3 = FLT_MAX; if (itSca != scaleBM.end())    t1 = std::get<float>(*itSca);
					
					currentKeyFrame.time = std::min(std::min(t1, t2), t3);
					if (currentKeyFrame.time == FLT_MAX) break;
				}
			}
			/*	Warnning : 
				if multiple animation are attached to mesh they will be all stacked into "animations" attributes 
				(time is discontinue when changing from one to another).
				maybe add some label to diferenciate them			
			*/
		}
		/*	Debug
			if (scene->mNumAnimations > 0) std::cout << std::endl << std::endl;
		*/

		// end
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
					joint->relativeBindTransform[l][m] = node->mTransformation[m][l];
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