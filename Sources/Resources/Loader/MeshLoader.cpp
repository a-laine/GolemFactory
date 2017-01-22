#include "MeshLoader.h"

//  Default
MeshLoader::MeshLoader() {}
MeshLoader::~MeshLoader()
{
	clear();
}
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

		unsigned int facesOffset = 0;	//	needed to pack multiple assimp object in the same mesh
		glm::vec3 defaultColor(0.5f, 0.5f, 0.5f);
		glm::vec3 meshColor;
		bool bonesPass = false;
		std::vector<int> meshVertexOffset;
		
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
					meshColor = defaultColor;
				else
				{
					aiColor3D color(0.f, 0.f, 0.f);
					m->Get(AI_MATKEY_COLOR_DIFFUSE, color);
					meshColor = glm::vec3(color.r, color.g, color.b);
				}
			}
			else meshColor = defaultColor;

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
			if (mesh->HasBones()) bonesPass = true;
		}


		if (bonesPass)
		{
			//	create bone map
			for (unsigned int i = 0; i < scene->mNumMeshes; i++)
				for (unsigned int j = 0; j < scene->mMeshes[i]->mNumBones; j++)
					if (boneMap.find(scene->mMeshes[i]->mBones[j]->mName.C_Str()) == boneMap.end())
						boneMap[std::string(scene->mMeshes[i]->mBones[j]->mName.C_Str())] = boneMap.size();

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

						if (bones[vertexIndex].x < 0)
						{
							bones[vertexIndex].x = boneMap[std::string(mesh->mBones[j]->mName.C_Str())];
							weights[vertexIndex].x = vertexWeight;
						}
						else if (bones[vertexIndex].y < 0)
						{
							bones[vertexIndex].y = boneMap[std::string(mesh->mBones[j]->mName.C_Str())];
							weights[vertexIndex].y = vertexWeight;
						}
						else if (bones[vertexIndex].z < 0)
						{
							bones[vertexIndex].z = boneMap[std::string(mesh->mBones[j]->mName.C_Str())];
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
			connectParent();
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
	weights.clear();
	bones.clear();
	faces.clear();

	boneMap.clear();
	joints.clear();
	roots.clear();
}
void MeshLoader::readSceneHierarchy(const aiNode* node, int depth)
{
	Joint joint;
	std::string name(node->mName.C_Str());
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		std::string sonName(node->mChildren[i]->mName.C_Str());
		if (boneMap.find(name) != boneMap.end() && boneMap.find(sonName) != boneMap.end())
			joint.sons.push_back(boneMap[sonName]);
		readSceneHierarchy(node->mChildren[i], depth + 1);
	}
	if (boneMap.find(name) != boneMap.end()) joints.push_back(joint);
}
void MeshLoader::connectParent()
{
	for (unsigned int i = 0; i < joints.size(); i++)
		joints[i].parent = -1;
	for (unsigned int i = 0; i < joints.size(); i++)
	{
		for (unsigned int j = 0; j < joints[i].sons.size(); j++)
			joints[joints[i].sons[j]].parent = i;
	}
	for (unsigned int i = 0; i < joints.size(); i++)
		if(joints[i].parent == (unsigned int)-1) roots.push_back(i);
}
//



