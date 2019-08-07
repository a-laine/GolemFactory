#pragma once

#include <glm/glm.hpp>

#include "Utiles/Singleton.h"
#include "Resources/ResourceManager.h"
#include "Renderer/Renderer.h"


class Debug : public Singleton<Debug>
{
	friend class Singleton<Debug>;

	public:
		// Public functions
		void initialize(const std::string& pointMeshName, const std::string& cubeMeshName, const std::string& sphereMeshName, const std::string& pointShaderName, const std::string& segmentShaderName, const std::string& defaultShaderName, const std::string& wiredShaderName);
		//

		//  Draw functions
		static void drawPoint(const glm::vec3& p);
		static void drawCube(const glm::mat4& transform, const glm::vec3& size);
		static void drawSphere(const glm::vec3& center, const float& radius);
		static void drawLine(const glm::vec3& point1, const glm::vec3& point2);
		static void drawMesh(const Mesh* const mesh, const glm::mat4& transform);

		static void drawWiredCube(const glm::mat4& transform, const glm::vec3& size);
		static void drawWiredSphere(const glm::vec3& center, const float& radius);
		static void drawWiredMesh(const Mesh* const mesh, const glm::mat4& transform);
		//

		// Attributes
		static glm::vec3 color;
		static glm::mat4 view;
		static glm::mat4 projection;
		//

	private:
		//  Default
		Debug();  //!< Default constructor.
		~Debug(); //!< Default destructor.
		//

		// Attributes
		Renderer* renderer;

		Mesh* pointMesh;
		Mesh* cubeMesh;
		Mesh* sphereMesh;

		Shader* pointShader;
		Shader* segmentShader;
		Shader* defaultShader;
		Shader* wiredShader;
		//
};
