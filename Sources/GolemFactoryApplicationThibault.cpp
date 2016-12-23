// Golem Factory 1.0.cpp : define consol application entry point
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
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>

#define GRID_SIZE 100
#define GRID_ELEMENT_SIZE 3.0f

// prototypes
GLFWwindow* initGLFW();
void initGLEW(int verbose = 1);
void initializeForestScene();
//


//
void generateRagdoll(Skeleton* s);
//


// program
int main()
{
	// init window and opengl
	GLFWwindow* window = initGLFW();
	initGLEW(0);

	// Init Event handler
	EventHandler::getInstance()->addWindow(window);
	EventHandler::getInstance()->setRepository("C:/Users/Thibault-SED/Documents/Github/GolemFactory/Resources/");
	//EventHandler::getInstance()->setRepository("C:/Users/Thibault/Documents/Github/GolemFactory/Resources/");
	//EventHandler::getInstance()->setRepository("Resources/");
	EventHandler::getInstance()->loadKeyMapping("RPG Key mapping");
	EventHandler::getInstance()->setCursorMode(false);
	
	// Init Resources manager and load some default shader
	ResourceManager::getInstance()->setRepository("C:/Users/Thibault-SED/Documents/Github/GolemFactory/Resources/");
	//ResourceManager::getInstance()->setRepository("C:/Users/Thibault/Documents/Github/GolemFactory/Resources/");
	//ResourceManager::getInstance()->setRepository("Resources/");
	ResourceManager::getInstance()->getFont("default");
	
	// Init Renderer;
	Camera camera;
		camera.setPosition(glm::vec3(-3,0,3));
	Renderer::getInstance()->setCamera(&camera);
	Renderer::getInstance()->setWindow(window);
	//Renderer::getInstance()->setDefaultShader(ResourceManager::getInstance()->getShader("default"));
	Renderer::getInstance()->initializeGrid(GRID_SIZE, GRID_ELEMENT_SIZE);
	ResourceManager::getInstance()->getShader("tree");

	// init scene
	SceneManager::getInstance()->setWorldPosition(glm::vec3(0,0,25));
	SceneManager::getInstance()->setWorldSize(glm::vec3(GRID_SIZE*GRID_ELEMENT_SIZE, GRID_SIZE*GRID_ELEMENT_SIZE, 50));

	//generateRagdoll(ResourceManager::getInstance()->getSkeleton("default"));
	//InstanceDrawable* firtree = InstanceManager::getInstance()->getInstanceDrawable("firTree1.obj","tree");
	//SceneManager::getInstance()->addStaticObject(firtree);

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
		std::vector<UserEventType> v;
		EventHandler::getInstance()->getFrameEvent(v);
		for (unsigned int i = 0; i<v.size(); i++)
		{
			if(v[i] == QUIT) glfwSetWindowShouldClose(window, GL_TRUE);
			else if(v[i] == CHANGE_CURSOR_MODE) EventHandler::getInstance()->setCursorMode(!EventHandler::getInstance()->getCursorMode());
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
//

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
	if (verbose) std::cout << "GLEW init success" << std::endl;
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
	std::string meshName;
	std::string shaderName;
	srand((unsigned int)time(NULL));

	for (int i = 0; i < GRID_SIZE; i++)
		for (int j = 0; j < GRID_SIZE; j++)
		{
			int r = rand() % 100;
			if (r < 20)
			{
				meshName = "rock1.obj";
			}
			else if (r < 80)
			{
				meshName = "firTree1.obj";
				shaderName = "tree";
			}
			else continue;

			glm::vec3 p(GRID_ELEMENT_SIZE*i - (GRID_SIZE * GRID_ELEMENT_SIZE) / 2 + ((rand() % 10) / 5.f - 1.f),
						GRID_ELEMENT_SIZE*j - (GRID_SIZE * GRID_ELEMENT_SIZE) / 2 + ((rand() % 10) / 5.f - 1.f),
						0);
			float s = 1.f + 0.2f*((rand() % 100) / 50.f - 1.f);
			glm::mat4 a = glm::rotate(glm::mat4(1.0), glm::radians((rand() % 3600) / 10.f), glm::vec3(0, 0, 1));
			InstanceDrawable* ins = InstanceManager::getInstance()->getInstanceDrawable();
			if (!ins) continue;

			ins->setMesh(meshName);
			ins->setShader(shaderName);
			
			ins->setPosition(p);
			ins->setSize(glm::vec3(s,s,s));
			ins->setOrientation(a);

			if(!SceneManager::getInstance()->addStaticObject(ins))
				fail++;
		}
	std::cout << "Instance count : " << InstanceManager::getInstance()->getNumberOfInstances() << std::endl;
	std::cout << "Insert fail : " << fail << std::endl;
}
//

//
void createBone(Skeleton* s, unsigned int joint, glm::vec3 parentPosition)
{
	InstanceDrawable* ins = InstanceManager::getInstance()->getInstanceDrawable("cube2.obj");
	if (!ins) return;

	if (joint == s->root)
	{
		ins->setSize(glm::vec3(0, 0, 0));
		ins->setPosition(parentPosition);
	}
	else
	{
		ins->setSize(glm::vec3(0, 0, 0));
		ins->setPosition(parentPosition);

		ins->setPosition(parentPosition + 0.5f * s->jointList[joint].position);
		ins->setSize(glm::vec3(0.1, 0.1, 0.48f * glm::length(s->jointList[joint].position)));
		glm::vec3 v = glm::normalize(s->jointList[joint].position);
		if (glm::dot(v, glm::vec3(0, 0, 1)) != 1.f)
			ins->setOrientation(glm::orientation(-v, glm::vec3(0, 0, 1)));
		//s->jointList[joint].orientation = glm::quat_cast(glm::orientation(-v, glm::vec3(0, 0, 1)));

		if (s->jointList[joint].sons.empty())
		{
			InstanceDrawable* leaf = InstanceManager::getInstance()->getInstanceDrawable("icosphere.obj");

			leaf->setSize(glm::vec3(0.3, 0.3, 0.3));
			leaf->setPosition(parentPosition + s->jointList[joint].position);
			SceneManager::getInstance()->addStaticObject(leaf);
		}
	}
	SceneManager::getInstance()->addStaticObject(ins);

	for (unsigned int i = 0; i < s->jointList[joint].sons.size(); i++)
		createBone(s, s->jointList[joint].sons[i], parentPosition + s->jointList[joint].position);
}
void generateRagdoll(Skeleton* s)
{
	std::cout << InstanceManager::getInstance()->getNumberOfInstances() << std::endl;
	createBone(s, s->root, glm::vec3(0, 0, 3));

	std::cout << InstanceManager::getInstance()->getNumberOfInstances()<< std::endl;
}
//

