#include "ReferenceMeshGenerator.h"
#include "Utiles/ToolBox.h"

#include <vector>
#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


Mesh* ReferenceMeshGenerator::getReferenceCapsule(const unsigned int& quadrature)
{
	//	create buffers
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
	std::vector<unsigned short int> faces;

	//	cylinder part
	glm::vec3 previous = glm::vec3(1, 0, 0);
	for (unsigned int i = 1; i < quadrature + 1; i++)
	{
		float angle = 2 * glm::pi<float>() * i / quadrature;
		glm::vec3 tmp = cos(angle) * glm::vec3(1, 0, 0) + sin(angle) * glm::vec3(0, 1, 0);

		glm::vec3 o = 0.5f * (tmp + previous);
		glm::vec3 s2 = 0.5f * glm::length(tmp - previous) * glm::normalize(tmp - previous);
		glm::vec3 s1 = glm::vec3(0, 0, 1);

		vertices.push_back(o - s1 - s2);  normals.push_back(glm::normalize(glm::vec3(vertices.back().x, vertices.back().y, 0))); colors.push_back(glm::vec3(1.f, 1.f, 1.f));
		vertices.push_back(o + s1 - s2);  normals.push_back(glm::normalize(glm::vec3(vertices.back().x, vertices.back().y, 0))); colors.push_back(glm::vec3(1.f, 1.f, 1.f));
		vertices.push_back(o - s1 + s2);  normals.push_back(glm::normalize(glm::vec3(vertices.back().x, vertices.back().y, 0))); colors.push_back(glm::vec3(1.f, 1.f, 1.f));
		vertices.push_back(o + s1 + s2);  normals.push_back(glm::normalize(glm::vec3(vertices.back().x, vertices.back().y, 0))); colors.push_back(glm::vec3(1.f, 1.f, 1.f));

		faces.push_back((unsigned short int)vertices.size() - 4);
		faces.push_back((unsigned short int)vertices.size() - 3);
		faces.push_back((unsigned short int)vertices.size() - 2);

		faces.push_back((unsigned short int)vertices.size() - 3);
		faces.push_back((unsigned short int)vertices.size() - 2);
		faces.push_back((unsigned short int)vertices.size() - 1);

		previous = tmp;
	}


	//	hemisphere 1 
	glm::vec3 v1, v2, v3, v4;
	unsigned int quadrature_s = quadrature / 4;
	for (unsigned int j = 0; j < quadrature_s; j++)
	{
		v4 = glm::vec3(0, 0, 0) + cos(glm::pi<float>() / 2 * j / quadrature_s) * glm::vec3(1, 0, 0) + sin(glm::pi<float>() / 2 * j / quadrature_s) * glm::vec3(0, 0, 1);
		v3 = glm::vec3(0, 0, 0) + cos(glm::pi<float>() / 2 * (j + 1) / quadrature_s) * glm::vec3(1, 0, 0) + sin(glm::pi<float>() / 2 * (j + 1) / quadrature_s) * glm::vec3(0, 0, 1);

		for (unsigned int i = 1; i < quadrature + 1; i++)
		{
			// compute new vectors
			v1 = glm::vec3(0, 0, 0) +
				cos(2 * glm::pi<float>() * i / quadrature) * cos(glm::pi<float>() / 2 * j / quadrature_s) * glm::vec3(1, 0, 0) +
				sin(2 * glm::pi<float>() * i / quadrature) * cos(glm::pi<float>() / 2 * j / quadrature_s) * glm::vec3(0, 1, 0) +
				sin(glm::pi<float>() / 2 * j / quadrature_s) * glm::vec3(0, 0, 1);

			v2 = glm::vec3(0, 0, 0) +
				cos(2 * glm::pi<float>() * i / quadrature) * cos(glm::pi<float>() / 2 * (j + 1) / quadrature_s) * glm::vec3(1, 0, 0) +
				sin(2 * glm::pi<float>() * i / quadrature) * cos(glm::pi<float>() / 2 * (j + 1) / quadrature_s) * glm::vec3(0, 1, 0) +
				sin(glm::pi<float>() / 2 * (j + 1) / quadrature_s) * glm::vec3(0, 0, 1);

			vertices.push_back(v2);  normals.push_back(glm::normalize(vertices.back())); colors.push_back(glm::vec3(1.f, 1.f, 1.f));
			vertices.push_back(v1);  normals.push_back(glm::normalize(vertices.back())); colors.push_back(glm::vec3(1.f, 1.f, 1.f));
			vertices.push_back(v3);  normals.push_back(glm::normalize(vertices.back())); colors.push_back(glm::vec3(1.f, 1.f, 1.f));
			vertices.push_back(v4);  normals.push_back(glm::normalize(vertices.back())); colors.push_back(glm::vec3(1.f, 1.f, 1.f));

			faces.push_back((unsigned short int)vertices.size() - 4);
			faces.push_back((unsigned short int)vertices.size() - 3);
			faces.push_back((unsigned short int)vertices.size() - 2);

			faces.push_back((unsigned short int)vertices.size() - 3);
			faces.push_back((unsigned short int)vertices.size() - 2);
			faces.push_back((unsigned short int)vertices.size() - 1);

			//	vector shift
			v4 = v1;
			v3 = v2;
		}
	}

	//	hemisphere 2 
	for (unsigned int j = 0; j < quadrature_s; j++)
	{
		v4 = glm::vec3(0, 0, 0) + cos(glm::pi<float>() / 2 * j / quadrature_s) * glm::vec3(1, 0, 0) + sin(glm::pi<float>() / 2 * j / quadrature_s) * glm::vec3(0, 0, -1);
		v3 = glm::vec3(0, 0, 0) + cos(glm::pi<float>() / 2 * (j + 1) / quadrature_s) * glm::vec3(1, 0, 0) + sin(glm::pi<float>() / 2 * (j + 1) / quadrature_s) * glm::vec3(0, 0, -1);

		for (unsigned int i = 1; i < quadrature + 1; i++)
		{
			// compute new vectors
			v1 = glm::vec3(0, 0, 0) +
				cos(2 * glm::pi<float>() * i / quadrature) * cos(glm::pi<float>() / 2 * j / quadrature_s) * glm::vec3(1, 0, 0) +
				sin(2 * glm::pi<float>() * i / quadrature) * cos(glm::pi<float>() / 2 * j / quadrature_s) * glm::vec3(0, 1, 0) +
				sin(glm::pi<float>() / 2 * j / quadrature_s) * glm::vec3(0, 0, -1);

			v2 = glm::vec3(0, 0, 0) +
				cos(2 * glm::pi<float>() * i / quadrature) * cos(glm::pi<float>() / 2 * (j + 1) / quadrature_s) * glm::vec3(1, 0, 0) +
				sin(2 * glm::pi<float>() * i / quadrature) * cos(glm::pi<float>() / 2 * (j + 1) / quadrature_s) * glm::vec3(0, 1, 0) +
				sin(glm::pi<float>() / 2 * (j + 1) / quadrature_s) * glm::vec3(0, 0, -1);

			vertices.push_back(v2);  normals.push_back(glm::normalize(vertices.back())); colors.push_back(glm::vec3(1.f, 1.f, 1.f));
			vertices.push_back(v1);  normals.push_back(glm::normalize(vertices.back())); colors.push_back(glm::vec3(1.f, 1.f, 1.f));
			vertices.push_back(v3);  normals.push_back(glm::normalize(vertices.back())); colors.push_back(glm::vec3(1.f, 1.f, 1.f));
			vertices.push_back(v4);  normals.push_back(glm::normalize(vertices.back())); colors.push_back(glm::vec3(1.f, 1.f, 1.f));

			faces.push_back((unsigned short int)vertices.size() - 4);
			faces.push_back((unsigned short int)vertices.size() - 3);
			faces.push_back((unsigned short int)vertices.size() - 2);

			faces.push_back((unsigned short int)vertices.size() - 3);
			faces.push_back((unsigned short int)vertices.size() - 2);
			faces.push_back((unsigned short int)vertices.size() - 1);

			//	vector shift
			v4 = v1;
			v3 = v2;
		}
	}

	//	end
	ToolBox::optimizeStaticMesh(vertices, normals, colors, faces);
	Mesh* mesh = new Mesh("generatedCapsule");
	std::vector<glm::ivec3> bonesArray;
	std::vector<glm::vec3> weightsArray;
	mesh->initialize(vertices, normals, colors, faces, bonesArray, weightsArray);
	return mesh;
}

