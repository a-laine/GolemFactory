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
	//

protected:
	// Attributes
	Mesh* mesh;
	//
};
