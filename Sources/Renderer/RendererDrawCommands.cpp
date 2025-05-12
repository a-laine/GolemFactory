#include "Renderer.h"

#include <iostream>
#include <sstream>

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <EntityComponent/Entity.hpp>
#include <HUD/WidgetManager.h>
#include <Resources/ResourceManager.h>
#include <Resources/Skeleton.h>
#include <Resources/Material.h>
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>

#include <Animation/AnimationComponent.h>


//	Drawing functions
void Renderer::drawObject(Entity* object, Shader* forceShader)
{
	DrawableComponent* drawableComp = object->getComponent<DrawableComponent>();
	SkeletonComponent* skeletonComp = object->getComponent<SkeletonComponent>();
	bool isSkinned = skeletonComp && skeletonComp->isValid();
	Material* material = drawableComp->getMaterial();

	Shader* shader = forceShader;
	if (!shader)
		shader = material->getShader()->getVariant(Shader::computeVariantCode(false, 0, renderOption == RenderOption::WIREFRAME));
	if (!shader)
		return;

	Renderer::ModelMatrix modelMatrix = { object->getWorldTransformMatrix(), object->getNormalMatrix() };
	bindMaterial(material, shader);
	loadMatrices(shader, (float*)&modelMatrix, 1);
	if (drawableComp->getInstanceDataSize() > 0)
		drawableComp->pushInstanceData(shader);
	if (drawableComp->hasConstantData())
		drawableComp->pushConstantData(shader);

	// animation uniforms
	if (isSkinned)
	{
		//	Load skeleton pose matrix list for vertex skinning calculation
		const std::vector<mat4f>& pose = skeletonComp->getPose();
		int loc = shader->getUniformLocation("skeletonPose");
		if (loc >= 0) glUniformMatrix4fv(loc, (int)pose.size(), FALSE, (float*)pose.data());

		if (skeletonComp->getSkeleton() != lastSkeleton)
		{
			//	Load inverse bind pose matrix list for vertex skinning calculation
			lastSkeleton = skeletonComp->getSkeleton();
			const std::vector<mat4f>& bind = skeletonComp->getInverseBindPose();
			loc = shader->getUniformLocation("inverseBindPose");
			if (loc >= 0) glUniformMatrix4fv(loc, (int)bind.size(), FALSE, (float*)bind.data());
		}
	}

	//	Draw mesh
	const Mesh* mesh = drawableComp->getMesh();
	if (drawableComp->hasCustomDraw())
	{
		drawableComp->customDraw(this, instanceDrawn, drawCalls, trianglesDrawn);
	}
	else
	{
		loadVAO(mesh->getVAO());
		glDrawElements(GL_TRIANGLES, mesh->getNumberIndices(), mesh->getIndicesType(), NULL);
		trianglesDrawn += mesh->getNumberFaces();
		instanceDrawn++;
		drawCalls++;

		if (renderOption == RenderOption::NORMALS && normalViewer)
		{
			bindMaterial(nullptr, normalViewer);
			loadMatrices(normalViewer, (float*)&modelMatrix);

			loadVAO(mesh->getVAO());
			glDrawElements(GL_TRIANGLES, mesh->getNumberIndices(), mesh->getIndicesType(), NULL);

			drawCalls++;
			instanceDrawn++;
			trianglesDrawn += mesh->getNumberFaces();
		}
	}
}
void Renderer::drawInstancedObject(Material* _material, Shader* _shader, Mesh* _mesh, float* _matrices, vec4f* _instanceDatas, 
	unsigned short _dataSize, unsigned short _instanceCount, DrawableComponent* _constantDataRef)
{
	//	Load MVP matrix
	bindMaterial(_material, _shader);
	loadMatrices(_shader, _matrices, _instanceCount);
	if (_dataSize)
		loadInstanceDatas(_shader, _instanceDatas, _dataSize, _instanceCount);
	if (_constantDataRef)
		_constantDataRef->pushConstantData(_shader);

	//	Draw instanced
	loadVAO(_mesh->getVAO());
	glDrawElementsInstanced(GL_TRIANGLES, _mesh->getNumberIndices(), _mesh->getIndicesType(), NULL, _instanceCount);

	if (renderOption == RenderOption::NORMALS && normalViewer)
	{
		bindMaterial(nullptr, normalViewer);
		loadMatrices(normalViewer, _matrices, _instanceCount);
		loadVAO(_mesh->getVAO());
		glDrawElementsInstanced(GL_TRIANGLES, _mesh->getNumberIndices(), _mesh->getIndicesType(), NULL, _instanceCount);

		drawCalls++;
		instanceDrawn += _instanceCount;
		trianglesDrawn += (int)(_instanceCount * _mesh->getNumberFaces());
	}
	drawCalls++;
	instanceDrawn += _instanceCount;
	trianglesDrawn += (int)(_instanceCount * _mesh->getNumberFaces());
}

