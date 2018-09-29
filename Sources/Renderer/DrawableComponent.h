#pragma once

#include <string>
#include <glm/glm.hpp>

#include "EntityComponent\Component.hpp"


class Shader;
class Mesh;


class DrawableComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(DrawableComponent, Component)
	public:
		DrawableComponent(const std::string& meshName = "default", const std::string& shaderName = "default");
		virtual ~DrawableComponent() override;

		void setShader(const std::string& shaderName);
		void setShader(Shader* shader);
		void setMesh(const std::string& meshName);
		void setMesh(Mesh* mesh);

		Shader* getShader() const;
		Mesh* getMesh() const;

		glm::vec3 getMeshBBMax() const;
		glm::vec3 getMeshBBMin() const;

	private:
		Mesh* m_mesh;
		Shader* m_shader;
};

