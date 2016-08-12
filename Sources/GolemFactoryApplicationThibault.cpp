// Golem Factory 1.0.cpp : définit le point d'entrée pour l'application console.
//

#include <iostream>
#include <list>
#include <time.h>
#include <GL/glew.h>

#include "Utiles/System.h"
#include "Events/EventHandler.h"
#include "Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GRID_SIZE 100
#define GRID_ELEMENT_SIZE 3.0f

// prototypes
GLFWwindow* initGLFW();
void initGLEW(int verbose = 1);
void initializeForestScene();


// program
int main()
{
	// init window and opengl
	GLFWwindow* window = initGLFW();
	initGLEW();

	// Init Event handler
	EventHandler::getInstance()->addWindow(window);
	//EventHandler::getInstance()->setRepository("C:/Users/Thibault-SED/Documents/Github/GolemFactory/Resources/");
	EventHandler::getInstance()->setRepository("C:/Users/Thibault/Documents/Github/GolemFactory/Resources/");
	EventHandler::getInstance()->loadKeyMapping("RPG Key mapping");
	EventHandler::getInstance()->setCursorMode(false);
	
	// Init Resources manager and load some default shader
	//ResourceManager::getInstance()->setRepository("C:/Users/Thibault-SED/Documents/Github/GolemFactory/Resources/");
	ResourceManager::getInstance()->setRepository("C:/Users/Thibault/Documents/Github/GolemFactory/Resources/");
	
	// Init Renderer;
	Camera camera;
	Renderer::getInstance()->setCamera(&camera);
	Renderer::getInstance()->setWindow(window);
	Renderer::getInstance()->setDefaultShader(ResourceManager::getInstance()->getShader("default"));
	Renderer::getInstance()->initializeGrid(GRID_SIZE, GRID_ELEMENT_SIZE);

	// init scene
	SceneManager::getInstance()->setWorldPosition(glm::vec3(0,0,25));
	SceneManager::getInstance()->setWorldSize(glm::vec3(GRID_SIZE*GRID_ELEMENT_SIZE, GRID_SIZE*GRID_ELEMENT_SIZE, 50));
	initializeForestScene();
	
	// init loop time tracking
	double startTime, elapseTime = 16;

	std::cout << "game loop initiated" << std::endl;
	while (!glfwWindowShouldClose(window))
	{
		// begin loop
		startTime = glfwGetTime();
		int width, height;
		glfwGetWindowSize(window,&width,&height);
		float angle = camera.getFrustrumAngleVertical() + EventHandler::getInstance()->getScrollingRelative().y;
		if (angle > 70.f) angle = 70.f;
		else if (angle < 3) angle = 3.f;
		camera.setFrustrumAngleVertical(angle);
		camera.setFrustrumAngleHorizontalFromScreenRatio((float)width / height);

		// Render frame
		Renderer::getInstance()->render();

		//  handle events
		EventHandler::getInstance()->handleEvent();
		std::vector<EventEnum> v;
		EventHandler::getInstance()->getFrameEvent(v);
		for (unsigned int i = 0; i<v.size(); i++)
		{
			switch (v[i])
			{
				case QUIT: glfwSetWindowShouldClose(window, GL_TRUE); break;
				case CHANGE_CURSOR_MODE: EventHandler::getInstance()->setCursorMode(!EventHandler::getInstance()->getCursorMode()); break;
				default: break;
			}
		}

		//Animate camera
		camera.animate((float)elapseTime,
			EventHandler::getInstance()->isActivated(FORWARD),	EventHandler::getInstance()->isActivated(BACKWARD),
			EventHandler::getInstance()->isActivated(LEFT),		EventHandler::getInstance()->isActivated(RIGHT),
			EventHandler::getInstance()->isActivated(RUN),		EventHandler::getInstance()->isActivated(SNEAKY)    );

		//	Debug
		//std::cout << 1000.f*(glfwGetTime() - startTime) << std::endl;

		// End loop
		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		elapseTime = 1000.0*(glfwGetTime() - startTime);
	}

	//	end
	std::cout << "ending game" << std::endl;
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

//	functions implementation
static void errorCallback(int error, const char* description) { std::cerr << "GLFW ERROR : " << description << std::endl; }
GLFWwindow* initGLFW()
{
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwSetErrorCallback(errorCallback);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow*window = glfwCreateWindow(640, 480, "Golem Factory v1.0", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	return window;
}
void initGLEW(int verbose)
{
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "ERROR : " << glewGetErrorString(err) << std::endl;
		glfwTerminate();
		exit(-1);
	}
	std::cout << "GLEW init success" << std::endl;
	if (verbose < 1) return;

	std::cout << "Status: GLEW version : " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "        OpenGL version : " << glGetString(GL_VERSION) << std::endl;
	std::cout << "        OpenGL implementation vendor : " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "        Renderer name : " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "        GLSL version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void initializeForestScene()
{
	int fail = 0;
	srand((unsigned int)time(NULL));
	for (int i = 0; i < GRID_SIZE; i++)
		for (int j = 0; j < GRID_SIZE; j++)
		{
			int r = rand() % 100;
			glm::vec3 p(GRID_ELEMENT_SIZE*i - (GRID_SIZE * GRID_ELEMENT_SIZE) / 2 + ((rand() % 10) / 5.f - 1.f),
						GRID_ELEMENT_SIZE*j - (GRID_SIZE * GRID_ELEMENT_SIZE) / 2 + ((rand() % 10) / 5.f - 1.f),
						0);
			float s = 1.f + 0.2f*((rand() % 100) / 50.f - 1.f);
			glm::mat4 a = glm::rotate(glm::mat4(1.0), glm::radians((rand() % 3600) / 10.f), glm::vec3(0, 0, 1));
			InstanceDrawable* ins = nullptr;

			if (r < 20)      ins = InstanceManager::getInstance()->getInstanceDrawable("rock1.obj");
			else if (r < 80) ins = InstanceManager::getInstance()->getInstanceDrawable("firTree1.obj");
			
			if (ins)
			{
				ins->setPosition(p);
				ins->setSize(glm::vec3(s,s,s));
				ins->setOrientation(a);

				if(!SceneManager::getInstance()->addStaticObject(ins))
					fail++;
			}
		}
	std::cout << "Instance count : " << InstanceManager::getInstance()->getNumberOfInstances() << std::endl;
	std::cout << "Insert fail : " << fail << std::endl;
}
