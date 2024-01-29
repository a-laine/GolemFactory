#include "ReferenceMeshGenerator.h"
#include <Utiles/ToolBox.h>

#include <vector>
#include <set>
/*#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>*/


Mesh* ReferenceMeshGenerator::getReferenceCapsule(const unsigned int& quadrature)
{
	//	create buffers
	std::vector<vec4f> vertices;
	std::vector<vec4f> normals;
	std::vector<vec4f> colors;
	std::vector<unsigned int> faces;

	//	cylinder part
	float pi2 = 2 * (float)PI;
	float pi_2 = 0.5f * (float)PI;
	vec4f previous = vec4f(1, 0, 0, 1);
	vec4f s1 = vec4f(0, 0, 1, 0);
	vec4f X = vec4f(1, 0, 0, 0);
	vec4f Y = vec4f(0, 1, 0, 0);
	vec4f Z = vec4f(0, 0, 1, 0);
	vec4f white = vec4f(1, 1, 1, 1);

	for (unsigned int i = 1; i < quadrature + 1; i++)
	{
		float angle = pi2 * i / quadrature;
		vec4f tmp = cos(angle) * X + sin(angle) * Y;
		tmp.w = 1.f;

		vec4f o = 0.5f * (tmp + previous);
		vec4f s2 = 0.5f * (tmp - previous).getNorm() * (tmp - previous).getNormal();

		vertices.push_back(o - s1 - s2);  
		normals.push_back(vec4f(vertices.back().x, vertices.back().y, 0, 0).getNormal());
		colors.push_back(vec4f(1.f, 1.f, 1.f, 1.f));

		vertices.push_back(o + s1 - s2);  
		normals.push_back(vec4f(vertices.back().x, vertices.back().y, 0, 0).getNormal());
		colors.push_back(vec4f(1.f, 1.f, 1.f, 1.f));

		vertices.push_back(o - s1 + s2);  
		normals.push_back(vec4f(vertices.back().x, vertices.back().y, 0, 0).getNormal());
		colors.push_back(vec4f(1.f, 1.f, 1.f, 1.f));

		vertices.push_back(o + s1 + s2);  
		normals.push_back(vec4f(vertices.back().x, vertices.back().y, 0, 0).getNormal());
		colors.push_back(vec4f(1.f, 1.f, 1.f, 1.f));

		faces.push_back(vertices.size() - 4);
		faces.push_back(vertices.size() - 3);
		faces.push_back(vertices.size() - 2);

		faces.push_back(vertices.size() - 3);
		faces.push_back(vertices.size() - 2);
		faces.push_back(vertices.size() - 1);

		previous = tmp;
	}


	//	hemisphere 1
	vec4f v1, v2, v3, v4;
	unsigned int quadrature_s = quadrature / 4;
	for (unsigned int j = 0; j < quadrature_s; j++)
	{
		v4 = cos(pi_2 * j / quadrature_s) * X + sin(pi_2 * j / quadrature_s) * Z;
		v3 = cos(pi_2 * (j + 1) / quadrature_s) * X + sin(pi_2 * (j + 1) / quadrature_s) * Z;

		for (unsigned int i = 1; i < quadrature + 1; i++)
		{
			// compute new vectors
			v1 = cos(pi2 * i / quadrature) * cos(pi_2 * j / quadrature_s) * X +
				 sin(pi2 * i / quadrature) * cos(pi_2 * j / quadrature_s) * Y +
				 sin(pi_2 * j / quadrature_s) * Z;

			v2 = cos(pi2 * i / quadrature) * cos(pi_2 * (j + 1) / quadrature_s) * Z +
				 sin(pi2 * i / quadrature) * cos(pi_2 * (j + 1) / quadrature_s) * Y +
				 sin(pi_2 * (j + 1) / quadrature_s) * Z;

			vertices.push_back(v2);
			normals.push_back(vertices.back().getNormal());
			colors.push_back(white);

			vertices.push_back(v1);
			normals.push_back(vertices.back().getNormal());
			colors.push_back(white);

			vertices.push_back(v3);
			normals.push_back(vertices.back().getNormal());
			colors.push_back(white);

			vertices.push_back(v4);  
			normals.push_back(vertices.back().getNormal());
			colors.push_back(white);

			faces.push_back(vertices.size() - 4);
			faces.push_back(vertices.size() - 3);
			faces.push_back(vertices.size() - 2);

			faces.push_back(vertices.size() - 3);
			faces.push_back(vertices.size() - 2);
			faces.push_back(vertices.size() - 1);

			//	vector shift
			v4 = v1;
			v3 = v2;
		}
	}

	//	hemisphere 2 
	for (unsigned int j = 0; j < quadrature_s; j++)
	{
		v4 = cos(pi_2 * j / quadrature_s) * X - sin(pi_2 * j / quadrature_s) * Z;
		v3 = cos(pi_2 * (j + 1) / quadrature_s) * X - sin(pi_2 * (j + 1) / quadrature_s) * Z;

		for (unsigned int i = 1; i < quadrature + 1; i++)
		{
			// compute new vectors
			v1 = cos(pi2 * i / quadrature) * cos(pi_2 * j / quadrature_s) * X +
				 sin(pi2 * i / quadrature) * cos(pi_2 * j / quadrature_s) * Y -
				 sin(pi_2 * j / quadrature_s) *Z;

			v2 = cos(pi2 * i / quadrature) * cos(pi_2 * (j + 1) / quadrature_s) * X +
				 sin(pi2 * i / quadrature) * cos(pi_2 * (j + 1) / quadrature_s) * Y -
				 sin(pi_2 * (j + 1) / quadrature_s) * Z;

			vertices.push_back(v2);
			normals.push_back(vertices.back().getNormal());
			colors.push_back(white);

			vertices.push_back(v1);
			normals.push_back(vertices.back().getNormal());
			colors.push_back(white);

			vertices.push_back(v3);
			normals.push_back(vertices.back().getNormal());
			colors.push_back(white);

			vertices.push_back(v4);
			normals.push_back(vertices.back().getNormal());
			colors.push_back(white);


			faces.push_back(vertices.size() - 4);
			faces.push_back(vertices.size() - 3);
			faces.push_back(vertices.size() - 2);

			faces.push_back(vertices.size() - 3);
			faces.push_back(vertices.size() - 2);
			faces.push_back(vertices.size() - 1);

			//	vector shift
			v4 = v1;
			v3 = v2;
		}
	}

	//	end
	ToolBox::optimizeStaticMesh(vertices, normals, colors, faces);
	Mesh* mesh = new Mesh("generatedCapsule");
	std::vector<vec4i> bonesArray;
	std::vector<vec4f> weightsArray;
	mesh->initialize(vertices, normals, colors, faces, bonesArray, weightsArray);
	return mesh;
}

