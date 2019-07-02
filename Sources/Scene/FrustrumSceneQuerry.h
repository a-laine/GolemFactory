#pragma once

#include "VirtualSceneQuerry.h"

class FrustrumSceneQuerry : public VirtualSceneQuerry
{
	public:
		FrustrumSceneQuerry(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& verticalDir, const glm::vec3& leftDir, float verticalAngle, float horizontalAngle);

		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;
		std::vector<const NodeVirtual*>& getResult();

	private:
		glm::vec3 cameraPosition;
		glm::vec3 cameraDirection;
		glm::vec3 cameraVerticalAxis;
		glm::vec3 cameraLeftAxis;
		float cameraVerticalAngle;
		float cameraHorizontalAngle;
};
