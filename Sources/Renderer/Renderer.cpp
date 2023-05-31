#include "Renderer.h"

#include <iostream>
#include <sstream>

#include <EntityComponent/Entity.hpp>
#include <HUD/WidgetManager.h>
#include <Scene/FrustrumSceneQuerry.h>
#include <Scene/FrustrumEntityCollector.h>
#include <Resources/ResourceManager.h>
#include <Resources/Skeleton.h>
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Renderer/Lighting/LightComponent.h>
#include <Utiles/Debug.h>


#define MAX_INSTANCE 32

#ifdef USE_IMGUI
bool RenderingWindowEnable = true;
#endif


//  Default
Renderer::Renderer() : 
	normalViewer(nullptr), renderOption(RenderOption::DEFAULT),
	vboGridSize(0), gridVAO(0), vertexbuffer(0), arraybuffer(0), colorbuffer(0), normalbuffer(0),
	instanceDrawn(0), trianglesDrawn(0), lastShader(nullptr)
{
	context = nullptr;
	camera = nullptr;
	world = nullptr;

	defaultShader[GRID] = nullptr;
	defaultShader[INSTANCE_ANIMATABLE_BB] = nullptr;
	defaultShader[INSTANCE_DRAWABLE_BB] = nullptr;
	defaultShader[HUD] = nullptr;

	lightClustering = nullptr;

	drawGrid = true;
	batchFreePool.reserve(512);

	initGlobalUniformBuffers();
	updateGlobalUniformBuffers();
}
Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &gridVAO);
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &arraybuffer);

	glDeleteBuffers(1, &m_environementLightingID);
	glDeleteBuffers(1, &m_lightsID);
	glDeleteBuffers(1, &m_globalMatricesID);
}
//

//  Public functions
void Renderer::initializeGrid(const unsigned int& gridSize,const float& elementSize, const vec4f& color)
{
	if (glIsVertexArray(gridVAO)) return;

	defaultShader[GRID] = ResourceManager::getInstance()->getResource<Shader>("wired");
	m_gridColor = color;

	//	generate grid vertex buffer
	float* vertexBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	float* normalBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	for (unsigned int i = 0; i < gridSize + 1; i++)
		for (unsigned int j = 0; j < gridSize + 1; j++)
		{
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = elementSize*i - (gridSize * elementSize) / 2;
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = 0;
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = elementSize*j - (gridSize * elementSize) / 2;

			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = 0.f;
			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = 1.f;
			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = 0.f;
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
	delete[] normalBufferGrid;
	delete[] indexBufferGrid;
}
void Renderer::initializeLightClusterBuffer(int width, int height, int depth)
{
	imageID = 0;
	imageSize = vec3i(16,9,32);

	const auto CheckError = [](const char* label)
	{
		GLenum error = glGetError();
		if (!label)
			return;
		switch (error)
		{
			case GL_INVALID_ENUM: std::cout << label << " : GL_INVALID_ENUM" << std::endl; break;
			case GL_INVALID_VALUE: std::cout << label << " : GL_INVALID_VALUE" << std::endl; break;
			case GL_INVALID_OPERATION: std::cout << label << " : GL_INVALID_OPERATION" << std::endl; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: std::cout << label << " : GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl; break;
			case GL_OUT_OF_MEMORY: std::cout << label << " : GL_OUT_OF_MEMORY" << std::endl; break;
			case GL_STACK_UNDERFLOW: std::cout << label << " : GL_STACK_UNDERFLOW" << std::endl; break;
			case GL_STACK_OVERFLOW: std::cout << label << " : GL_STACK_OVERFLOW" << std::endl; break;
			default: break;
		}
	};

	CheckError(nullptr);
	glGenTextures(1, &imageID); CheckError("glGenTextures");
	glBindTexture(GL_TEXTURE_3D, imageID); CheckError("glBindTexture");

	const int length = imageSize.x * imageSize.y * imageSize.z * 4;
	uint32_t* buffer = new uint32_t[length];
	for (int i = 0; i < imageSize.x; i++)
		for (int j = 0; j < imageSize.y; j++)
			for (int k = 0; k < imageSize.z; k++)
			{
				int id = i * imageSize.y * imageSize.z * 4 + j * imageSize.z * 4 + k * 4;

				buffer[id] = 0xFFFFFFFF;
				buffer[id + 1] = 0xFFFFFFFF;
				buffer[id + 2] = 0xFFFFFFFF;
				buffer[id + 3] = 0xFFFFFFFF;
			}

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32UI, imageSize.x, imageSize.y, imageSize.z, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, buffer); CheckError("glTexImage3D");
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CheckError("glTexParameteri WRAP_S");
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CheckError("glTexParameteri WRAP_T");
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); CheckError("glTexParameteri WRAP_R");
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); CheckError("glTexParameteri MAG_FILTER");
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); CheckError("glTexParameteri MIN_FILTER");
	glBindTexture(GL_TEXTURE_3D, 0);
	delete[] buffer;


	m_sceneLights.m_near = 2.f;
	m_sceneLights.m_far = 1500.f;
	float logRatio = imageSize.z / log(m_sceneLights.m_far / m_sceneLights.m_near);
	m_sceneLights.m_clusterDepthScale = logRatio;
	m_sceneLights.m_clusterDepthBias = logRatio * log(m_sceneLights.m_near);
}
void Renderer::initGlobalUniformBuffers()
{
	// global matrices and camera position
	m_globalMatrices.view = mat4f::identity;
	m_globalMatrices.projection = mat4f::identity;
	m_globalMatrices.cameraPosition = vec4f(0, 0, 0, 1);

	glGenBuffers(1, &m_globalMatricesID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_globalMatricesID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_globalMatrices), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_globalMatricesID, 0, sizeof(m_globalMatrices));

	// environment lighting settings
	m_environementLighting.m_directionalLightDirection = vec4f(-10, -20, 10, 0);
	m_environementLighting.m_directionalLightColor = vec4f(0.15, 0.15, 0.15, 1.0);
	m_environementLighting.m_ambientColor = vec4f(0.05, 0.05, 0.05, 1.0);

	glGenBuffers(1, &m_environementLightingID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_environementLightingID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_environementLighting), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_environementLightingID, 0, sizeof(m_environementLighting));

	// lights data
	glGenBuffers(1, &m_lightsID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_lightsID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_sceneLights), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 2, m_lightsID, 0, sizeof(m_sceneLights));

	// end
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void Renderer::updateGlobalUniformBuffers()
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_globalMatricesID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_globalMatrices), &m_globalMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, m_environementLightingID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_environementLighting), &m_environementLighting);


