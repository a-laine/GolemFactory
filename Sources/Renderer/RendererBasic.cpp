#include "Renderer.h"
#include "CameraComponent.h"
#include "imgui_internal.h"

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
bool RenderingWindowEnable = false;
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
	m_OcclusionElapsedTime = m_OcclusionAvgTime = 0;

	defaultShader[GRID] = nullptr;
	defaultShader[INSTANCE_ANIMATABLE_BB] = nullptr;
	defaultShader[INSTANCE_DRAWABLE_BB] = nullptr;
	defaultShader[HUD] = nullptr;

	lightClustering = nullptr;

	drawGrid = true;
	batchFreePool.reserve(512);

	m_environementLighting.m_shadowFarPlanes = vec4f(20.f, 50.f, 150.f, 400.f);
	shadowAreaMargin = 10.f; 
	shadowAreaMarginLightDirection = 50.f;

	omniLightCollector.m_flags = (uint64_t)Entity::Flags::Fl_Drawable;
	omniLightCollector.m_exclusionFlags = (uint64_t)Entity::Flags::Fl_Hide;

	collector.m_exclusionFlags = (uint64_t)Entity::Flags::Fl_Hide;

	glGenQueries(1, &m_timerQueryID);

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

	if (m_occlusionDepth)
	{
		delete[] m_occlusionDepth;
		delete[] m_occlusionCenterX;
		delete[] m_occlusionCenterY;
	}

}
//

