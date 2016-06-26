#pragma once

#include "InstanceVirtual.h"

class InstanceDrawable : public InstanceVirtual
{
public:
	//  Default
	InstanceDrawable(std::string meshName);
	virtual ~InstanceDrawable();
	//

	//	Public functions
	void draw(Shader* shaderToUse);

	void setOrientation(glm::mat4 m);
	//

protected:
	// Attributes
	Mesh* mesh;
	glm::mat4 rotationMatrix;
	//
};
