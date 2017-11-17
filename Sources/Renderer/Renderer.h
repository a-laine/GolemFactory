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

#include "HUD/Layer.h"
#include "HUD/WidgetVirtual.h"


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
			GRID
		};
		//

		//  Public functions
		void initGLEW(int verbose = 1);
		void initializeGrid(unsigned int gridSize, float elementSize = 1.f);
		void render();
		//

		//  Set/get functions
		void setCamera(Camera* cam);
		void setWindow(GLFWwindow* win);
		void setShader(ShaderIdentifier id, Shader* s);
		void setGridVisible(bool enable);
		
		Camera* getCamera();
		GLFWwindow* getWindow();
		Shader* getShader(ShaderIdentifier id);
		bool isGridVisible();
		//

	private:
		//  Default
		Renderer();
		~Renderer();
		//

		//	Protected functions
		void loadMVPMatrix(Shader* shader, const float* model, const float* view, const float* projection) const;
		void drawInstanceDrawable(InstanceVirtual* ins, const float* view, const float* projection);
		void drawInstanceAnimatable(InstanceVirtual* ins, const float* view, const float* projection);
		void drawInstanceContainer(InstanceVirtual* ins, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model);

		void drawWidgetVirtual(WidgetVirtual* widget, const glm::mat4& modelBase, const float* view, const float* projection);
		void drawLayer(Layer* layer, const glm::mat4& modelBase, const float* view, const float* projection);
		//

		//  Attributes
		GLFWwindow* window;
		Camera* camera;

		std::map<ShaderIdentifier, Shader*> defaultShader;
		bool drawGrid;
		unsigned int vboGridSize;
		GLuint gridVAO, vertexbuffer, arraybuffer, colorbuffer, normalbuffer;
		/*float* vertexBufferGrid;
		uint32_t* indexBufferGrid;*/

		double dummy;
		WidgetVirtual* dummyPlaceHolder;
		Layer* dummyLayer;
		//
};

