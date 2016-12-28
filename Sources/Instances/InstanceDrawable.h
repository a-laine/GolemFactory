#pragma once

#include "InstanceVirtual.h"

class InstanceDrawable : public InstanceVirtual
{
	public:
		//  Default
		InstanceDrawable(std::string meshName = "default", std::string shaderName = "default");
		virtual ~InstanceDrawable();
		//

		//	Public functions
		void setShader(std::string shaderName);
		void setShader(Shader* s);
		void setMesh(std::string meshName);
		void setMesh(Mesh* m);

		glm::vec3 getBBSize() const;
		Shader* getShader() const;
		Mesh* getMesh() const;
		//

	protected:
		// Attributes
		Mesh* mesh;
		Shader* shader;
		//
};
