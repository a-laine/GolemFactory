#pragma once

#include <iostream>

#include <GL/glew.h>

#include "Utiles/Mutex.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Utiles/Camera.h"
#include "Utiles/Singleton.h"
#include "Resources/ResourceManager.h"
#include "Instances/InstanceManager.h"
#include "Scene/SceneManager.h"

#include "HUD/WidgetManager.h"


class Renderer : public Singleton<Renderer>
{
	friend class Singleton<Renderer>;

	public:
		//	Miscellaneous
		enum ShaderIdentifier
		{
			INSTANCE_DRAWABLE = 1,
			INSTANCE_ANIMATABLE,
			HUD,
			GRID,

			INSTANCE_DRAWABLE_BB,
			INSTANCE_ANIMATABLE_BB
		};
		enum RenderOption
		{
			DEFAULT,
			BOUNDING_BOX
		};
		//

		//  Public functions
		void initGLEW(int verbose = 1);
		void initializeGrid(const unsigned int& gridSize, const float& elementSize = 1.f, const glm::vec3& color = glm::vec3(0.4f, 0.2f, 0.1f));
		void render(Camera* renderCam);
		void renderHUD(Camera* renderCam);
		//

		//  Set/get functions
		void setCamera(Camera* cam);
		void setWindow(GLFWwindow* win);
		void setShader(ShaderIdentifier id, Shader* s);
		void setGridVisible(bool enable);
		void setRenderOption(const RenderOption& option);
		
		Camera* getCamera();
		GLFWwindow* getWindow();
		Shader* getShader(ShaderIdentifier id);
		bool isGridVisible();
		unsigned int getNbDrawnInstances() const;
		unsigned int getNbDrawnTriangles() const;
		RenderOption getRenderOption() const;
		//

		//	Render function
		void drawInstanceDrawable(InstanceVirtual* ins, const float* view, const float* projection, const glm::mat4& base = glm::mat4(1.f));
		void drawInstanceAnimatable(InstanceVirtual* ins, const float* view, const float* projection);
		void drawInstanceContainer(InstanceVirtual* ins, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model);
		//

	private:
		//  Default
		Renderer();
		~Renderer();
		//

		//	Protected functions
		void loadMVPMatrix(Shader* shader, const float* model, const float* view, const float* projection);
		//

		//  Attributes
		GLFWwindow* window;
		Camera* camera;
		std::map<ShaderIdentifier, Shader*> defaultShader;
		RenderOption renderOption;

		bool drawGrid;
		unsigned int vboGridSize;
		GLuint gridVAO, vertexbuffer, arraybuffer, colorbuffer, normalbuffer;

		unsigned int instanceDrawn, trianglesDrawn;
		double dummy;
		Shader* lastShader;
		//
};