#if 0
	bool needLightUpdate = false;
	for (int i = 0; i < m_lightComponents.size() && !needLightUpdate; i++)
		needLightUpdate |= m_lightComponents[i]->m_isUniformBufferDirty;

	if (needLightUpdate)
	{
		m_sceneLights.m_lightCount = 0;
		for (int i = 0; i < MAX_LIGHT_COUNT && i < m_lightComponents.size(); i++)
		{
			m_sceneLights.m_lights[i].m_color = m_lightComponents[i]->m_color;
			m_sceneLights.m_lights[i].m_position = m_lightComponents[i]->getPosition();
			m_sceneLights.m_lights[i].m_direction = m_lightComponents[i]->isPointLight() ? vec4f(0.f) : m_lightComponents[i]->getDirection();
			m_sceneLights.m_lights[i].m_range = m_lightComponents[i]->m_range;
			m_sceneLights.m_lights[i].m_intensity = m_lightComponents[i]->m_intensity;
			m_sceneLights.m_lights[i].m_inCutOff = cos((float)DEG2RAD * m_lightComponents[i]->m_innerCutoffAngle);
			m_sceneLights.m_lights[i].m_outCutOff = cos((float)DEG2RAD * m_lightComponents[i]->m_outerCutoffAngle);
			m_lightComponents[i]->m_isUniformBufferDirty = false;
			m_sceneLights.m_lightCount++;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, m_lightsID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, m_sceneLights.m_lightCount * sizeof(Light) + sizeof(vec4i), &m_sceneLights);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
#endif
}
void Renderer::CPULightClustering()
{
	const float nearFarRatio = m_sceneLights.m_far / m_sceneLights.m_near;
	const vec3f invImageSize = vec3f(1.f / imageSize.x, 1.f / imageSize.y, 1.f / imageSize.z);
	const int length = imageSize.x * imageSize.y * imageSize.z;
	if (!cpuClusterBuffer)
		cpuClusterBuffer = new vec4ui[length];

#ifdef USE_IMGUI
	if (!clustersMin)
	{
		clustersMin = new vec4f[length];
		clustersMax = new vec4f[length];
	}
#endif

	for (int i = 0; i < imageSize.x; i++)
		for (int j = 0; j < imageSize.y; j++)
			for (int k = 0; k < imageSize.z; k++)
			{
				vec4ui clusterLightMask = vec4ui(0xFFFFFFFF);
				vec4f cellCornerMin, cellCornerMax;
				if (m_lightClustering && m_sceneLights.m_lightCount > 0)
				{
					clusterLightMask = vec4ui(0);
					float maxDepthBound = -pow(nearFarRatio, float(k + 1) * invImageSize.z) * m_sceneLights.m_near;
					float minDepthBound;
					if (k == 0)
						minDepthBound = 1.0;
					else
						minDepthBound = -pow(nearFarRatio, float(k) * invImageSize.z) * m_sceneLights.m_near;

					vec2f frustrumSize = vec2f(-m_sceneLights.m_tanFovX * maxDepthBound, -m_sceneLights.m_tanFovY * maxDepthBound);
					vec2f cellSize = 2.0f * vec2f(frustrumSize.x * invImageSize.x, frustrumSize.y * invImageSize.y);

					cellCornerMin = vec4f(i * cellSize.x - frustrumSize.x, j * cellSize.y - frustrumSize.y, maxDepthBound, 1);
					cellCornerMax = vec4f(cellCornerMin.x + cellSize.x, cellCornerMin.y + cellSize.y, minDepthBound, 1);
					vec4f cellCenter = 0.5f * (cellCornerMax + cellCornerMin);
					vec4f cellHalfSize = 0.5f * (cellCornerMax - cellCornerMin);

					// gather lights
					for (int l = 0; l < m_sceneLights.m_lightCount; l++)
					{
						vec4f lightPosition = m_globalMatrices.view * m_sceneLights.m_lights[l].m_position;
						vec4f closest = cellCenter + clamp(lightPosition - cellCenter, -cellHalfSize, cellHalfSize);
						if ((lightPosition - closest).getNorm2() <= m_sceneLights.m_lights[l].m_range * m_sceneLights.m_lights[l].m_range)
						{
							unsigned int colorIndex = l >> 5;
							unsigned int lightBit = l & 0x1F;
							clusterLightMask[colorIndex] |= (1 << lightBit);
						}
					}
				}

				int id = k * imageSize.x * imageSize.y + j * imageSize.x + i;
				cpuClusterBuffer[id] = clusterLightMask;

				#ifdef USE_IMGUI
					clustersMin[id] = cellCornerMin;
					clustersMax[id] = cellCornerMax;
				#endif
			}

	glBindTexture(GL_TEXTURE_3D, imageID);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32UI, imageSize.x, imageSize.y, imageSize.z, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, cpuClusterBuffer);
}


