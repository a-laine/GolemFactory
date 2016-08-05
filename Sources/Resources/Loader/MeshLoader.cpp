#include "MeshLoader.h"

//  Public functions
int MeshLoader::loadMesh(std::string file,
						 std::vector<glm::vec3>& vertices,
						 std::vector<glm::vec3>& normales,
						 std::vector<glm::vec3>& color,
						 std::vector<unsigned int>& faces)
{
	if (file.substr(file.find_last_of(".") + 1) == "gfmesh")
	{
		return -1;
	}
	else
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file.c_str(), aiProcessPreset_TargetRealtime_Quality);
		if (!scene) { std::cerr << "ERROR : loading mesh : " << file << "\ncould not open file" << std::endl; return 1; }

		unsigned int facesOffset = 0;	//	needed to pack multiple object in the same mesh
		glm::vec3 meshColor = glm::vec3(1.0, 1.0, 1.0);// 0.1137f, 0.5137f, 0.1333f);
		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			// import material and pack into vertex color
			aiMesh* mesh = scene->mMeshes[i];
			if (scene->HasMaterials())
			{
				aiMaterial* m = scene->mMaterials[mesh->mMaterialIndex];
				aiColor3D color(0.f, 0.f, 0.f);
				m->Get(AI_MATKEY_COLOR_DIFFUSE, color);
				meshColor = glm::vec3(color.r, color.g, color.b);
			}

			//	import faces index
			for (int j = 0; j < mesh->mNumFaces; j++)
				for (int k = 0; k < 3; k++)
					faces.push_back(mesh->mFaces[j].mIndices[k] + facesOffset);
			facesOffset += mesh->mNumVertices;

			//	import vertex attributes
			for (int j = 0; j < mesh->mNumVertices; j++)
			{
				aiVector3D pos = mesh->mVertices[j];
				vertices.push_back(glm::vec3(pos.x,pos.y,pos.z));

				aiVector3D normal = (mesh->HasNormals() ? mesh->mNormals[j] : aiVector3D(0.0f, 0.0f, 0.0f));
				normales.push_back(glm::vec3(normal.x, normal.y, normal.z));

				color.push_back(meshColor);
			}
		}
		importer.FreeScene();
		return 0;
	}
}
//