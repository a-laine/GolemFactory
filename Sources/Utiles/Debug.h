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
		void initialize(const std::string& pointMeshName, const std::string& cubeMeshName, const std::string& sphereMeshName, const std::string& capsuleMeshName, const std::string& pointShaderName, const std::string& segmentShaderName, const std::string& defaultShaderName, const std::string& wiredShaderName);
		//

		//  Draw functions
		static void drawPoint(const glm::vec3& p) { Debug::point(p, This->pointShader); };
		static void drawCube(const glm::mat4& transform, const glm::vec3& size) { Debug::mesh(This->cubeMesh, glm::scale(transform, size), This->defaultShader); };
		static void drawSphere(const glm::vec3& center, const float& radius) { Debug::mesh(This->sphereMesh, glm::scale(glm::translate(glm::mat4(1.f), center), glm::vec3(radius)), This->defaultShader); };
		static void drawLine(const glm::vec3& point1, const glm::vec3& point2) { Debug::line(point1, point2, This->segmentShader); };
		static void drawCapsule(const glm::vec3& point1, const glm::vec3& point2, const float& radius) { Debug::capsule(point1, point2, radius, This->wiredShader); };
		static void drawMesh(const Mesh* const mesh, const glm::mat4& transform) { Debug::mesh(mesh, transform, This->wiredShader); };

		static void drawWiredCube(const glm::mat4& transform, const glm::vec3& size) { Debug::mesh(This->cubeMesh, glm::scale(transform, size), This->wiredShader); };
		static void drawWiredSphere(const glm::vec3& center, const float& radius) { Debug::mesh(This->sphereMesh, glm::scale(glm::translate(glm::mat4(1.f), center), glm::vec3(radius)), This->wiredShader); };
		static void drawWiredCapsule(const glm::vec3& point1, const glm::vec3& point2, const float& radius) { Debug::capsule(point1, point2, radius, This->wiredShader); };
		static void drawWiredMesh(const Mesh* const mesh, const glm::mat4& transform) { Debug::mesh(mesh, transform, This->wiredShader); };
		//

		//	Log fuctions
		static void log(const std::string& message);
		static void logWarning(const std::string& message);
		static void logError(const std::string& message);
		//

		// Attributes
		static glm::vec3 color;
		static glm::mat4 view;
		static glm::mat4 projection;

		static const glm::vec3 black;
		static const glm::vec3 white;
		static const glm::vec3 magenta;
		static const glm::vec3 orange;
		static const glm::vec3 grey;
		static const glm::vec3 red;
		static const glm::vec3 green;
		static const glm::vec3 blue;

		static const glm::vec3 darkBlue;
		static const glm::vec3 darkGreen;
		//

	private:
		//  Default
		Debug();  //!< Default constructor.
		~Debug(); //!< Default destructor.
		//

		// real draw functions
		static void point(const glm::vec3& p, Shader* shader);
		static void line(const glm::vec3& point1, const glm::vec3& point2, Shader* shader);
		static void capsule(const glm::vec3& point1, const glm::vec3& point2, const float& radius, Shader* shader);
		static void mesh(const Mesh* const mesh, const glm::mat4& transform, Shader* shader);
		//

		// Attributes
		Renderer* renderer;

		Mesh* pointMesh;
		Mesh* cubeMesh;
		Mesh* sphereMesh;
		Mesh* capsuleMesh;

		Shader* pointShader;
		Shader* segmentShader;
		Shader* defaultShader;
		Shader* wiredShader;
		//
};
