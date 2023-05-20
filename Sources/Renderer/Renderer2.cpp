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


#include <World/WorldComponents/Map.h>
#include <Terrain/Chunk.h>


//	Drawing functions
void Renderer::drawObject(Entity* object)
{
	DrawableComponent* drawableComp = object->getComponent<DrawableComponent>();
	SkeletonComponent* skeletonComp = object->getComponent<SkeletonComponent>();
	bool isSkinned = skeletonComp && skeletonComp->isValid();

	Shader* shaderToUse;
	if (renderOption == RenderOption::BOUNDING_BOX)
		shaderToUse = isSkinned ? defaultShader[INSTANCE_ANIMATABLE_BB] : defaultShader[INSTANCE_DRAWABLE_BB];
	else 
		shaderToUse = drawableComp->getShader()->getVariant(Shader::computeVariantCode(false, false, renderOption == RenderOption::WIREFRAME));

	loadModelMatrix(shaderToUse, &object->getWorldTransformMatrix(), &object->getNormalMatrix());
	if (!shaderToUse)
		return;

	int loc = shaderToUse->getUniformLocation("lightCount");
	if (loc >= 0) glUniform1i(loc, m_lightCount);

	// animation uniforms
	if (isSkinned)
	{
		//	Load skeleton pose matrix list for vertex skinning calculation
		std::vector<mat4f> pose = skeletonComp->getPose();
		int loc = shaderToUse->getUniformLocation("skeletonPose");
		if (loc >= 0) glUniformMatrix4fv(loc, (int)pose.size(), FALSE, (float*)pose.data());

		//	Load inverse bind pose matrix list for vertex skinning calculation
		std::vector<mat4f> bind;
		bind = skeletonComp->getInverseBindPose();
		loc = shaderToUse->getUniformLocation("inverseBindPose");
		if (loc >= 0) glUniformMatrix4fv(loc, (int)bind.size(), FALSE, (float*)bind.data());
	}

	//	Draw mesh
	if (renderOption == RenderOption::BOUNDING_BOX)
	{
		if (isSkinned)
		{
			loadVAO(skeletonComp->getCapsuleVAO());
			glDrawArrays(GL_POINTS, 0, (int)skeletonComp->getSegmentsIndex().size());
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
			loadModelMatrix(normalViewer, &object->getWorldTransformMatrix(), &object->getNormalMatrix());
			loadVAO(drawableComp->getMesh()->getVAO());
			glDrawElements(GL_TRIANGLES, (int)drawableComp->getMesh()->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);
		}
	}
	instanceDrawn++;
	trianglesDrawn += drawableComp->getMesh()->getNumberFaces();
}
void Renderer::drawInstancedObject(Shader* s, Mesh* m, std::vector<mat4f>& models, std::vector<mat4f>& normalMatrices)
{
	//	Get shader and prepare matrix
	Shader* shaderToUse;
	if (renderOption == RenderOption::BOUNDING_BOX)
		shaderToUse = defaultShader[INSTANCE_DRAWABLE_BB];
	else
		shaderToUse = s->getVariant(Shader::computeVariantCode(true, false, renderOption == RenderOption::WIREFRAME));

	//	Load MVP matrix
	loadModelMatrix(shaderToUse, models.data(), normalMatrices.data(), (int)models.size());

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
	}
	instanceDrawn += (int)(models.size());
	trianglesDrawn += (int)(models.size() * m->getNumberFaces());
}
void Renderer::drawMap(Map* map, Shader* s)
{
	mat4f scale = mat4f::scale(mat4f::identity, map->getScale());
	mat4f model = scale * map->getModelMatrix();
	mat4f normalMatrix = map->getNormalMatrix();

	// raw
	loadModelMatrix(map->getShader(), &model, &normalMatrix);
	vec4i exclusion = map->getExclusionZone();
	int loc = map->getShader()->getUniformLocation("exclusion");
	//if (loc >= 0) glUniform4iv(loc, 1, (int*)&exclusion);
	if (loc >= 0) glUniform4iv(loc, 1, &vec4i(-1,0,0,0)[0]);
	
	loadVAO(map->getVAO());
	glDrawElements(GL_TRIANGLES, map->getFacesCount(), GL_UNSIGNED_INT, NULL);
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
			loadModelMatrix(map->getShader(), &model, &normalMatrix);
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
			
			instanceDrawn++;
		}
	}
	color = vec4f(-1.f, 0.f, 0.f, 1.f);
	if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);
}
//





