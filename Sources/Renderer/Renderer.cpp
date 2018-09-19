#include "Renderer.h"
#include "World/World.h"
#include "Scene/SceneQueryTests.h"
#include "Resources/ComponentResource.h"
#include "EntityComponent/AnimationEngine.h"


#define BATCH_SIZE 30


//  Default
Renderer::Renderer() : renderOption(DEFAULT)
{
	window = nullptr;
	camera = nullptr;

	defaultShader[GRID] = nullptr;
	defaultShader[INSTANCE_ANIMATABLE] = nullptr;
	defaultShader[INSTANCE_DRAWABLE] = nullptr;
	defaultShader[INSTANCE_ANIMATABLE_BB] = nullptr;
	defaultShader[INSTANCE_DRAWABLE_BB] = nullptr;
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
}
void Renderer::render(Camera* renderCam)
{
	trianglesDrawn = 0;
	instanceDrawn = 0;
	lastShader = nullptr;
	if (!window || !camera || !world || !renderCam) return;
	
	// dummy animation timeline
	dummy += 0.16 / 3.f;
	if (dummy >= 6.28) dummy = 0.0;

	// bind matrix
	glm::mat4 view(renderCam->getViewMatrix());

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glm::mat4 projection(glm::perspective(glm::radians(renderCam->getFrustrumAngleVertical()), (float)width / height, 0.1f, 1500.f));
	
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
	std::vector<InstanceVirtual*> instanceList;
	DefaultSceneManagerFrustrumTest sceneTest(camera->getPosition(), camera->getForward(), camera->getVertical(), camera->getLeft(),
		camera->getFrustrumAngleVertical() / 1.6f, camera->getFrustrumAngleVertical() / 1.6f);
	world->getSceneManager().getObjects(instanceList, sceneTest);

	//	draw instance list
	std::map<Shader*, std::vector<InstanceVirtual*> > batches;
	for (InstanceVirtual* object : instanceList)
	{
		ComponentResource<Shader>* objectComponentShader = object->getComponent<ComponentResource<Shader> >();
		if (!objectComponentShader) continue;

		batches[objectComponentShader->getResource()].push_back(object);
		if (batches[objectComponentShader->getResource()].size() > BATCH_SIZE)
		{
			std::vector<InstanceVirtual*>& batch = batches[objectComponentShader->getResource()];
			for (unsigned int i = 0; i < BATCH_SIZE + 1; i++)
			{
				switch (batch[i]->getRenderingType())
				{
					case InstanceVirtual::DRAWABLE:
						drawInstanceDrawable(batch[i], &view[0][0], &projection[0][0]);
						break;
					case InstanceVirtual::ANIMATABLE:
						drawInstanceAnimatable(batch[i], &view[0][0], &projection[0][0]);
						break;
					default:
						break;
				}
				if (batch[i]->getChildList())
					drawInstanceContainer(batch[i], view, projection, glm::mat4(1.f));
			}
			batch.clear();
		}
	}
	for (auto it = batches.begin(); it != batches.end(); ++it)
	{
		std::vector<InstanceVirtual*>& batch = it->second;
		for (unsigned int i = 0; i < batch.size(); i++)
		{
			switch (batch[i]->getRenderingType())
			{
				case InstanceVirtual::DRAWABLE:
					drawInstanceDrawable(batch[i], &view[0][0], &projection[0][0]);
					break;
				case InstanceVirtual::ANIMATABLE:
					drawInstanceAnimatable(batch[i], &view[0][0], &projection[0][0]);
					break;
				default:
					break;
			}
			if(batch[i]->getChildList())
				drawInstanceContainer(batch[i], view, projection, glm::mat4(1.f));
		}
	}
}
void Renderer::renderHUD(Camera* renderCam)
{
	if (!window || !camera || !renderCam) return;

	// bind matrix
	glm::mat4 view = renderCam->getViewMatrix();

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glm::mat4 projection = glm::perspective(glm::radians(ANGLE_VERTICAL_HUD_PROJECTION), (float)width / height, 0.1f, 1500.f);
	glm::mat4 base = glm::translate(glm::mat4(1.f), DISTANCE_HUD_CAMERA*camera->getForward()) * camera->getModelMatrix();

	//	change opengl states
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	uint8_t stencilMask = 0x00;

	//	draw all widget in hudList
	auto wList = WidgetManager::getInstance()->hudList;
	for (std::map<std::string, std::vector<Layer*> >::iterator it = wList.begin(); it != wList.end(); ++it)
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			if (it->second[i]->isVisible())		// layer visible
			{
				glm::mat4 model = base * it->second[i]->getModelMatrix();
				std::vector<WidgetVirtual*>& list = it->second[i]->getChildrenList();
				for (std::vector<WidgetVirtual*>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
				{
					if ((*it2)->isVisible())	// widget visible
					{
						//	Get shader and prepare matrix
						Shader* shader = nullptr;
						if (!defaultShader[HUD]) shader = (*it2)->getShader();
						loadMVPMatrix(shader, &glm::translate(model, (*it2)->getPosition())[0][0], &view[0][0], &projection[0][0]);
						if (!shader) continue;

						//	Draw
						(*it2)->draw(shader, stencilMask, model);
						instanceDrawn++;
						trianglesDrawn += (*it2)->getNumberFaces();
					}
				}
			}
		}
	}
	glDisable(GL_BLEND);
}
//


//	Protected functions
void Renderer::loadMVPMatrix(Shader* shader, const float* model, const float* view, const float* projection)
{
	if (shader == lastShader && shader)
	{
		int loc = shader->getUniformLocation("model");
		if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, model);
	}
	else if (shader)
	{
		shader->enable();
		int loc = shader->getUniformLocation("model");
		if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, model);
		loc = shader->getUniformLocation("view");
		if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, view);
		loc = shader->getUniformLocation("projection");
		if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, projection);
		lastShader = shader;
	}
	else  lastShader = nullptr;
}
//


