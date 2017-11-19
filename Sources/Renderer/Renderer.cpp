#include "Renderer.h"

//  Default
Renderer::Renderer()
{
	window = nullptr;
	camera = nullptr;

	defaultShader[GRID] = nullptr;
	defaultShader[INSTANCE_ANIMATABLE] = nullptr;
	defaultShader[INSTANCE_DRAWABLE] = nullptr;
	defaultShader[HUD] = nullptr;

	gridVAO = 0;
	vertexbuffer = 0;
	arraybuffer = 0;
	drawGrid = true;

	dummy = 0.0;
}
Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &gridVAO);
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &arraybuffer);
}
//

//  Public functions
void Renderer::initGLEW(int verbose)
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
void Renderer::initializeGrid(const unsigned int& gridSize,const float& elementSize, const glm::vec3& color)
{
	if (glIsVertexArray(gridVAO)) return;

	defaultShader[GRID] = ResourceManager::getInstance()->getShader("wired");

	//	generate grid vertex buffer
	float* vertexBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	float* colorBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	float* normalBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	for (unsigned int i = 0; i < gridSize + 1; i++)
		for (unsigned int j = 0; j < gridSize + 1; j++)
		{
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = elementSize*i - (gridSize * elementSize) / 2;
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = elementSize*j - (gridSize * elementSize) / 2;
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = 0;

			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = 0.f;
			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = 0.f;
			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = 1.f;

			colorBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = color.x;
			colorBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = color.y;
			colorBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = color.z;
		}

	uint32_t* indexBufferGrid = new uint32_t[6 * gridSize*gridSize];
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

	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * (gridSize + 1)*(gridSize + 1) * sizeof(float), colorBufferGrid, GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * (gridSize + 1)*(gridSize + 1) * sizeof(float), normalBufferGrid, GL_STATIC_DRAW);

	glGenBuffers(1, &arraybuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arraybuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * gridSize*gridSize * sizeof(unsigned int), indexBufferGrid, GL_STATIC_DRAW);

	//	initialize VAO
	glGenVertexArrays(1, &gridVAO);
	glBindVertexArray(gridVAO);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arraybuffer);
	glBindVertexArray(0);

	//	free grid resources
	delete[] vertexBufferGrid;
	delete[] colorBufferGrid;
	delete[] normalBufferGrid;
	delete[] indexBufferGrid;

	//	dummy
	dummyPlaceHolder = new WidgetImage("10points.png");
		dummyPlaceHolder->setSize(glm::vec2(1.f, 1.f));
		dummyPlaceHolder->initialize();
	WidgetBoard* board = new WidgetBoard();
		board->setPosition(glm::vec3(0.f, -0.05f, 0.f));
		board->setSize(glm::vec2(1.f,0.5f));
		board->initialize(0.02f, 0.1f, WidgetBoard::TOP_LEFT | WidgetBoard::TOP_RIGHT | WidgetBoard::BOTTOM_RIGHT | WidgetBoard::BOTTOM_LEFT);
		board->setColor(glm::vec4(0.5f, 0.f, 0.2f, 1.f));
	dummyLayer = new Layer();
		dummyLayer->setSize(0.05f);
		dummyLayer->setPosition(glm::vec3(0.f, 0.f, 0.f));
		dummyLayer->add(dummyPlaceHolder);
		dummyLayer->add(board);
}
void Renderer::render()
{
	if (!window || !camera) return;
	
	// dummy animation timeline
	dummy += 0.16 / 3.f;
	if (dummy >= 6.28) dummy = 0.0;

	// bind matrix
	glm::mat4 view = camera->getViewMatrix();

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glm::mat4 projection = glm::perspective(glm::radians(camera->getFrustrumAngleVertical()), (float)width / height, 0.1f, 1500.f);
	
	// opengl state
	glEnable(GL_DEPTH_TEST);

	//	draw grid
	Shader* s = defaultShader[GRID];
	if (drawGrid && s && glIsVertexArray(gridVAO))
	{
		s->enable();
		loadMVPMatrix(s, &glm::mat4(1.0)[0][0], &view[0][0], &projection[0][0]);

		glBindVertexArray(gridVAO);
		glDrawElements(GL_TRIANGLES, vboGridSize, GL_UNSIGNED_INT, NULL);
	}

	//	get instance list
	SceneManager::getInstance()->setCameraAttributes(camera->getPosition(), camera->getForward(), camera->getVertical(), camera->getLeft(),
		camera->getFrustrumAngleVertical() / 1.6f, camera->getFrustrumAngleVertical() / 1.6f);
	
	std::vector<std::pair<int, InstanceVirtual*> > instanceList;
	SceneManager::getInstance()->getInstanceList(instanceList);
	std::sort(instanceList.begin(), instanceList.end());

	//	draw instance list
	unsigned int drawnInstance = 0;
	for (auto it = instanceList.begin(); it != instanceList.end() && drawnInstance < 8000; it++, drawnInstance++)
	{
		switch (it->second->getType())
		{
			case InstanceVirtual::DRAWABLE:
				drawInstanceDrawable(it->second, &view[0][0], &projection[0][0]);
				break;
			case InstanceVirtual::ANIMATABLE:
				drawInstanceAnimatable(it->second, &view[0][0], &projection[0][0]);
				break;
			case InstanceVirtual::CONTAINER:
				drawInstanceContainer(it->second, view, projection, glm::mat4(1.f));
				break;
			default:
				break;
		}
	}

	//	HUD
	dummyLayer->update(16);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);

	projection = glm::perspective(glm::radians(45.f), (float)width / height, 0.1f, 1500.f);
	glm::mat4 model = glm::translate(glm::mat4(1.f), 0.15f*camera->getForward()) * camera->getModelMatrix();
	if(dummyLayer->isVisible())
		drawLayer(dummyLayer, model, &view[0][0], &projection[0][0]);

	glDisable(GL_BLEND);
}
//


