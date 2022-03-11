#pragma once

#include "VirtualSceneQuerry.h"

class FrustrumSceneQuerry : public VirtualSceneQuerry
{
	public:
		FrustrumSceneQuerry(const glm::vec4& position, const glm::vec4& direction, const glm::vec4& verticalDir, const glm::vec4& leftDir, float verticalAngle, float horizontalAngle);

		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;
		std::vector<const NodeVirtual*>& getResult();

	private:
		glm::vec4 cameraPosition;
		glm::vec4 cameraDirection;
		glm::vec4 cameraVerticalAxis;
		glm::vec4 cameraLeftAxis;
		float cameraVerticalAngle;
		float cameraHorizontalAngle;
};
