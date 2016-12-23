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
		void setOrientation(glm::mat4 m);
		void setShader(std::string shaderName);
		void setShader(Shader* s);
		void setMesh(std::string meshName);
		void setMesh(Mesh* m);

		Shader* getShader() const;
		Mesh* getMesh() const;
		glm::mat4 getModelMatrix() const;
		//

	protected:
		// Attributes
		Mesh* mesh;
		Shader* shader;
		glm::mat4 rotationMatrix;
		//
};