//	Protected functions
void Renderer::loadMVPMatrix(Shader* shader, const float* model, const float* view, const float* projection) const
{
	int loc = shader->getUniformLocation("model");
	if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, model);
	loc = shader->getUniformLocation("view");
	if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, view);
	loc = shader->getUniformLocation("projection");
	if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, projection);
}
void Renderer::drawInstanceDrawable(InstanceVirtual* ins, const float* view, const float* projection)
{
	//	Get shader
	Shader* shaderToUse = defaultShader[INSTANCE_DRAWABLE];
	if (!shaderToUse) shaderToUse = ins->getShader();
	if (!shaderToUse) return;
	shaderToUse->enable();

	//	Enable mvp matrix
	loadMVPMatrix(shaderToUse, &ins->getModelMatrix()[0][0], view, projection);

	//	Map with wind if instance sensible to wind
	int loc = shaderToUse->getUniformLocation("wind");
	if (loc >= 0)
	{
		double phase = 0.05*ins->getPosition().x + 0.05*ins->getSize().z;
		glm::vec4 wind(0.1 * sin(dummy + phase), 0.0, 0.0, 0.0);
		glUniform4fv(loc, 1, &wind.x);
	}

	//	Draw mesh
	ins->getMesh()->draw();

	/*
	glm::mat4 model = glm::translate(glm::mat4(1.f), ins->getPosition());
	model = glm::translate(model, glm::vec3(0.f, 0.f, ins->getSize().z * (1.f + ins->getMesh()->sizeZ.y - ins->getMesh()->sizeZ.x))) * camera->getOrientationMatrix();
	drawWidgetVirtual(dummyPlaceHolder, &model[0][0], view, projection);
	*/
}
void Renderer::drawInstanceAnimatable(InstanceVirtual* ins, const float* view, const float* projection)
{
	//	Get shader
	Shader* shaderToUse = defaultShader[INSTANCE_ANIMATABLE];
	if (!shaderToUse) shaderToUse = ins->getShader();
	if (!shaderToUse) return;
	shaderToUse->enable();

	//	Enable mvp matrix
	loadMVPMatrix(shaderToUse, &ins->getModelMatrix()[0][0], view, projection);

	//	Load skeleton pose matrix list for vertex skinning calculation
	std::vector<glm::mat4> pose = ins->getPose();
	int loc = shaderToUse->getUniformLocation("skeletonPose");
	if (loc >= 0) glUniformMatrix4fv(loc, pose.size(), FALSE, (float*)pose.data());

	//	Load inverse bind pose matrix list for vertex skinning calculation
	std::vector<glm::mat4> bind;
	Skeleton* skeleton = ins->getSkeleton();
	if (skeleton) bind = skeleton->getInverseBindPose();
	loc = shaderToUse->getUniformLocation("inverseBindPose");
	if (loc >= 0) glUniformMatrix4fv(loc, bind.size(), FALSE, (float*)bind.data());

	//	Draw mesh
	ins->getMesh()->draw();

	/*
	glm::mat4 model = glm::translate(glm::mat4(1.f), ins->getPosition());
	model = glm::translate(model, glm::vec3(0.f, 0.f, ins->getSize().z * (1.f + ins->getMesh()->sizeZ.y - ins->getMesh()->sizeZ.x))) * camera->getOrientationMatrix();
	drawWidgetVirtual(dummyPlaceHolder, &model[0][0], view, projection);
	*/
}
void Renderer::drawInstanceContainer(InstanceVirtual* ins, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model)
{
	glm::mat4 modelMatrix = model * ins->getModelMatrix();
	auto instanceList = *ins->getChildList();
	for (auto it = instanceList.begin(); it != instanceList.end(); it++)
	{
		if (InstanceDrawable* d = dynamic_cast<InstanceDrawable*>(*it))
		{
			// Get shader
			Shader* shaderToUse = defaultShader[INSTANCE_DRAWABLE];
			if (!shaderToUse) shaderToUse = d->getShader();
			if (!shaderToUse) continue;
			shaderToUse->enable();

			// Enable mvp matrix
			loadMVPMatrix(shaderToUse, &(modelMatrix * d->getModelMatrix())[0][0], &view[0][0], &projection[0][0]);
			d->getMesh()->draw();
		}
		else if (InstanceContainer* d = dynamic_cast<InstanceContainer*>(*it))
			drawInstanceContainer(d, view, projection, glm::mat4(1.f));
	}
}


