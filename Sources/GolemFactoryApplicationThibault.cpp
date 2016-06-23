// Golem Factory 1.0.cpp : définit le point d'entrée pour l'application console.
//

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Utiles/System.h"
#include "Events/EventHandler.h"
#include "Utiles/Camera.h"
#include "Resources/ResourceManager.h"

#define GRID_SIZE 20
#define GRID_ELEMENT_SIZE 1.0f

// prototypes
GLFWwindow* initGLFW();
void initGLEW(int verbose = 1);
void initializeGrid();


// utiles
static void errorCallback(int error, const char* description) { std::cerr << "GLFW ERROR : " << description << std::endl; }


// global variables
GLuint gridBuffer[2];

static const GLfloat g_vertex_buffer_data[] =
{
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	0.0f,  1.0f, 0.0f,
};
GLuint triangle;
void dummyTriangleInit()
{
	glGenBuffers(1, &triangle);
	glBindBuffer(GL_ARRAY_BUFFER, triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
}


// program
int main()
{
	GLFWwindow* window = initGLFW();
	initGLEW();
	//initializeGrid();

	EventHandler::getInstance()->addWindow(window);
	EventHandler::getInstance()->reload("C:/Users/Thibault-SED/Documents/Github/GolemFactory/Resources/", "RPG Key mapping");
	//EventHandler::getInstance()->reload("C:/Users/Thibault-SED/Documents/Github/GolemFactory/Resources/", "RPG Key mapping");
	EventHandler::getInstance()->setCursorMode(false);

	ResourceManager::getInstance()->setRepository("C:/Users/Thibault-SED/Documents/Github/GolemFactory/Resources/");
	Shader* defaultShader = ResourceManager::getInstance()->getShader("default");

	GLuint M, V, P;
	if (defaultShader)
	{
		defaultShader->enable();
		M = glGetUniformLocation(defaultShader->getProgram(), "m");
		V = glGetUniformLocation(defaultShader->getProgram(), "v");
		P = glGetUniformLocation(defaultShader->getProgram(), "p");
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		glUniformMatrix4fv(M, 1, GL_FALSE, &modelMatrix[0][0]);
	}
	Camera camera;
	double startTime, elapseTime = 16;

	std::cout << "end init" << std::endl;
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	while (!glfwWindowShouldClose(window))
	{
		// begin loop
		startTime = glfwGetTime();

		defaultShader->enable();
		// bind matrix
		glm::mat4 viewMatrix = camera.getViewMatrix();
		glUniformMatrix4fv(V, 1, GL_FALSE, &viewMatrix[0][0]);

		int width = 1, height = 1;
		glfwGetWindowSize(window, &width, &height);
		glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera.getFrustrumAngleVertical()), (float)width / (float)height, 0.1f, 100.0f);
		glUniformMatrix4fv(P, 1, GL_FALSE, &projectionMatrix[0][0]);

		// draw grid
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, gridBuffer[0]);
		glVertexPointer(3, GL_FLOAT, 6 * sizeof(float), ((float*)NULL + (3)));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridBuffer[1]);
		glDrawElements(GL_TRIANGLES, GRID_SIZE*GRID_SIZE, GL_UNSIGNED_INT, 0);
		glDisableVertexAttribArray(0);

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
			EventHandler::getInstance()->isActivated(FORWARD), EventHandler::getInstance()->isActivated(BACKWARD),
			EventHandler::getInstance()->isActivated(LEFT), EventHandler::getInstance()->isActivated(RIGHT),
			EventHandler::getInstance()->isActivated(RUN), EventHandler::getInstance()->isActivated(SNEAKY));

		// End loop
		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		elapseTime = 1000.0*(glfwGetTime() - startTime);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

// functions implementation
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


void initializeGrid()
{
	float vertexBufferGrid[3 * (GRID_SIZE + 1)*(GRID_SIZE + 1)];
	for (int i = 0; i < GRID_SIZE + 1; i++)
		for (int j = 0; j < GRID_SIZE + 1; j++)
		{
			vertexBufferGrid[3 * (i*(GRID_SIZE + 1) + j) + 0] = i - (GRID_SIZE * GRID_ELEMENT_SIZE) / 2;
			vertexBufferGrid[3 * (i*(GRID_SIZE + 1) + j) + 1] = j - (GRID_SIZE * GRID_ELEMENT_SIZE) / 2;
			vertexBufferGrid[3 * (i*(GRID_SIZE + 1) + j) + 2] = 0;
		}

	uint16_t indexBufferGrid[6 * GRID_SIZE*GRID_SIZE];
	for (int i = 0; i < GRID_SIZE; i++)
		for (int j = 0; j < GRID_SIZE; j++)
		{
			indexBufferGrid[6 * (i*GRID_SIZE + j) + 0] = i*(GRID_SIZE + 1) + j;
			indexBufferGrid[6 * (i*GRID_SIZE + j) + 1] = (i + 1)*(GRID_SIZE + 1) + j;
			indexBufferGrid[6 * (i*GRID_SIZE + j) + 2] = i*(GRID_SIZE + 1) + (j + 1);

			indexBufferGrid[6 * (i*GRID_SIZE + j) + 3] = (i + 1)*(GRID_SIZE + 1) + j;
			indexBufferGrid[6 * (i*GRID_SIZE + j) + 4] = (i + 1)*(GRID_SIZE + 1) + (j + 1);
			indexBufferGrid[6 * (i*GRID_SIZE + j) + 5] = i*(GRID_SIZE + 1) + (j + 1);
		}

	glGenBuffers(2, gridBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gridBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferGrid), vertexBufferGrid, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridBuffer[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexBufferGrid), indexBufferGrid, GL_STATIC_DRAW);
}