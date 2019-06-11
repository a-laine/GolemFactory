#pragma once

#include "Shape.h"

#include <vector>

class Mesh;

class Hull : public Shape
{
	public:
		//	Default
		Hull(Mesh* m);
		//

		//	Public functions
		virtual Sphere toSphere() const override;
		virtual AxisAlignedBox toAxisAlignedBox() const override;
		virtual void operator=(const Shape& s) override;
		virtual void transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) override;
		virtual Shape* duplicate() const override;
		virtual glm::vec3 GJKsupport(const glm::vec3& direction) const override;

		static Mesh* fromMesh(const Mesh* m);
		//

		//	Attributes
		Mesh* mesh;
		//bool degenerate;
		//

	private:
		struct HullMesh
		{
			bool degenerated;
			std::vector<glm::vec3> vertices;
			std::vector<unsigned short> faces;
			std::vector<glm::vec3> normales;
		};

		static HullMesh initializePolyhedron(const Mesh* mesh);

		void quickhull(const Mesh* mesh);
		void _gethull(std::vector<glm::vec3>& v, std::vector<unsigned short>& f);
};