void Renderer::drawWidgetVirtual(WidgetVirtual* widget, const glm::mat4& modelBase, const float* view, const float* projection)
{
	//	Get shader
	Shader* shaderToUse = defaultShader[HUD];
	if (!shaderToUse) shaderToUse = widget->getShader();
	if (!shaderToUse) return;
	shaderToUse->enable();

	//	Enable mvp matrix
	loadMVPMatrix(shaderToUse, &glm::translate(modelBase, widget->getPosition())[0][0], view, projection);

	//	Draw 
	widget->draw(shaderToUse);
}
void Renderer::drawLayer(Layer* layer, const glm::mat4& modelBase, const float* view, const float* projection)
{
	std::vector<WidgetVirtual*>& list = layer->getWidgetList();
	for (std::vector<WidgetVirtual*>::iterator it = list.begin(); it != list.end(); ++it)
	{
		if((*it)->isVisible())
			drawWidgetVirtual(*it, modelBase * layer->getModelMatrix(), view, projection);
	}
}
//


//  Set/get functions
void Renderer::setCamera(Camera* cam) { camera = cam; }
void Renderer::setWindow(GLFWwindow* win) { window = win; }
void Renderer::setShader(ShaderIdentifier id, Shader* s)
{
	std::map<ShaderIdentifier, Shader*>::iterator it = defaultShader.find(id);
	if(it != defaultShader.end()) ResourceManager::getInstance()->release(defaultShader[id]);

	if (s) defaultShader[id] = ResourceManager::getInstance()->getShader(s->name);
	else defaultShader[id] = nullptr;
}
void Renderer::setGridVisible(bool enable) { drawGrid = enable; }


Camera* Renderer::getCamera() { return camera; }
GLFWwindow* Renderer::getWindow() { return window; }
Shader* Renderer::getShader(ShaderIdentifier id)
{
	std::map<ShaderIdentifier, Shader*>::iterator it = defaultShader.find(id);
	if (it != defaultShader.end()) return defaultShader[id];
	else return nullptr;
}
bool Renderer::isGridVisible() { return drawGrid; }
//
