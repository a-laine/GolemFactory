#include "Renderer.h"

//  Default
Renderer::Renderer()
{
	window = nullptr;
	camera = nullptr;
	defaultShader = nullptr;

	gridVAO = 0;
	vertexbuffer = 0;
	arraybuffer = 0;
	drawGrid = true;
	gridShader = nullptr;
	vertexBufferGrid = nullptr;
	indexBufferGrid = nullptr;

	dummy = 0.0;
}
Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &gridVAO);
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &arraybuffer);

	if (vertexBufferGrid) delete[] vertexBufferGrid;
	if (indexBufferGrid) delete[] indexBufferGrid;
}
//

//  Public functions
void Renderer::render()
{
	if (!window || !camera) return;

	dummy += 0.016;
	if (dummy >= 6.28)
		dummy = 0.0;

	// bind matrix
	glm::mat4 view = camera->getViewMatrix();

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glm::mat4 projection = glm::perspective(glm::radians(camera->getFrustrumAngleVertical()), (float)width / height, 0.1f, 1000.f);
	
	// opengl state
	glEnable(GL_DEPTH_TEST);

	//	draw grid
	if (drawGrid && gridShader && glIsVertexArray(gridVAO))
	{
		gridShader->enable();
		loadMVPMatrix(gridShader, &glm::mat4(1.0)[0][0], &view[0][0], &projection[0][0]);

		glBindVertexArray(gridVAO);
		glDrawElements(GL_TRIANGLES, vboGridSize, GL_UNSIGNED_SHORT, NULL);
	}

	//	get instance list
	SceneManager::getInstance()->setCameraAttributes(camera->getPosition(), camera->getForward(), camera->getVertical(), camera->getLeft(),
		camera->getFrustrumAngleVertical() / 1.6f, camera->getFrustrumAngleVertical() / 1.6f);
	
	std::vector<std::pair<int, InstanceVirtual*> > instanceList;
	SceneManager::getInstance()->getInstanceList(instanceList);
	std::sort(instanceList.begin(), instanceList.end());

	//	draw instance list
	Shader* shaderToUse;
	int loc;
	glm::vec4 wind(0.1*sin(dummy), 0.0, 0.0, 0.0);


	for (auto it = instanceList.begin(); it != instanceList.end(); it++)
	{
		if (InstanceDrawable* d = dynamic_cast<InstanceDrawable*>(it->second))
		{
			// Get shader
			if (defaultShader) shaderToUse = defaultShader;
			else shaderToUse = d->getShader();
			if (!shaderToUse) continue;

			// Activate shader
			shaderToUse->enable();

			// Enable mvp matrix
			loadMVPMatrix(shaderToUse, &d->getModelMatrix()[0][0], &view[0][0], &projection[0][0]);

			int loc = shaderToUse->getUniformLocation("wind");
			if (loc >= 0) glUniform4fv(loc, 1, &wind.x);

			// Draw mesh
			d->getMesh()->draw();
		}
	}
}

void Renderer::setGridVisible(bool enable)
{
	drawGrid = enable;
}
bool Renderer::gridVisible()
{
	return drawGrid;
}

void Renderer::initializeGrid(unsigned int gridSize, float elementSize)
{
	if (glIsVertexArray(gridVAO)) return;

	gridShader = ResourceManager::getInstance()->getShader("wiredGrid");
	if (!gridShader) std::cout << "loading grid shader fail" << std::endl;

	//	generate grid vertex buffer
	vertexBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	for (unsigned int i = 0; i < gridSize + 1; i++)
		for (unsigned int j = 0; j < gridSize + 1; j++)
		{
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = elementSize*i - (gridSize * elementSize) / 2;
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = elementSize*j - (gridSize * elementSize) / 2;
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = 0;
		}

	indexBufferGrid = new uint16_t[6 * gridSize*gridSize];
	for (unsigned int i = 0; i < gridSize; i++)
		for (unsigned int j = 0; j < gridSize; j++)
		{
			indexBufferGrid[6 * (i*gridSize + j) + 0] = i*(gridSize + 1) + j + (gridSize + 1);
			indexBufferGrid[6 * (i*gridSize + j) + 1] = i*(gridSize + 1) + j;
			indexBufferGrid[6 * (i*gridSize + j) + 2] = i*(gridSize + 1) + j + 1;

			indexBufferGrid[6 * (i*gridSize + j) + 3] = i*(gridSize + 1) + j + (gridSize + 1);
			indexBufferGrid[6 * (i*gridSize + j) + 4] = i*(gridSize + 1) + j + (gridSize + 1) + 1;
			indexBufferGrid[6 * (i*gridSize + j) + 5] = i*(gridSize + 1) + j + 1;
		}
	
	vboGridSize = 6 * gridSize * gridSize;

	//	initialize VBO
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * (gridSize + 1)*(gridSize + 1) * sizeof(float), vertexBufferGrid, GL_STATIC_DRAW);

	glGenBuffers(1, &arraybuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arraybuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * gridSize*gridSize * sizeof(unsigned short), indexBufferGrid, GL_STATIC_DRAW);

	//	initialize VAO
	glGenVertexArrays(1, &gridVAO);
	glBindVertexArray(gridVAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arraybuffer);

	glBindVertexArray(0);
}
//


//	Protected functions
void Renderer::loadMVPMatrix(Shader* shader, float* model, float* view, float* projection) const
{
	int loc = shader->getUniformLocation("model");
	if (loc >= 0)
		glUniformMatrix4fv(loc, 1, GL_FALSE, model);
	loc = shader->getUniformLocation("view");
	if (loc >= 0)
		glUniformMatrix4fv(loc, 1, GL_FALSE, view);
	loc = shader->getUniformLocation("projection");
	if (loc >= 0)
		glUniformMatrix4fv(loc, 1, GL_FALSE, projection);
}
//


//  Set/get functions
void Renderer::setCamera(Camera* cam)
{
	camera = cam;
}
void Renderer::setWindow(GLFWwindow* win)
{
	window = win;
}
void Renderer::setDefaultShader(Shader* shad)
{
	defaultShader = shad;
}

Camera* Renderer::getCamera()
{
	return camera;
}
GLFWwindow* Renderer::getWindow()
{
	return window;
}
Shader* Renderer::getDefaultShader()
{
	return defaultShader;
}
//