void Renderer::fullScreenDraw(const Texture* texture, Shader* shader, float alpha, bool bindIntoImage)
{
	if (!texture)
		return;
	if (!shader && !fullscreenTriangle)
		return;

	lastShader = shader ? shader : fullscreenTriangle;
	lastVAO = fullscreenVAO;
	lastShader->enable();

	glEnable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0);
	if (bindIntoImage)
	{
		glBindImageTexture(0, texture->getTextureId(), 0, texture->size.z > 0, 0, GL_READ_ONLY, texture->m_internalFormat);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
	}

	int loc = lastShader->getUniformLocation("alpha");
	if (loc >= 0)
		glUniform1f(loc, alpha);

	glBindVertexArray(fullscreenVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	drawCalls++;
	instanceDrawn++;
	trianglesDrawn++;
}


GLuint Renderer::renderMeshOverview(Mesh* mesh, float angle0, float angle1, float zoom)
{
	if (!mesh)
		return 0;

	Sphere bounding = mesh->getBoundingBox().toSphere();
	float trackballRadius = 2.f * bounding.radius;
	vec4f forward = vec4f(sin(angle0) * cos(angle1),  sin(angle1), cos(angle0) * cos(angle1),0);
	vec4f right = vec4f::cross(forward, vec4f(0, 1, 0, 0));
	right.normalize();
	vec4f up = vec4f::cross(forward, right);
	vec4f camPosition = bounding.center + (trackballRadius * zoom) * forward;
	camPosition.w = 1;
	mat4f camTransform(right, up, forward, camPosition);
	mat4f view = mat4f::inverse(camTransform);

	m_globalMatrices.view = view;
	m_globalMatrices.projection = mat4f::perspective((float)DEG2RAD * 90.f, (float)overviewTexture.size.x / overviewTexture.size.y, 0.01f, 3 * trackballRadius);
	m_globalMatrices.cameraPosition = camPosition;
	glBindBuffer(GL_UNIFORM_BUFFER, m_globalMatricesID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_globalMatrices), &m_globalMatrices);

	glBindFramebuffer(GL_FRAMEBUFFER, overviewFBO);
	glViewport(0, 0, overviewTexture.size.x, overviewTexture.size.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	Shader* shader = defaultShader[DEFAULT];
	if (mesh->getNormals()->empty())
		shader = defaultShader[DEFAULT]->getVariant(Shader::computeVariantCode(false, 0, true));

	Renderer::ModelMatrix modelMatrix = { mat4f::identity, mat4f::identity };
	bindMaterial(nullptr, shader);
	loadMatrices(shader, (float*)&modelMatrix);

	vec4f color = vec4f(1, 1, 1, 1);
	int loc = shader->getUniformLocation("overrideColor");
	if (loc >= 0)
		glUniform4fv(loc, 1, (float*)&color);

	glBindVertexArray(mesh->getVAO());
	glDrawElements(GL_TRIANGLES, mesh->getNumberIndices(), mesh->getIndicesType(), NULL);

	if (loc >= 0) glUniform4fv(loc, 1, (float*)&vec4f(-1.f, 0.f, 0.f, 1.f)[0]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return overviewTexture.getTextureId();
}
//