void Renderer::render(CameraComponent* renderCam)
{
	//	clear previous states
	trianglesDrawn = 0;
	instanceDrawn = 0;
	drawCalls = 0;
	lastShader = nullptr;
	lastVAO = 0;
	if (!context || !camera || !world || !renderCam) return;
	
	//	bind matrix
	m_globalMatrices.view = renderCam->getViewMatrix();
	m_globalMatrices.projection = mat4f::perspective(renderCam->getVerticalFieldOfView(), context->getViewportRatio(), 0.1f, 1500.f);
	m_globalMatrices.cameraPosition = renderCam->getPosition();
	updateGlobalUniformBuffers();
	
	//	opengl state
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	//	draw grid
	Shader* shader = defaultShader[GRID];
	if (drawGrid && shader && glIsVertexArray(gridVAO))
	{
		shader->enable();
		ModelMatrix modelMatrix = {mat4f::identity, mat4f::identity};
		loadModelMatrix(shader, &modelMatrix);
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform4fv(loc, 1, &m_gridColor[0]);

		glBindVertexArray(gridVAO);
		glDrawElements(GL_TRIANGLES, vboGridSize, GL_UNSIGNED_INT, NULL);

		if (loc >= 0) glUniform4fv(loc, 1, (float*)&vec4f(-1.f, 0.f, 0.f, 1.f)[0]);
	}

	//	get instance list
	float camFovVert = camera->getVerticalFieldOfView();
	vec4f camPos, camFwd, camUp, camRight;
	camera->getFrustrum(camPos, camFwd, camRight, camUp);
	FrustrumSceneQuerry sceneTest(camPos, camFwd, camUp, -camRight, camFovVert, context->getViewportRatio());
	VirtualEntityCollector collector;
	collector.m_flags = (uint64_t)Entity::Flags::Fl_Drawable | (uint64_t)Entity::Flags::Fl_Light;
	collector.m_exclusionFlags = (uint64_t)Entity::Flags::Fl_Hide;
	world->getSceneManager().getEntities(&sceneTest, &collector);

	m_sceneLights.m_tanFovY = tan(0.5f * camera->getVerticalFieldOfView());
	m_sceneLights.m_tanFovX = m_sceneLights.m_tanFovY * context->getViewportRatio();


	// first pass
	// gather lights, compute entities hash, ...
	uint64_t transparentMask = 1ULL << 63;
	uint64_t faceCullingMask = 1ULL << 62;
	bool doBatching = false;
	renderQueue.clear();
	m_sceneLights.m_lightCount = 0;
	for (Entity* object : collector.getResult())
	{
		if (object->getFlags() & (uint64_t)Entity::Flags::Fl_Drawable)
		{
			DrawableComponent* comp = object->getComponent<DrawableComponent>();
			bool ok = comp && comp->isValid();
	#ifdef USE_IMGUI
			ok &= comp->visible();
	#endif
			if (ok)
			{
				doBatching |= comp->getShader()->supportInstancing();
				uint64_t queue = comp->getShader()->getRenderQueue();
				queue = queue << 48;

				vec4f v = object->getWorldPosition() - camPos;
				uint32_t d = (uint32_t)(1000.f * v.getNorm());
				if (queue & transparentMask)
				{
					//compute 2's complement of d
					d = ~d; 
					d++;
				}

				uint64_t hash = queue | d;
				renderQueue.push_back({ hash, object, nullptr});
			}


		}
		if ((object->getFlags() & (uint64_t)Entity::Flags::Fl_Light))
		{
			LightComponent* comp = object->getComponent<LightComponent>();
			bool ok = comp && m_sceneLights.m_lightCount < MAX_LIGHT_COUNT;

			#ifdef USE_IMGUI
				if (ok && m_lightFrustrumCulling)
					ok = sceneTest.TestSphere(object->getWorldPosition(), comp->getRange());
			#else
				if (ok)
					ok = sceneTest.TestSphere(object->getWorldPosition(), comp->getRange());
			#endif

			if (ok)
			{
				int i = m_sceneLights.m_lightCount;
				m_sceneLights.m_lights[i].m_color = comp->m_color;
				m_sceneLights.m_lights[i].m_position = comp->getPosition();
				m_sceneLights.m_lights[i].m_direction = comp->isPointLight() ? vec4f(0.f) : comp->getDirection();
				m_sceneLights.m_lights[i].m_range = comp->m_range;
				m_sceneLights.m_lights[i].m_intensity = comp->m_intensity;
				m_sceneLights.m_lights[i].m_inCutOff = cos((float)DEG2RAD * comp->m_innerCutoffAngle);
				m_sceneLights.m_lights[i].m_outCutOff = cos((float)DEG2RAD * comp->m_outerCutoffAngle);
				comp->m_isUniformBufferDirty = false;
				m_sceneLights.m_lightCount++;
			}
		}
	}

	// bind lights
	glBindBuffer(GL_UNIFORM_BUFFER, m_lightsID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, m_sceneLights.m_lightCount * sizeof(Light) + 2*sizeof(vec4i), &m_sceneLights);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	if (lightClustering)
	{
		//CPULightClustering();
		

		lightClustering->enable();

		GLint lightClusterLocation = glGetUniformLocation(lightClustering->getProgram(), "lightClusters");
		if (lightClusterLocation >= 0)
		{
			glUniform1i(lightClusterLocation, 3);
			glActiveTexture(GL_TEXTURE3);
			glBindImageTexture(3, imageID, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32UI);
		}

		glDispatchCompute(4, 3, 4);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		lastShader = nullptr;
		glUseProgram(0);
	}

	//	sort
	uint64_t compareMask = ~(transparentMask | faceCullingMask); // don't use the states bits for comparing entities
	std::sort(renderQueue.begin(), renderQueue.end(), [compareMask](DrawElement& a, DrawElement& b)
		{
			return (compareMask & a.hash) < (compareMask & b.hash);
		});

	// batching
	if (m_enableInstancing && doBatching)
	{
		// clear batches containers
		for (auto it : batchClosedPool)
		{
			it->models.clear();
			it->mesh = nullptr;
			it->shader = nullptr;
			batchFreePool.push_back(it);
		}
		batchClosedPool.clear();
		for (auto it : batchOpened)
		{
			it.second->models.clear();
			it.second->mesh = nullptr;
			it.second->shader = nullptr;
			batchFreePool.push_back(it.second);
		}
		batchOpened.clear();

		// second pass of renderqueue
		int variantCode = Shader::computeVariantCode(true, false, renderOption == RenderOption::WIREFRAME);
		for (auto& it : renderQueue)
		{
			DrawableComponent* comp = it.entity->getComponent<DrawableComponent>();
			if (!comp->getShader()->supportInstancing())
				continue;

			Shader* shader = comp->getShader()->getVariant(variantCode);
			Mesh* mesh = comp->getMesh();

			// search insertion batch
			Batch* batch;
			bool isNewBatch = false;
			std::pair<Shader*, Mesh*> key = {shader, mesh};
			auto it2 = batchOpened.find(key);
			if (it2 == batchOpened.end())
			{
				if (batchFreePool.empty())
					batch = new Batch();
				else
				{
					batch = batchFreePool.back();
					batchFreePool.pop_back();
				}

				isNewBatch = true;
				batch->shader = shader;
				batch->mesh = mesh;
				batchOpened[key] = batch;
			}
			else
				batch = it2->second;

			// insert object
			batch->models.push_back({ it.entity->getWorldTransformMatrix() , it.entity->getNormalMatrix()});
			it.batch = batch;

			if (!isNewBatch)
			{
				it.entity = nullptr;
				if (batch->models.size() >= MAX_INSTANCE)
				{
					batchClosedPool.push_back(batch);
					batchOpened.erase(it2);
				}
			}
		}
	}

	//	draw instance list
	bool blendingEnabled = false;
	bool faceCullingEnabled = true;
	for (const auto& it : renderQueue)
	{
		// skip batched entities
		if (!it.entity)
			continue;

		//	opengl states managing
		bool isTransparent = (it.hash & transparentMask);
		if (!blendingEnabled && isTransparent)
		{
			blendingEnabled = true;
			glEnable(GL_BLEND);
		}
		else if (blendingEnabled && !isTransparent)
		{
			blendingEnabled = false;
			glDisable(GL_BLEND);
		}

		bool needCulling = (it.hash & faceCullingMask);
		if (!faceCullingEnabled && needCulling)
		{
			faceCullingEnabled = true;
			glEnable(GL_CULL_FACE);
		}
		else if (faceCullingEnabled && !needCulling)
		{
			faceCullingEnabled = false;
			glDisable(GL_CULL_FACE);
		}

		if (it.batch)
			drawInstancedObject(it.batch->shader, it.batch->mesh, it.batch->models);
		else
			drawObject(it.entity);
	}
}
void Renderer::renderHUD()
{
	if (!context) return;

	// bind matrix
	m_globalMatrices.view = mat4f::eulerAngleZX((float)PI, (float)PI * 0.5f);
	m_globalMatrices.view[3] = vec4f(0.f, 0.f, -DISTANCE_HUD_CAMERA, 1.f);
	m_globalMatrices.projection = mat4f::perspective((float)DEG2RAD * ANGLE_VERTICAL_HUD_PROJECTION, context->getViewportRatio(), 0.1f, 1500.f);
	updateGlobalUniformBuffers();

	//	change opengl states
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	uint8_t stencilMask = 0x00;

	//	draw all widget in hudList
	std::map<std::string, std::vector<Layer*>>& wList = WidgetManager::getInstance()->hudList;
	for (std::map<std::string, std::vector<Layer*> >::iterator it = wList.begin(); it != wList.end(); ++it)
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			if (it->second[i]->isVisible())		// layer visible
			{
				mat4f model = it->second[i]->getModelMatrix();
				std::vector<WidgetVirtual*>& list = it->second[i]->getChildrenList();
				for (std::vector<WidgetVirtual*>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
				{
					if ((*it2)->isVisible())	// widget visible
					{
						//	Get shader and prepare matrix
						Shader* shader = nullptr;
						if (!defaultShader[HUD]) 
							shader = (*it2)->getShader();

						mat4f m = mat4f::translate(model, (*it2)->getPosition());
						ModelMatrix modelMatrix = { m, model };
						loadModelMatrix(shader, &modelMatrix);
						if (!shader)
							continue;

						//	Draw
						(*it2)->draw(shader, stencilMask, model);
						drawCalls;
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
void Renderer::loadModelMatrix(Shader* shader, const ModelMatrix* model, const int& modelSize)
{
	if (shader)
	{
		if (shader != lastShader)
		{
			shader->enable();

			GLint lightClusterLocation = glGetUniformLocation(shader->getProgram(), "lightClusters");
			if (lightClusterLocation >= 0)
			{
				glUniform1i(lightClusterLocation, 3);
				glActiveTexture(GL_TEXTURE3);
				glBindImageTexture(3, imageID, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32UI);
			}
		}
		lastShader = shader;

		int loc = shader->getUniformLocation("matrixArray");
		if (loc >= 0)
			glUniformMatrix4fv(loc, 2 * modelSize, GL_FALSE, (const float*)model);
	}
	else  lastShader = nullptr;
}
void Renderer::loadVAO(const GLuint& vao)
{
	if (vao != lastVAO)
	{
		glBindVertexArray(vao);
		lastVAO = vao;
	}
}
//

//  Set/get functions
void Renderer::setCamera(CameraComponent* cam) { camera = cam; }
void Renderer::setWorld(World* currentWorld) { world = currentWorld; }
void Renderer::setContext(RenderContext* ctx) { context = ctx; }
void Renderer::setShader(ShaderIdentifier id, Shader* s)
{
	std::map<ShaderIdentifier, Shader*>::iterator it = defaultShader.find(id);
	if(it != defaultShader.end()) ResourceManager::getInstance()->release(defaultShader[id]);

	if (s) defaultShader[id] = ResourceManager::getInstance()->getResource<Shader>(s);
	else defaultShader[id] = nullptr;
}
void Renderer::setGridVisible(bool enable) { drawGrid = enable; }
void Renderer::setRenderOption(const RenderOption& option) { renderOption = option; }


CameraComponent* Renderer::getCamera() { return camera; }
World* Renderer::getWorld() { return world; }
RenderContext* Renderer::getContext() { return context; }
Shader* Renderer::getShader(ShaderIdentifier id)
{
	std::map<ShaderIdentifier, Shader*>::iterator it = defaultShader.find(id);
	if (it != defaultShader.end()) return defaultShader[id];
	else return nullptr;
}
bool Renderer::isGridVisible() { return drawGrid; }
unsigned int Renderer::getNbDrawnInstances() const { return instanceDrawn; }
unsigned int Renderer::getNbDrawCalls() const { return drawCalls; }
unsigned int Renderer::getNbDrawnTriangles() const { return trianglesDrawn; }
Renderer::RenderOption Renderer::getRenderOption() const { return renderOption; }


void Renderer::setEnvBackgroundColor(vec4f color)
{ 
	m_environementLighting.m_backgroundColor = color;
	glClearColor(m_environementLighting.m_backgroundColor.x, m_environementLighting.m_backgroundColor.y, m_environementLighting.m_backgroundColor.z, m_environementLighting.m_backgroundColor.w);
}
void Renderer::setEnvAmbientColor(vec4f color) { m_environementLighting.m_ambientColor = color; }
void Renderer::setEnvDirectionalLightDirection(vec4f direction) { m_environementLighting.m_directionalLightDirection = direction; }
void Renderer::setEnvDirectionalLightColor(vec4f color) { m_environementLighting.m_directionalLightColor = color; }
vec4f Renderer::getEnvBackgroundColor() const { return m_environementLighting.m_backgroundColor; }
vec4f Renderer::getEnvAmbientColor() const { return m_environementLighting.m_ambientColor; }
vec4f Renderer::getEnvDirectionalLightDirection() const { return m_environementLighting.m_directionalLightDirection; }
vec4f Renderer::getEnvDirectionalLightColor() const { return m_environementLighting.m_directionalLightColor; }

//

//	Debug
void Renderer::drawImGui(World& world)
{
#ifdef USE_IMGUI
	ImGui::Begin("Rendering setings");
	ImGui::PushID(this);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Environement lighting");
	ImGui::DragFloat3("Directional light direction", &m_environementLighting.m_directionalLightDirection[0], 0.01f);
	ImGui::ColorEdit3("Directional light color", &m_environementLighting.m_directionalLightColor[0]);
	ImGui::ColorEdit3("Ambient color", &m_environementLighting.m_ambientColor[0]);
	if (ImGui::ColorEdit3("Background color", &m_environementLighting.m_backgroundColor[0]))
		glClearColor(m_environementLighting.m_backgroundColor.x, m_environementLighting.m_backgroundColor.y, m_environementLighting.m_backgroundColor.z, m_environementLighting.m_backgroundColor.w);
	ImGui::Checkbox("Draw light direction", &m_drawLightDirection);

	const char* renderOptions[] = { "Default", "Bounding box", "Wireframe", "Normals" };
	static int renderOptionsCurrentIdx = (int)renderOption;
	const char* renderOptionPreviewValue = renderOptions[renderOptionsCurrentIdx];


	ImGui::Spacing();
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Rendering parameters");
	ImGui::Checkbox("Instancing enabled", &m_enableInstancing);
	if (ImGui::BeginCombo("Render option", renderOptionPreviewValue))
	{
		for (int n = 0; n < IM_ARRAYSIZE(renderOptions); n++)
		{
			const bool is_selected = (renderOptionsCurrentIdx == n);
			if (ImGui::Selectable(renderOptions[n], is_selected))
			{
				renderOptionsCurrentIdx = n;
				renderOption = (RenderOption)n;
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::Checkbox("Light frustrum culling", &m_lightFrustrumCulling);
	ImGui::Checkbox("Do light clustering", &m_lightClustering);
	ImGui::Checkbox("Draw light clusters", &m_drawClusters);


	ImGui::Spacing();
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "DrawInfos");
	ImGui::Text("Drawcalls : %d", drawCalls);
	ImGui::Text("Entities : %d", instanceDrawn);
	ImGui::Text("Triangles : %d", trianglesDrawn);
	ImGui::Text("Lights : %d", m_sceneLights.m_lightCount);


	ImGui::PopID();
	ImGui::End();

	if (m_drawLightDirection)
	{
		vec4f d = m_environementLighting.m_directionalLightDirection;
		d.normalize();

		Debug::color = m_environementLighting.m_directionalLightColor;
		for (int i = -50; i <= 50; i++)
			for (int j = -50; j <= 50; j++)
			{
				vec4f p = vec4f(5 * i, 0, 5 * j, 1.f);
				Debug::drawLine(p, p - 100.f * d);
			}
	}

	CameraComponent* mainCamera = world.getMainCamera();
	if (m_drawClusters && mainCamera && clustersMin && clustersMax)
	{
		//Debug::color = Debug::white;
		const float shrink = 1.f;
		for (int i = 0; i < imageSize.x; i++)
			for (int j = 0; j < imageSize.y; j++)
				for (int k = 0; k < imageSize.z; k++)
				{
					//if (k != 0) continue;

					int id = i * imageSize.y * imageSize.z + j * imageSize.z + k;
					vec4f cellCenter = 0.5f * (clustersMax[id] + clustersMin[id]);
					vec4f cellHalfSize = 0.5f *(clustersMax[id] - clustersMin[id]);

					if (m_sceneLights.m_lightCount)
					{
						Debug::color = Debug::black;
						vec4ui lightMask = cpuClusterBuffer[id];
						for (int l = 0; l < 4; l++)
							for (int m = 0; m < 32; m++)
							{
								if ((lightMask[l] & (1 << m)) != 0)
									Debug::color[l] += 1.0;
							}
					}

					Debug::drawLineCube(mainCamera->getModelMatrix(), cellCenter - shrink * cellHalfSize, cellCenter + shrink * cellHalfSize);
				}
	}
#endif // USE_IMGUI
}
//