//  Public functions
void Renderer::initializeGrid(const unsigned int& gridSize,const float& elementSize, const vec4f& color)
{
	if (glIsVertexArray(gridVAO)) return;

	defaultShader[GRID] = defaultShader[DEFAULT]->getVariant(Shader::computeVariantCode(false, 0, true));
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
	vec3i imageSize = vec3i(width, height, depth);
	lightClustering = ResourceManager::getInstance()->getResource<Shader>("lightClustering");

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

	using TC = Texture::TextureConfiguration;
	uint8_t config = (uint8_t)TC::TEXTURE_3D | (uint8_t)TC::MIN_NEAREST | (uint8_t)TC::MAG_NEAREST | (uint8_t)TC::WRAP_CLAMP;
	lightClusterTexture.initialize("lightClusterTexture", imageSize, buffer, config, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
	ResourceManager::getInstance()->addResource(&lightClusterTexture);
	lightClusterTexture.isEnginePrivate = true;
	delete[] buffer;

	m_sceneLights.m_near = 2.f;
	m_sceneLights.m_far = 1500.f;
	float logRatio = imageSize.z / log(m_sceneLights.m_far / m_sceneLights.m_near);
	m_sceneLights.m_clusterDepthScale = logRatio;
	m_sceneLights.m_clusterDepthBias = logRatio * log(m_sceneLights.m_near);
	m_sceneLights.m_shadingConfiguration = (1 << eUseLightClustering) |(1 << eUseShadow);
}
void Renderer::initializeOcclusionBuffers(int width, int height)
{
	m_occlusionBufferSize = vec2i(width, height);
	int size = m_occlusionBufferSize.x * m_occlusionBufferSize.y;
	m_occlusionDepth = new float[size];
	m_occlusionCenterX = new float[size];
	m_occlusionCenterY = new float[size];
	//m_occlusionDepthColor = new uint32_t[size];

	constexpr float depthMax = std::numeric_limits<float>::min();
	for (int i = 0; i < m_occlusionBufferSize.x; i++)
		for (int j = 0; j < m_occlusionBufferSize.y; j++)
		{
			int id = j * m_occlusionBufferSize.x + i;
			m_occlusionDepth[id] = depthMax;
			m_occlusionCenterX[id] = (float)(i + 0.5f) / m_occlusionBufferSize.x;
			m_occlusionCenterY[id] = (float)(j + 0.5f) / m_occlusionBufferSize.y;
		}

#ifdef USE_IMGUI
	using TC = Texture::TextureConfiguration;
	uint8_t config = (uint8_t)TC::TEXTURE_2D | (uint8_t)TC::MIN_NEAREST | (uint8_t)TC::MAG_NEAREST | (uint8_t)TC::WRAP_CLAMP;
	occlusionTexture.initialize("occlusionTexture", vec3i(m_occlusionBufferSize.x, m_occlusionBufferSize.y, 0),
		nullptr, config, GL_R32F, GL_RED, GL_FLOAT);// GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	ResourceManager::getInstance()->addResource(&occlusionTexture);
	occlusionTexture.isEnginePrivate = true;
	occlusionResultDraw = ResourceManager::getInstance()->getResource<Shader>("occlusionResult");
#endif
}
void Renderer::initGlobalUniformBuffers()
{
	// full screen quad 
	glCreateVertexArrays(1, &fullscreenVAO);
	glBindVertexArray(fullscreenVAO);
	glBindVertexArray(0);

	fullscreenTriangle = ResourceManager::getInstance()->getResource<Shader>("fullscreenTriangle");

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
	m_environementLighting.m_directionalLightDirection = vec4f(-10, -20, 0, 0);
	m_environementLighting.m_directionalLightColor = vec4f(1.f);// vec4f(0.15, 0.15, 0.15, 1.0);
	m_environementLighting.m_ambientColor = vec4f(0);// 0.05, 0.05, 0.05, 1.0);

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

	// lights data
	m_debugShaderUniform.vertexNormalColor = vec4f(0.0, 0.0, 1.0, 0.1);
	m_debugShaderUniform.faceNormalColor = vec4f(0.0, 1.0, 1.0, 0.1);
	m_debugShaderUniform.wireframeEdgeFactor = 0.4f;
	m_debugShaderUniform.occlusionResultCuttoff = 50;
	m_debugShaderUniform.occlusionResultDrawAlpha = 0.8f;

	glGenBuffers(1, &m_DebugShaderUniformID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_DebugShaderUniformID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_debugShaderUniform), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 3, m_DebugShaderUniformID, 0, sizeof(m_debugShaderUniform));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// omni shadow matrices
	glGenBuffers(1, &m_omniShadowsID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_omniShadowsID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_OmniShadows), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 4, m_omniShadowsID, 0, sizeof(m_OmniShadows));
}
void Renderer::initializeShadows(int cascadesWidth, int cascadesHeight, int omniWidth, int omniHeight)
{
	// shadow cascade
	std::string header = "Shadow init : ";
	const auto CheckError = [header](const char* label)
	{
		GLenum error = glGetError();
		if (!label)
			return false;
		switch (error)
		{
			case GL_INVALID_ENUM: std::cout << header << label << " : GL_INVALID_ENUM" << std::endl; break;
			case GL_INVALID_VALUE: std::cout << header << label << " : GL_INVALID_VALUE" << std::endl; break;
			case GL_INVALID_OPERATION: std::cout << header << label << " : GL_INVALID_OPERATION" << std::endl; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: std::cout << header << label << " : GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl; break;
			case GL_OUT_OF_MEMORY: std::cout << header << label << " : GL_OUT_OF_MEMORY" << std::endl; break;
			case GL_STACK_UNDERFLOW: std::cout << header << label << " : GL_STACK_UNDERFLOW" << std::endl; break;
			case GL_STACK_OVERFLOW: std::cout << header << label << " : GL_STACK_OVERFLOW" << std::endl; break;
			default: break;
		}
		return error != GL_NO_ERROR;
	};

	using TC = Texture::TextureConfiguration;
	uint8_t config = (uint8_t)TC::TEXTURE_ARRAY | (uint8_t)TC::WRAP_CLAMP;
	shadowCascadeTexture.initialize("shadowCascadeTexture", vec3i(cascadesWidth, cascadesHeight, 4),
		nullptr, config, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
	shadowCascadeTexture.isEnginePrivate = true;

	glBindTexture(GL_TEXTURE_2D_ARRAY, shadowCascadeTexture.getTextureId());
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); CheckError("cascade : GL_TEXTURE_COMPARE_MODE");
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL); CheckError("cascade : GL_TEXTURE_COMPARE_FUNC");
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	ResourceManager::getInstance()->addResource(&shadowCascadeTexture);

	glGenFramebuffers(1, &m_ShadowCascadeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowCascadeFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowCascadeTexture.getTextureId(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << header << "Cascade FBO : Framebuffer is not complete !" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// omni cubemap array
	config = (uint8_t)TC::CUBEMAP_ARRAY | (uint8_t)TC::WRAP_CLAMP;
	shadowOmniTextures.initialize("shadowOmniTextures", vec3i(omniWidth, omniHeight, MAX_OMNILIGHT_SHADOW_COUNT),
		nullptr, config, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
	shadowOmniTextures.isEnginePrivate = true;

	ResourceManager::getInstance()->addResource(&shadowOmniTextures);

	glGenFramebuffers(1, &m_ShadowOmniFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowOmniFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowOmniTextures.getTextureId(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << header << "Omni FBO Framebuffer is not complete !" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::initializeOverviewRenderer(int width, int height)
{
	using TC = Texture::TextureConfiguration;
	uint8_t config = (uint8_t)TC::TEXTURE_2D | (uint8_t)TC::WRAP_CLAMP;
	overviewDepth.initialize("overviewDepth", vec3i(width, height, 0),
		nullptr, config, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
	overviewTexture.initialize("overviewTexture", vec3i(width, height, 0),
		nullptr, config, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	ResourceManager::getInstance()->addResource(&overviewDepth);
	ResourceManager::getInstance()->addResource(&overviewTexture);
	overviewDepth.isEnginePrivate = true;
	overviewTexture.isEnginePrivate = true;

	glGenFramebuffers(1, &overviewFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, overviewFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, overviewTexture.getTextureId(), 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, overviewDepth.getTextureId(), 0);
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Debug::reinterpreteTexture" << std::endl;
		return;
	}
}

void Renderer::updateGlobalUniformBuffers()
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_globalMatricesID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_globalMatrices), &m_globalMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, m_environementLightingID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_environementLighting), &m_environementLighting);

	glBindBuffer(GL_UNIFORM_BUFFER, m_DebugShaderUniformID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_debugShaderUniform), &m_debugShaderUniform);
}
void Renderer::updateShadowCascadeMatrices(CameraComponent* renderCam, float viewportRatio)
{
	vec4f nears = vec4f(0.f, m_environementLighting.m_shadowFarPlanes.x, m_environementLighting.m_shadowFarPlanes.y, m_environementLighting.m_shadowFarPlanes.z);
	m_environementLighting.m_shadowBlendMargin = shadowAreaMargin;

	// compute frustrum params and base direction matrix
	vec4f u1, u2, u3, u4, pos;
	mat4f lightOrientation;
	{
		pos = camera->getPosition();
		vec4f dir = camera->getForward();
		vec4f left = -camera->getRight();
		vec4f up = camera->getUp();
		float a1 = 0.5f * camera->getVerticalFieldOfView();
		float ca1 = cos(a1);
		float sa1 = sin(a1);
		float sa2 = viewportRatio * sa1;
		vec4f tdir = ca1 * dir;
		u1 = tdir + sa2 * left + sa1 * up;
		u2 = tdir - sa2 * left + sa1 * up;
		u3 = tdir + sa2 * left - sa1 * up;
		u4 = tdir - sa2 * left - sa1 * up;

		vec4f lightForward = -m_environementLighting.m_directionalLightDirection.getNormal();
		vec4f lightUp = std::abs(lightForward.x) > std::abs(lightForward.z) ? vec4f(lightForward.y, -lightForward.x, 0, 0) : vec4f(0, lightForward.z, -lightForward.y, 0);
		lightUp.normalize();
		vec4f lightRight = vec4f::cross(lightUp, lightForward);
		lightOrientation[0] = lightRight;
		lightOrientation[1] = lightUp;
		lightOrientation[2] = lightForward;
	}

	// compute matrices
	vec4f corners[8];
	for (int i = 0; i < 4; i++)
	{
		// frustrum area corner
		corners[0] = pos + nears[i] * u1;
		corners[1] = pos + nears[i] * u2;
		corners[2] = pos + nears[i] * u3;
		corners[3] = pos + nears[i] * u4;
		corners[4] = pos + m_environementLighting.m_shadowFarPlanes[i] * u1;
		corners[5] = pos + m_environementLighting.m_shadowFarPlanes[i] * u2;
		corners[6] = pos + m_environementLighting.m_shadowFarPlanes[i] * u3;
		corners[7] = pos + m_environementLighting.m_shadowFarPlanes[i] * u4;

		// compute area center and a radius to fit the frustrum area
		vec4f center = pos + (0.5f * (m_environementLighting.m_shadowFarPlanes[i] + nears[i])) * camera->getForward();
		float R = 0;
		for (int j = 0; j < 8; j++)
			R = std::max(R, (center - corners[j]).getNorm2());
		vec4f max = vec4f(std::sqrt(R));
		vec4f min = -max;

		// snap the center to a texel size
		mat4f invView = mat4f::transpose(lightOrientation);
		vec4f lightSpaceCenter = -(invView * center);
		float texelSize = (max.x - min.x) / shadowCascadeTexture.size.x;
		float invTexelSize = 1.f / texelSize;
		lightSpaceCenter *= invTexelSize;
		vec4f snapedCenter = texelSize * vec4f(vec4i(lightSpaceCenter));
		snapedCenter.w = 1.f;
		invView[3] = snapedCenter;

		// aply offset on Z axis
		mat4f view = lightOrientation;
		view[3] = center;
		float zoffset = std::max(0.f, shadowAreaMarginLightDirection - shadowAreaMargin);
		shadowAreaBoxes[i].base = view;
		shadowAreaBoxes[i].min = min;
		shadowAreaBoxes[i].max = max;
		shadowAreaBoxes[i].max.z = max.z + zoffset;
		shadowAreaBoxes[i].min.z = min.z - shadowAreaMargin;
		shadowAreaBoxes[i].min.w = 1;
		shadowAreaBoxes[i].max.w = 1;

		// end
		mat4f lightProjection = mat4f::ortho(min.x, max.x, min.y, max.y, min.z - zoffset, max.z + shadowAreaMargin);
		m_environementLighting.m_shadowCascadeProjections[i] = lightProjection * invView;
	}
}
void Renderer::computeOmniShadowProjection(LightComponent* light, int omniIndex)
{
	float aspect = (float)shadowOmniTextures.size.x / (float)shadowOmniTextures.size.y;
	float n = 1.f;
	float f = light->getRange();
	mat4f shadowProj = mat4f::perspective((float)DEG2RAD * 90.f, aspect, n, f);
	vec4f center = light->getPosition();
	vec3f lightPos = vec3f(center.x, center.y, center.z);

	vec4f x(1.f, 0.f, 0.f, 0.f);
	vec4f y(0.f, 1.f, 0.f, 0.f);
	vec4f z(0.f, 0.f, 1.f, 0.f);

	m_OmniShadows.m_omniShadowLightNear[omniIndex] = n;
	m_OmniShadows.m_shadowOmniProjections[6 * omniIndex]     = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	m_OmniShadows.m_shadowOmniProjections[6 * omniIndex + 1] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	m_OmniShadows.m_shadowOmniProjections[6 * omniIndex + 2] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	m_OmniShadows.m_shadowOmniProjections[6 * omniIndex + 3] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
	m_OmniShadows.m_shadowOmniProjections[6 * omniIndex + 4] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
	m_OmniShadows.m_shadowOmniProjections[6 * omniIndex + 5] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
}

void Renderer::render(CameraComponent* renderCam)
{
	SCOPED_CPU_MARKER("Frame Renderering");

	//	clear previous states
	trianglesDrawn = 0;
	instanceDrawn = 0;
	drawCalls = 0;
	occlusionCulledInstances = 0;
	lastShader = nullptr;
	lastVAO = 0;
	glBeginQuery(GL_TIME_ELAPSED, m_timerQueryID);

	if (!context || !camera || !world || !renderCam)
		return;

	//	bind matrix
	m_globalMatrices.view = renderCam->getViewMatrix();
	m_globalMatrices.projection = mat4f::perspective(renderCam->getVerticalFieldOfView(), context->getViewportRatio(), 0.1f, 1500.f);
	m_globalMatrices.cameraPosition = renderCam->getPosition();
	updateShadowCascadeMatrices(camera, context->getViewportRatio());
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
		loadGlobalUniforms(shader);
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform4fv(loc, 1, &m_gridColor[0]);

		glBindVertexArray(gridVAO);
		glDrawElements(GL_TRIANGLES, vboGridSize, GL_UNSIGNED_INT, NULL);

		if (loc >= 0) glUniform4fv(loc, 1, (float*)&vec4f(-1.f, 0.f, 0.f, 1.f)[0]);
	}

	m_sceneLights.m_tanFovY = tan(0.5f * camera->getVerticalFieldOfView());
	m_sceneLights.m_tanFovX = m_sceneLights.m_tanFovY * context->getViewportRatio();
	CollectEntitiesBindLights();
	CollectTerrainQueueData();

	if (lightClustering)
		LightClustering();
	if (m_enableOcclusionCulling && !m_occluders.empty() && m_occlusionDepth)
		OcclusionCulling();
	else
	{
		m_OcclusionAvgTime = 0.f;
		m_OcclusionElapsedTime = 0.f;
	}

	if (m_sceneLights.m_shadingConfiguration & (1 << eUseShadow))
		ShadowCasting();

	//	sort
	uint64_t transparentMask = 1ULL << 63;
	uint64_t faceCullingMask = 1ULL << 62;
	uint64_t compareMask = ~(transparentMask | faceCullingMask); // don't use the states bits for comparing entities
	std::sort(renderQueue.begin(), renderQueue.end(), [compareMask](DrawElement& a, DrawElement& b)
		{
			return (compareMask & a.hash) < (compareMask & b.hash);
		});

	// batching
	if (m_enableInstancing && m_hasInstancingShaders)
		DynamicBatching();

	// state tracking
	bool blendingEnabled = false;
	bool faceCullingEnabled = true;
	const auto SetBlending = [&blendingEnabled](bool state)
	{
		if (!blendingEnabled && state)
			glEnable(GL_BLEND);
		else if (blendingEnabled && !state)
			glDisable(GL_BLEND);
		blendingEnabled = state;
	};
	const auto SetCulling = [&faceCullingEnabled](bool state)
	{
		if (!faceCullingEnabled && state)
			glEnable(GL_CULL_FACE);
		else if (faceCullingEnabled && !state)
			glDisable(GL_CULL_FACE);
		faceCullingEnabled = state;
	};

	//	draw instance list
	glViewport(0, 0, context->getViewportSize().x, context->getViewportSize().y);
	{
		SCOPED_CPU_MARKER("Drawing");

		for (const auto& it : renderQueue)
		{
			// skip batched entities
			if (!it.entity)
				continue;

			//	opengl states managing
			SetBlending(it.hash & transparentMask);
			SetCulling(it.hash & faceCullingMask);

			if (it.batch)
				drawInstancedObject(it.batch->shader, it.batch->mesh, it.batch->models);
			else
				drawObject(it.entity);
		}
	}


	glEndQuery(GL_TIME_ELAPSED);
}
void Renderer::renderHUD()
{
	SCOPED_CPU_MARKER("HUD rendering");

	if (!context) return;

	// bind matrix
	m_globalMatrices.view = mat4f::eulerAngleZX((float)PI, (float)PI * 0.5f);
	m_globalMatrices.view[3] = vec4f(0.f, 0.f, -DISTANCE_HUD_CAMERA, 1.f);
	m_globalMatrices.projection = mat4f::perspective((float)DEG2RAD * ANGLE_VERTICAL_HUD_PROJECTION, context->getViewportRatio(), 0.1f, 1500.f);
	updateGlobalUniformBuffers();

	//	change opengl states
	glViewport(0, 0, context->getViewportSize().x, context->getViewportSize().y);
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
}
void Renderer::swap()
{
	int stopTimerAvailable = 0;
	int timeout = 3;
	while (!stopTimerAvailable)
	{
		glGetQueryObjectiv(m_timerQueryID, GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
	}

	if (timeout)
	{
		GLuint64 elapsedGPUtimer;
		glGetQueryObjectui64v(m_timerQueryID, GL_QUERY_RESULT, &elapsedGPUtimer);
		m_GPUelapsedTime = (float)(elapsedGPUtimer) * 1E-06f;
		m_GPUavgTime = 0.95f * m_GPUavgTime + 0.05f * m_GPUelapsedTime;
	}

	Debug::getInstance()->clearVBOs();
}
//


//	Protected functions
void Renderer::loadModelMatrix(Shader* shader, const ModelMatrix* model, const int& modelSize)
{
	if (shader)
	{
		shaderJustActivated = shader != lastShader;
		if (shader != lastShader)
		{
			shader->enable();
			glBindVertexArray(0);
			lastVAO = 0;
		}
		lastShader = shader;

		int loc = shader->getUniformLocation("matrixArray");
		if (loc >= 0)
			glUniformMatrix4fv(loc, 2 * modelSize, GL_FALSE, (const float*)model);
	}
	else 
	{
		shaderJustActivated = false;
		lastShader = nullptr;
	}
}
void Renderer::loadGlobalUniforms(Shader* shader)
{
	if (shader && shaderJustActivated)
	{
		int loc = shader->getUniformLocation("_globalLightClusters");
		if (loc >= 0)
		{
			uint8_t unit = shader->getGlobalTextureUnit("_globalLightClusters");
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindImageTexture(unit, lightClusterTexture.getTextureId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32UI);
		}

		loc = shader->getUniformLocation("_globalShadowCascades");
		if (loc >= 0)
		{
			uint8_t unit = shader->getGlobalTextureUnit("_globalShadowCascades");
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(GL_TEXTURE_2D_ARRAY, shadowCascadeTexture.getTextureId());
		}

		loc = shader->getUniformLocation("_globalOmniShadow");
		if (loc >= 0)
		{
			uint8_t unit = shader->getGlobalTextureUnit("_globalOmniShadow");
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, shadowOmniTextures.getTextureId());
		}

		if (baseLayerUniform >= 0)
		{
			loc = shader->getUniformLocation("baseLayer");
			if (loc >= 0)
				glUniform1i(loc, baseLayerUniform);
		}
	}
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

	if (id == DEFAULT)
	{
		if (defaultShader.find(GRID) == defaultShader.end())
			defaultShader[GRID] = defaultShader[DEFAULT]->getVariant(Shader::computeVariantCode(false, 0, true));
		if (defaultShader.find(INSTANCE_DRAWABLE_BB) == defaultShader.end())
			defaultShader[INSTANCE_DRAWABLE_BB] = defaultShader[DEFAULT]->getVariant(Shader::computeVariantCode(false, 0, true));
	}
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
vec2i Renderer::getOverviewTextureSize() const { return vec2i(overviewTexture.size.x, overviewTexture.size.y); }

double Renderer::getElapsedTime() const { return m_GPUelapsedTime; }
double Renderer::getAvgElapsedTime() const { return m_GPUavgTime; }

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
	SCOPED_CPU_MARKER("Renderer");

	ImVec4 titleColor = ImVec4(1, 1, 0, 1);

	ImGui::Begin("Rendering setings");
	ImGui::PushID(this);
	ImGui::TextColored(titleColor, "Environement lighting");
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
	ImGui::TextColored(titleColor, "Rendering parameters");
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

	const auto& CheckboxFlag = [](const char* label, unsigned int& bitfield, int flag, unsigned int mask = 0xFFFFFFFF)
	{
		bool b = bitfield & (1 << flag);
		if (ImGui::Checkbox(label, &b))
		{
			bitfield &= mask;
			if (b)
				bitfield |= 1 << flag;
			else	
				bitfield &= ~(1 << flag);
		}
	};
	CheckboxFlag("Do light clustering", m_sceneLights.m_shadingConfiguration, eUseLightClustering);
	CheckboxFlag("Draw light count heatmap", m_sceneLights.m_shadingConfiguration, eLightCountHeatmap);

	ImGui::Checkbox("Occlusion culling", &m_enableOcclusionCulling);
	ImGui::Checkbox("Draw light clusters", &m_drawClusters);
	ImGui::Checkbox("Draw occlusion buffer", &m_drawOcclusionBuffer);
	ImGui::Checkbox("Draw grid", &drawGrid);

	if (!m_drawOcclusionBuffer)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}
	{
		ImGui::SliderFloat("Occlusion buffer alpha", &m_debugShaderUniform.occlusionResultDrawAlpha, 0, 1);
		ImGui::SliderFloat("Occlusion  gradiant cutoff", &m_debugShaderUniform.occlusionResultCuttoff, 1.f, m_sceneLights.m_far, "%.2f", ImGuiSliderFlags_Logarithmic);
	}
	if (!m_drawOcclusionBuffer)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}


	ImGui::TextColored(titleColor, "Shadows");
	CheckboxFlag("Shadow cascades", m_sceneLights.m_shadingConfiguration, eUseShadow);
	if (!(m_sceneLights.m_shadingConfiguration & (1 << eUseShadow)))
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}
	{
		ImGui::Checkbox("Stable fit", &m_shadowStableFit);
		ImGui::DragFloat4("Shadow far planes", &m_environementLighting.m_shadowFarPlanes[0], 0.3f, 0.5f, 2000.f);
		CheckboxFlag("Draw shadow cascades", m_sceneLights.m_shadingConfiguration, eDrawShadowCascades);
		ImGui::DragFloat("Shadow projection margin", &shadowAreaMargin, 0.1f, 0.f, 20.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("Shadow light direction margin", &shadowAreaMarginLightDirection, 0.1f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	}
	if (!(m_sceneLights.m_shadingConfiguration & (1 << eUseShadow)))
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::Spacing();
	ImGui::TextColored(titleColor, "Debug shader uniforms");
	ImGui::ColorEdit3("Vertex normal color", &m_debugShaderUniform.vertexNormalColor[0]);
	ImGui::ColorEdit3("Face normal color", &m_debugShaderUniform.faceNormalColor[0]);
	ImGui::SliderFloat("Vertex normal length", &m_debugShaderUniform.vertexNormalColor[3], 0.f, 1.f, "%.3f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Face normal length", &m_debugShaderUniform.faceNormalColor[3], 0.f, 1.f, "%.3f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Wireframe edge factor", &m_debugShaderUniform.wireframeEdgeFactor, 0.f, 1.f, "%.3f", ImGuiSliderFlags_Logarithmic);

	ImGui::Spacing();
	float columnWidth = 250.f;
	ImGui::TextColored(titleColor, "DrawInfos");
	ImGui::Text("Entities : %d", instanceDrawn);				ImGui::SameLine(columnWidth); ImGui::Text("Occluder triangles : %d", occluderTriangles);
	ImGui::Text("Drawcalls : %d", drawCalls);					ImGui::SameLine(columnWidth); ImGui::Text("Occlusion rasterized triangles : %d", occluderRasterizedTriangles);
	ImGui::Text("Triangles : %d", trianglesDrawn);				ImGui::SameLine(columnWidth); ImGui::Text("Occlusion pixel test : %d", occluderPixelsTest);
	ImGui::Text("Lights : %d", m_sceneLights.m_lightCount);		ImGui::SameLine(columnWidth); ImGui::Text("Occlusion skipped : %d", occlusionCulledInstances);
	ImGui::Text("Occlusion time : %d ms (%d)", (int)m_OcclusionAvgTime, (int)m_OcclusionElapsedTime);
	
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
	if (m_sceneLights.m_shadingConfiguration & (1 << eDrawShadowCascades))
	{
		vec4f nears = vec4f(-1.f, m_environementLighting.m_shadowFarPlanes.x, m_environementLighting.m_shadowFarPlanes.y, m_environementLighting.m_shadowFarPlanes.z);
		for (int i = 0; i < 4; i++)
		{
			camera->drawDebug(Debug::viewportRatio, nears[i], m_environementLighting.m_shadowFarPlanes[i], Debug::white);
			Debug::color = Debug::yellow;
			Debug::drawLineCube(shadowAreaBoxes[i].base, shadowAreaBoxes[i].min, shadowAreaBoxes[i].max);
		}
	}

	CameraComponent* mainCamera = world.getMainCamera();
	if (m_drawClusters)
	{
		static std::vector<Debug::Vertex> clusterLines;
		if (clusterLines.empty())
		{
			vec4f color = Debug::white;
			vec3i imageSize = lightClusterTexture.size;
			int clusterCount = imageSize.x * imageSize.y * imageSize.z;
			int rightClusterCount = imageSize.y * imageSize.z;
			int topClusterCount = imageSize.x * imageSize.z;

			int a = imageSize.x;
			int b = imageSize.y;
			int c = imageSize.z;
			int size = 2 * (3 * a * b * c + 4 * a * b + 2 * b * c + 2 * a * c + c);
			clusterLines.reserve(size);

			for (int i = 0; i < imageSize.x; i++)
				for (int j = 0; j < imageSize.y; j++)
					for (int k = 0; k < imageSize.z; k++)
					{
						vec3f invImageSize = vec3f(1.0 / imageSize.x, 1.0 / imageSize.y, 1.0 / imageSize.z);
						float nearFarRatio = m_sceneLights.m_far / m_sceneLights.m_near;
						float maxDepthBound = -pow(nearFarRatio, (k + 1) * invImageSize.z) * m_sceneLights.m_near;
						float minDepthBound;
						if (k == 0) minDepthBound = 1.0;
						else minDepthBound = -pow(nearFarRatio, k * invImageSize.z) * m_sceneLights.m_near;

						vec2f frustrumSize = vec2f(-m_sceneLights.m_tanFovX * maxDepthBound, -m_sceneLights.m_tanFovY * maxDepthBound);
						vec2f cellSize = 2.f * vec2f(frustrumSize.x * invImageSize.x, frustrumSize.y * invImageSize.y);

						vec2f s = vec2f(i * cellSize.x, j * cellSize.y) - frustrumSize;
						vec4f m = vec4f(s.x, s.y, minDepthBound, 1);
						vec4f M = vec4f(s.x + cellSize.x, s.y + cellSize.y, maxDepthBound, 1);

						clusterLines.push_back({ m, color }); clusterLines.push_back({ vec4f(M.x, m.y, m.z, 1.f) , color });
						clusterLines.push_back({ m, color }); clusterLines.push_back({ vec4f(m.x, M.y, m.z, 1.f) , color });
						clusterLines.push_back({ m, color }); clusterLines.push_back({ vec4f(m.x, m.y, M.z, 1.f) , color });

						if (j == imageSize.y - 1)
						{
							clusterLines.push_back({ vec4f(m.x, M.y, m.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, M.y, m.z, 1.f) , color });
							clusterLines.push_back({ vec4f(m.x, M.y, m.z, 1.f), color }); clusterLines.push_back({ vec4f(m.x, M.y, M.z, 1.f) , color });

							if (i == imageSize.x - 1)
							{
								 clusterLines.push_back({ vec4f(M.x, M.y, m.z, 1.f), color }); 
								 clusterLines.push_back({ M , color });
							}
						}
						if (i == imageSize.x - 1)
						{
							clusterLines.push_back({ vec4f(M.x, m.y, m.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, M.y, m.z, 1.f) , color });
							clusterLines.push_back({ vec4f(M.x, m.y, m.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, m.y, M.z, 1.f) , color });
						}

						if (k == imageSize.z - 1)
						{
							clusterLines.push_back({ vec4f(m.x, m.y, M.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, m.y, M.z, 1.f) , color });
							clusterLines.push_back({ vec4f(m.x, m.y, M.z, 1.f), color }); clusterLines.push_back({ vec4f(m.x, M.y, M.z, 1.f) , color });
							clusterLines.push_back({ vec4f(m.x, M.y, M.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, M.y, M.z, 1.f) , color });
							clusterLines.push_back({ vec4f(M.x, m.y, M.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, M.y, M.z, 1.f) , color });
						}
					}
		}

		Debug::drawMultiplePrimitive(clusterLines.data(), (unsigned int)clusterLines.size(), mainCamera->getModelMatrix(), GL_LINES);
	}

	if (m_drawOcclusionBuffer)
	{
		fullScreenDraw(&occlusionTexture, occlusionResultDraw);
	}
#endif // USE_IMGUI
}
//