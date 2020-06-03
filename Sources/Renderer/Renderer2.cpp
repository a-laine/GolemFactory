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


#include "World/WorldComponents/Map.h"
#include "Terrain/Chunk.h"


//	Drawing functions
void Renderer::drawObject(Entity* object, const float* view, const float* projection)
{
	ShaderIdentifier shaderType = INSTANCE_DRAWABLE;
	ShaderIdentifier shaderBBType = INSTANCE_DRAWABLE_BB;
	ShaderIdentifier shaderWire = INSTANCE_DRAWABLE_WIRED;
	DrawableComponent* drawableComp = object->getComponent<DrawableComponent>();
	SkeletonComponent* skeletonComp = object->getComponent<SkeletonComponent>();
	if (!drawableComp || !drawableComp->isValid()) return;
	if (skeletonComp && skeletonComp->isValid())
	{
		shaderType = INSTANCE_ANIMATABLE;
		shaderBBType = INSTANCE_ANIMATABLE_BB;
		shaderWire = INSTANCE_ANIMATABLE_WIRED;
	}

	//	Get shader and prepare matrix
	Shader* shaderToUse;
	if (renderOption == BOUNDING_BOX) shaderToUse = defaultShader[shaderBBType];
	else if (renderOption == WIREFRAME) shaderToUse = defaultShader[shaderWire];
	else shaderToUse = defaultShader[shaderType];
	if (!shaderToUse) shaderToUse = drawableComp->getShader();
	loadMVPMatrix(shaderToUse, &object->getMatrix()[0][0], view, projection);
	if (!shaderToUse) return;

	if (shaderType == INSTANCE_ANIMATABLE)
	{
		//	Load skeleton pose matrix list for vertex skinning calculation
		std::vector<glm::mat4> pose = skeletonComp->getPose();
		int loc = shaderToUse->getUniformLocation("skeletonPose");
		if (loc >= 0) glUniformMatrix4fv(loc, (int)pose.size(), FALSE, (float*)pose.data());

		//	Load inverse bind pose matrix list for vertex skinning calculation
		std::vector<glm::mat4> bind;
		bind = skeletonComp->getInverseBindPose();
		loc = shaderToUse->getUniformLocation("inverseBindPose");
		if (loc >= 0) glUniformMatrix4fv(loc, (int)bind.size(), FALSE, (float*)bind.data());
	}

	//	Draw mesh
	if (renderOption == BOUNDING_BOX && shaderType == INSTANCE_ANIMATABLE)
	{
		loadVAO(skeletonComp->getCapsuleVAO());
		glDrawArrays(GL_POINTS, 0, (int)skeletonComp->getSegmentsIndex().size());
	}
	else if (renderOption == BOUNDING_BOX)
	{
		loadVAO(drawableComp->getMesh()->getBBoxVAO());
		glDrawElements(GL_TRIANGLES, (int)drawableComp->getMesh()->getBBoxFaces()->size(), GL_UNSIGNED_SHORT, NULL);
	}
	else
	{
		loadVAO(drawableComp->getMesh()->getVAO());
		glDrawElements(GL_TRIANGLES, (int)drawableComp->getMesh()->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);
	}
	instanceDrawn++;
	trianglesDrawn += drawableComp->getMesh()->getNumberFaces();
}
void Renderer::drawInstancedObject(Shader* s, Mesh* m, std::vector<glm::mat4>& models, const float* view, const float* projection)
{
	//	Get shader and prepare matrix
	Shader* shaderToUse;
	if (renderOption == BOUNDING_BOX) shaderToUse = defaultShader[INSTANCE_DRAWABLE_BB];
	else if (renderOption == WIREFRAME) shaderToUse = defaultShader[INSTANCE_DRAWABLE_WIRED];
	else shaderToUse = defaultShader[INSTANCE_DRAWABLE];
	if (!shaderToUse || !shaderToUse->getInstanciable()) shaderToUse = s;
	else shaderToUse = shaderToUse->getInstanciable();

	//	Load MVP matrix
	loadMVPMatrix(shaderToUse, (const float*)models.data(), view, projection, (int)models.size());

	//	Draw instanced
	if (renderOption == BOUNDING_BOX)
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
void Renderer::drawMap(Map* map, const float* view, const float* projection, Shader* s)
{
	glm::mat4 scale = glm::scale(glm::mat4(1.f), map->getScale());
	glm::mat4 model = scale * map->getModelMatrix();

	// raw
	loadMVPMatrix(map->getShader(), &model[0][0], view, projection);
	glm::ivec4 exclusion = map->getExclusionZone();
	int loc = map->getShader()->getUniformLocation("exclusion");
	if (loc >= 0) glUniform4iv(loc, 1, (int*)&exclusion);
	
	loadVAO(map->getVAO());
	glDrawElements(GL_TRIANGLES, map->getFacesCount(), GL_UNSIGNED_INT, NULL);
	instanceDrawn++;
	trianglesDrawn += map->getFacesCount() / 6;

	// chunks
	exclusion = glm::ivec4(-1, 0, 0, 0);
	if (loc >= 0) glUniform4iv(loc, 1, (int*)&exclusion);
	loc = map->getShader()->getUniformLocation("overrideColor");
	glm::vec3 color;

	std::vector<glm::ivec2> chunksIndexes = map->getDrawableChunks();
	for (int i = 0; i < chunksIndexes.size(); i++)
	{
		glm::ivec2 v = chunksIndexes[i];
		Chunk* chunk = map->getChunk(v.x, v.y);
		if (chunk->isInitialized())
		{
			model = scale * chunk->getModelMatrix();
			loadMVPMatrix(map->getShader(), &model[0][0], view, projection);
			//color = 0.25f * glm::vec3(lod % 3, (lod / 3) % 3, (lod / 9) % 3) + glm::vec3(0.25f);
			//if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);
			loadVAO(chunk->getVAO());

			int lod = chunk->getLod();
			int lodLeft  = map->inBound(v.x - 1, v.y) ? map->getChunk(v.x - 1, v.y)->getLod() : lod;
			int lodRight = map->inBound(v.x + 1, v.y) ? map->getChunk(v.x + 1, v.y)->getLod() : lod;
			int lodUp    = map->inBound(v.x, v.y + 1) ? map->getChunk(v.x, v.y + 1)->getLod() : lod;
			int lodDown  = map->inBound(v.x, v.y - 1) ? map->getChunk(v.x, v.y - 1)->getLod() : lod;

			//color = glm::vec3(1, 1, 1); if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);
			glDrawElements(GL_TRIANGLES, chunk->getCenterFacesCount(), GL_UNSIGNED_INT, NULL);
			trianglesDrawn += chunk->getCenterFacesCount() / 6;
			
			unsigned int offset;
			//color = glm::vec3(1, 0, 0); if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);
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

			//color = glm::vec3(0, 1, 0); if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);
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

			//color = glm::vec3(0, 0, 1); if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);
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

			//color = glm::vec3(1, 1, 0); if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);
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
	color = glm::vec3(-1.0, 0.0, 0.0);
	if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);
}
//