//	Render function
void Renderer::drawInstanceDrawable(InstanceVirtual* ins, const float* view, const float* projection, const glm::mat4& base)
{
	//	Get shader and prepare matrix
	Shader* shaderToUse;

	if (renderOption == BOUNDING_BOX) shaderToUse = defaultShader[INSTANCE_DRAWABLE_BB];
	else shaderToUse = defaultShader[INSTANCE_DRAWABLE];
	if (!shaderToUse) shaderToUse = ins->getComponent<ComponentResource<Shader> >()->getResource();
	loadMVPMatrix(shaderToUse, &(base * ins->getModelMatrix())[0][0], view, projection);
	if (!shaderToUse) return;

	//	Draw mesh
	if (renderOption == BOUNDING_BOX) ins->getComponent<ComponentResource<Mesh> >()->getResource()->drawBB();
	else ins->getComponent<ComponentResource<Mesh> >()->getResource()->draw();
	instanceDrawn++;
	trianglesDrawn += ins->getComponent<ComponentResource<Mesh> >()->getResource()->getNumberFaces();
}
void Renderer::drawInstanceAnimatable(InstanceVirtual* ins, const float* view, const float* projection)
{
	//	Get shader and prepare matrix
	Shader* shaderToUse;

	if (renderOption == BOUNDING_BOX) shaderToUse = defaultShader[INSTANCE_ANIMATABLE_BB];
	else shaderToUse = defaultShader[INSTANCE_ANIMATABLE];
	if (!shaderToUse) shaderToUse = ins->getComponent<ComponentResource<Shader> >()->getResource();
	loadMVPMatrix(shaderToUse, &ins->getModelMatrix()[0][0], view, projection);
	if (!shaderToUse) return;

	//	Load skeleton pose matrix list for vertex skinning calculation
	std::vector<glm::mat4> pose = ins->getComponent<AnimationEngine>()->getPose();
	int loc = shaderToUse->getUniformLocation("skeletonPose");
	if (loc >= 0) glUniformMatrix4fv(loc, (int)pose.size(), FALSE, (float*)pose.data());

	//	Load inverse bind pose matrix list for vertex skinning calculation
	std::vector<glm::mat4> bind;
	Skeleton* skeleton = ins->getComponent<ComponentResource<Skeleton> >()->getResource();
	if (skeleton) bind = skeleton->getInverseBindPose();
	loc = shaderToUse->getUniformLocation("inverseBindPose");
	if (loc >= 0) glUniformMatrix4fv(loc, (int)bind.size(), FALSE, (float*)bind.data());

	//	Draw mesh
	if (renderOption == BOUNDING_BOX)
	{
		/*InstanceAnimatable* tmpIns = static_cast<InstanceAnimatable*>(ins);
		std::vector<float> radius = tmpIns->getCapsules();
		int loc = shaderToUse->getUniformLocation("capsuleRadius");
		if (loc >= 0) glUniformMatrix4fv(loc, radius.size(), FALSE, (float*)radius.data());

		tmpIns->drawBB();*/
	}
	else ins->getComponent<ComponentResource<Mesh> >()->getResource()->draw();
	instanceDrawn++;
	trianglesDrawn += ins->getComponent<ComponentResource<Mesh> >()->getResource()->getNumberFaces();
}
void Renderer::drawInstanceContainer(InstanceVirtual* ins, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model)
{
	glm::mat4 modelMatrix = model * ins->getModelMatrix();
	auto instanceList = *ins->getChildList();
	for (auto it = instanceList.begin(); it != instanceList.end(); it++)
	{
		if ((*it)->getRenderingType() == InstanceVirtual::DRAWABLE)
			drawInstanceDrawable(*it, &view[0][0], &projection[0][0], modelMatrix);
		if ((*it)->getChildList())
			drawInstanceDrawable(*it, &view[0][0], &projection[0][0], modelMatrix);
	}
}
//


//  Set/get functions
void Renderer::setCamera(Camera* cam) { camera = cam; }
void Renderer::setWorld(World* currentWorld) { world = currentWorld; }
void Renderer::setWindow(GLFWwindow* win) { window = win; }
void Renderer::setShader(ShaderIdentifier id, Shader* s)
{
	std::map<ShaderIdentifier, Shader*>::iterator it = defaultShader.find(id);
	if(it != defaultShader.end()) ResourceManager::getInstance()->release(defaultShader[id]);

	if (s) defaultShader[id] = ResourceManager::getInstance()->getShader(s->name);
	else defaultShader[id] = nullptr;
}
void Renderer::setGridVisible(bool enable) { drawGrid = enable; }
void Renderer::setRenderOption(const RenderOption& option) { renderOption = option; }


Camera* Renderer::getCamera() { return camera; }
World* Renderer::getWorld() { return world; }
GLFWwindow* Renderer::getWindow() { return window; }
Shader* Renderer::getShader(ShaderIdentifier id)
{
	std::map<ShaderIdentifier, Shader*>::iterator it = defaultShader.find(id);
	if (it != defaultShader.end()) return defaultShader[id];
	else return nullptr;
}
bool Renderer::isGridVisible() { return drawGrid; }
unsigned int Renderer::getNbDrawnInstances() const { return instanceDrawn; }
unsigned int Renderer::getNbDrawnTriangles() const { return trianglesDrawn; }
Renderer::RenderOption Renderer::getRenderOption() const { return renderOption; }
//
