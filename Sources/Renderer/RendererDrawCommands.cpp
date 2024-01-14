#include "Renderer.h"

#include <iostream>
#include <sstream>

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <EntityComponent/Entity.hpp>
#include <HUD/WidgetManager.h>
#include <Resources/ResourceManager.h>
#include <Resources/Skeleton.h>
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>


//#include <World/WorldComponents/Map.h>
#include <Terrain/Chunk.h>
#include <Animation/AnimationComponent.h>


//	Drawing functions
void Renderer::drawObject(Entity* object, Shader* forceShader)
{
	DrawableComponent* drawableComp = object->getComponent<DrawableComponent>();
	SkeletonComponent* skeletonComp = object->getComponent<SkeletonComponent>();
	bool isSkinned = skeletonComp && skeletonComp->isValid();

	Shader* shaderToUse = forceShader;
	if (!shaderToUse)
	{
		if (renderOption == RenderOption::BOUNDING_BOX)
			shaderToUse = isSkinned ? defaultShader[INSTANCE_ANIMATABLE_BB] : defaultShader[INSTANCE_DRAWABLE_BB];
		else 
			shaderToUse = drawableComp->getShader()->getVariant(Shader::computeVariantCode(false, 0, renderOption == RenderOption::WIREFRAME));
	}

	ModelMatrix modelMatrix = {object->getWorldTransformMatrix(), object->getNormalMatrix()};
	loadModelMatrix(shaderToUse, &modelMatrix);
	if (!shaderToUse)
		return;

	loadGlobalUniforms(shaderToUse);

	// animation uniforms
	if (isSkinned)
	{
		//	Load skeleton pose matrix list for vertex skinning calculation
		const std::vector<mat4f>& pose = skeletonComp->getPose();
		int loc = shaderToUse->getUniformLocation("skeletonPose");
		if (loc >= 0) glUniformMatrix4fv(loc, (int)pose.size(), FALSE, (float*)pose.data());

		//	Load inverse bind pose matrix list for vertex skinning calculation
		const std::vector<mat4f>& bind = skeletonComp->getInverseBindPose();
		loc = shaderToUse->getUniformLocation("inverseBindPose");
		if (loc >= 0) glUniformMatrix4fv(loc, (int)bind.size(), FALSE, (float*)bind.data());
	}

	//	Draw mesh
	if (renderOption == RenderOption::BOUNDING_BOX)
	{
		if (isSkinned)
		{
			//loadVAO(skeletonComp->getCapsuleVAO());
			//glDrawArrays(GL_POINTS, 0, (int)skeletonComp->getSegmentsIndex().size());
		}
		else
		{
			loadVAO(drawableComp->getMesh()->getBBoxVAO());
			glDrawElements(GL_TRIANGLES, (int)drawableComp->getMesh()->getBBoxFaces()->size(), GL_UNSIGNED_SHORT, NULL);
		}
	}
	else
	{
		loadVAO(drawableComp->getMesh()->getVAO());
		glDrawElements(GL_TRIANGLES, (int)drawableComp->getMesh()->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		if (renderOption == RenderOption::NORMALS && normalViewer)
		{
			loadModelMatrix(normalViewer, &modelMatrix);
			loadVAO(drawableComp->getMesh()->getVAO());
			glDrawElements(GL_TRIANGLES, (int)drawableComp->getMesh()->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

			drawCalls++;
			instanceDrawn++;
			trianglesDrawn += drawableComp->getMesh()->getNumberFaces();
		}
	}
	drawCalls++;
	instanceDrawn++;
	trianglesDrawn += drawableComp->getMesh()->getNumberFaces();
}
void Renderer::drawInstancedObject(Shader* s, Mesh* m, std::vector<ModelMatrix>& models)
{
	//	Get shader and prepare matrix
	Shader* shaderToUse;
	if (renderOption == RenderOption::BOUNDING_BOX)
		shaderToUse = defaultShader[INSTANCE_DRAWABLE_BB];
	else
		shaderToUse = s;

	//	Load MVP matrix
	loadModelMatrix(shaderToUse, models.data(), (int)models.size());
	if (!shaderToUse)
		return;

	loadGlobalUniforms(shaderToUse);

	//	Draw instanced
	if (renderOption == RenderOption::BOUNDING_BOX)
	{
		loadVAO(m->getBBoxVAO());
		glDrawElementsInstanced(GL_TRIANGLES, (int)m->getBBoxFaces()->size(), GL_UNSIGNED_SHORT, NULL, (unsigned short)models.size());
	}
	else
	{
		loadVAO(m->getVAO());
		glDrawElementsInstanced(GL_TRIANGLES, (int)m->getFaces()->size(), GL_UNSIGNED_SHORT, NULL, (unsigned short)models.size());

		if (renderOption == RenderOption::NORMALS && normalViewer)
		{
			loadModelMatrix(normalViewer, models.data(), (int)models.size());
			loadVAO(m->getVAO());
			glDrawElementsInstanced(GL_TRIANGLES, (int)m->getFaces()->size(), GL_UNSIGNED_SHORT, NULL, (unsigned short)models.size());

			drawCalls++;
			instanceDrawn += (int)(models.size());
			trianglesDrawn += (int)(models.size() * m->getNumberFaces());
		}
	}
	drawCalls++;
	instanceDrawn += (int)(models.size());
	trianglesDrawn += (int)(models.size() * m->getNumberFaces());
}
/*void Renderer::drawMap(Map* map, Shader* s)
{
	mat4f scale = mat4f::scale(mat4f::identity, map->getScale());
	mat4f model = scale * map->getModelMatrix();
	ModelMatrix modelMatrix = { scale * map->getModelMatrix(), map->getNormalMatrix() };

	// raw
	loadModelMatrix(map->getShader(), &modelMatrix);
	loadGlobalUniforms(map->getShader());
	vec4i exclusion = map->getExclusionZone();
	int loc = map->getShader()->getUniformLocation("exclusion");
	//if (loc >= 0) glUniform4iv(loc, 1, (int*)&exclusion);
	if (loc >= 0) glUniform4iv(loc, 1, &vec4i(-1,0,0,0)[0]);
	
	loadVAO(map->getVAO());
	glDrawElements(GL_TRIANGLES, map->getFacesCount(), GL_UNSIGNED_INT, NULL);
	drawCalls++;
	instanceDrawn++;
	trianglesDrawn += map->getFacesCount() / 6;
	//return;

	// chunks
	exclusion = vec4i(-1, 0, 0, 0);
	if (loc >= 0) glUniform4iv(loc, 1, (int*)&exclusion);
	loc = map->getShader()->getUniformLocation("overrideColor");
	vec4f color;

	std::vector<vec2i> chunksIndexes = map->getDrawableChunks();
	for (int i = 0; i < chunksIndexes.size(); i++)
	{
		vec2i v = chunksIndexes[i];
		Chunk* chunk = map->getChunk(v.x, v.y);
		if (chunk->isInitialized())
		{
			model = scale * chunk->getModelMatrix();
			modelMatrix.model = model;
			loadModelMatrix(map->getShader(), &modelMatrix);
			loadVAO(chunk->getVAO());

			int lod = chunk->getLod();
			int lodLeft  = map->inBound(v.x - 1, v.y) ? map->getChunk(v.x - 1, v.y)->getLod() : lod;
			int lodRight = map->inBound(v.x + 1, v.y) ? map->getChunk(v.x + 1, v.y)->getLod() : lod;
			int lodUp    = map->inBound(v.x, v.y + 1) ? map->getChunk(v.x, v.y + 1)->getLod() : lod;
			int lodDown  = map->inBound(v.x, v.y - 1) ? map->getChunk(v.x, v.y - 1)->getLod() : lod;

			glDrawElements(GL_TRIANGLES, chunk->getCenterFacesCount(), GL_UNSIGNED_INT, NULL);
			trianglesDrawn += chunk->getCenterFacesCount() / 6;
			
			unsigned int offset;
			if (lod > lodUp)
			{
				offset = chunk->getCenterFacesCount() + 4 * chunk->getBorderFacesCount();
				glDrawElements(GL_TRIANGLES, chunk->getSeamlessBorderFacesCount(), GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
				trianglesDrawn += chunk->getSeamlessBorderFacesCount() / 6;
			}
			else
			{
				offset = chunk->getCenterFacesCount();
				glDrawElements(GL_TRIANGLES, chunk->getBorderFacesCount(), GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
				trianglesDrawn += chunk->getBorderFacesCount() / 6;
			}

			if (lod > lodDown) 
			{
				offset = chunk->getCenterFacesCount() + 4 * chunk->getBorderFacesCount() + chunk->getSeamlessBorderFacesCount();
				glDrawElements(GL_TRIANGLES, chunk->getSeamlessBorderFacesCount(), GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
				trianglesDrawn += chunk->getSeamlessBorderFacesCount() / 6;
			}
			else 
			{
				offset = chunk->getCenterFacesCount() + chunk->getBorderFacesCount();
				glDrawElements(GL_TRIANGLES, chunk->getBorderFacesCount(), GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
				trianglesDrawn += chunk->getBorderFacesCount() / 6;
			}

			if (lod > lodLeft)
			{
				offset = chunk->getCenterFacesCount() + 4 * chunk->getBorderFacesCount() + 2 * chunk->getSeamlessBorderFacesCount();
				glDrawElements(GL_TRIANGLES, chunk->getSeamlessBorderFacesCount(), GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
				trianglesDrawn += chunk->getSeamlessBorderFacesCount() / 6;
			}
			else
			{
				offset = chunk->getCenterFacesCount() + 2 * chunk->getBorderFacesCount();
				glDrawElements(GL_TRIANGLES, chunk->getBorderFacesCount(), GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
				trianglesDrawn += chunk->getBorderFacesCount() / 6;
			}

			if (lod > lodRight)
			{
				offset = chunk->getCenterFacesCount() + 4 * chunk->getBorderFacesCount() + 3 * chunk->getSeamlessBorderFacesCount();
				glDrawElements(GL_TRIANGLES, chunk->getSeamlessBorderFacesCount(), GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
				trianglesDrawn += chunk->getSeamlessBorderFacesCount() / 6;
			}
			else
			{
				offset = chunk->getCenterFacesCount() + 3 * chunk->getBorderFacesCount();
				glDrawElements(GL_TRIANGLES, chunk->getBorderFacesCount(), GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
				trianglesDrawn += chunk->getBorderFacesCount() / 6;
			}

			drawCalls++;
			instanceDrawn++;
		}
	}
	color = vec4f(-1.f, 0.f, 0.f, 1.f);
	if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);
}*/


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


GLuint Renderer::renderMeshOverview(Mesh* mesh, float angle0, float angle1)
{
	if (!mesh)
		return 0;

	Sphere bounding = mesh->getBoundingBox().toSphere();
	float trackballRadius = 2.f * bounding.radius;
	vec4f forward = vec4f(sin(angle0) * cos(angle1),  sin(angle1), cos(angle0) * cos(angle1),0);
	vec4f right = vec4f::cross(forward, vec4f(0, 1, 0, 0));
	right.normalize();
	vec4f up = vec4f::cross(forward, right);
	vec4f camPosition = bounding.center + trackballRadius * forward;
	camPosition.w = 1;
	mat4f camTransform(right, up, forward, camPosition);
	mat4f view = mat4f::inverse(camTransform);

	m_globalMatrices.view = view;
	m_globalMatrices.projection = mat4f::perspective((float)DEG2RAD * 90.f, (float)overviewTexture.size.x / overviewTexture.size.y, 0.1f, 3 * trackballRadius);
	m_globalMatrices.cameraPosition = camPosition;
	glBindBuffer(GL_UNIFORM_BUFFER, m_globalMatricesID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_globalMatrices), &m_globalMatrices);

	glBindFramebuffer(GL_FRAMEBUFFER, overviewFBO);
	glViewport(0, 0, overviewTexture.size.x, overviewTexture.size.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	Renderer::ModelMatrix modelMatrix = { mat4f::identity,  mat4f::identity };
	Renderer::getInstance()->loadModelMatrix(defaultShader[DEFAULT], &modelMatrix);

	vec4f color = vec4f(1, 1, 1, 1);
	int loc = defaultShader[DEFAULT]->getUniformLocation("overrideColor");
	if (loc >= 0)
		glUniform4fv(loc, 1, (float*)&color);

	glBindVertexArray(mesh->getVAO());
	glDrawElements(GL_TRIANGLES, (int)mesh->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

	if (loc >= 0) glUniform4fv(loc, 1, (float*)&vec4f(-1.f, 0.f, 0.f, 1.f)[0]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return overviewTexture.getTextureId();
}
//





