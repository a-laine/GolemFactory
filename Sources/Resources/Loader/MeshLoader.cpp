#include "MeshLoader.h"

//  Public functions
int MeshLoader::loadMesh(std::string file,
						 std::vector<glm::vec3>& vertices,
						 std::vector<glm::vec3>& normales,
						 std::vector<glm::vec3>& color,
						 std::vector<unsigned int>& faces)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file.c_str(),
		aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
	if (!scene) { std::cerr << "ERROR : loading mesh : " << file << "\ncould not open file" << std::endl; return 1; }

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		for (int j = 0; j < mesh->mNumFaces; j++)
			for (int k = 0; k < 3; k++)
				faces.push_back(mesh->mFaces[j].mIndices[k]);

		for (int j = 0; j < mesh->mNumVertices; j++)
		{
			aiVector3D pos = mesh->mVertices[j];
			vertices.push_back(glm::vec3(pos.x,pos.y,pos.z));

			aiVector3D normal = (mesh->HasNormals() ? mesh->mNormals[j] : aiVector3D(0.0f, 0.0f, 0.0f));
			normales.push_back(glm::vec3(normal.x, normal.y, normal.z));

			color.push_back(glm::vec3(1.0,1.0,1.0));
		}
	}
	importer.FreeScene();
	return 0;
}
//