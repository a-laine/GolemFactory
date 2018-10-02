#include "Renderer.h"

#include <iostream>

#include <EntityComponent/Entity.hpp>
#include <HUD/WidgetManager.h>
#include <Scene/SceneQueryTests.h>
#include <Resources/ResourceManager.h>
#include <Resources/Skeleton.h>
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>


#define BATCH_SIZE 30


//  Default
Renderer::Renderer() : renderOption(DEFAULT)
{
	window = nullptr;
	camera = nullptr;
	world = nullptr;

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

	defaultShader[GRID] = ResourceManager::getInstance()->getResource<Shader>("wired");

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
	std::vector<Entity*> instanceList;
	DefaultSceneManagerFrustrumTest sceneTest(camera->getPosition(), camera->getForward(), camera->getVertical(), camera->getLeft(),
		camera->getFrustrumAngleVertical() / 1.6f, camera->getFrustrumAngleVertical() / 1.6f);
	world->getSceneManager().getObjects(instanceList, sceneTest);

	//	draw instance list
	std::map<Shader*, std::vector<Entity*> > batches;
	for (Entity* object : instanceList)
	{
		DrawableComponent* comp = object->getComponent<DrawableComponent>();
		if(!comp || !comp->isValid()) continue;

		batches[comp->getShader()].push_back(object);
		if (batches[comp->getShader()].size() > BATCH_SIZE)
		{
			std::vector<Entity*>& batch = batches[comp->getShader()];
			for (unsigned int i = 0; i < BATCH_SIZE + 1; i++)
			{
				drawObject(batch[i], &view[0][0], &projection[0][0]);
			}
			batch.clear();
		}
	}
	for (auto it = batches.begin(); it != batches.end(); ++it)
	{
		std::vector<Entity*>& batch = it->second;
		for (unsigned int i = 0; i < batch.size(); i++)
		{
			drawObject(batch[i], &view[0][0], &projection[0][0]);
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
void Renderer::drawObject(Entity* object, const float* view, const float* projection)
{
	ShaderIdentifier shaderType = INSTANCE_DRAWABLE;
	ShaderIdentifier shaderBBType = INSTANCE_DRAWABLE_BB;
	DrawableComponent* drawableComp = object->getComponent<DrawableComponent>();
	SkeletonComponent* skeletonComp = object->getComponent<SkeletonComponent>();
	if(!drawableComp || !drawableComp->isValid()) return;
	if(skeletonComp && skeletonComp->isValid())
	{
		shaderType = INSTANCE_ANIMATABLE;
		shaderBBType = INSTANCE_ANIMATABLE_BB;
	}

	//	Get shader and prepare matrix
	Shader* shaderToUse;
	if(renderOption == BOUNDING_BOX) shaderToUse = defaultShader[shaderBBType];
	else shaderToUse = defaultShader[shaderType];
	if(!shaderToUse) shaderToUse = drawableComp->getShader();
	loadMVPMatrix(shaderToUse, &object->getMatrix()[0][0], view, projection);
	if(!shaderToUse) return;

	if(skeletonComp && skeletonComp->isValid())
	{
		//	Load skeleton pose matrix list for vertex skinning calculation
		std::vector<glm::mat4> pose = skeletonComp->getPose();
		int loc = shaderToUse->getUniformLocation("skeletonPose");
		if(loc >= 0) glUniformMatrix4fv(loc, (int)pose.size(), FALSE, (float*) pose.data());

		//	Load inverse bind pose matrix list for vertex skinning calculation
		std::vector<glm::mat4> bind;
		bind = skeletonComp->getInverseBindPose();
		loc = shaderToUse->getUniformLocation("inverseBindPose");
		if(loc >= 0) glUniformMatrix4fv(loc, (int)bind.size(), FALSE, (float*) bind.data());
	}

	//	Draw mesh
	if(renderOption == BOUNDING_BOX) drawableComp->getMesh()->drawBB();
	else drawableComp->getMesh()->draw();
	instanceDrawn++;
	trianglesDrawn += drawableComp->getMesh()->getNumberFaces();
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

	if (s) defaultShader[id] = ResourceManager::getInstance()->getResource<Shader>(s);
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
