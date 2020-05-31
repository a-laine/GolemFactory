#include "Renderer.h"

#include <iostream>
#include <sstream>

#include <glm/gtx/vector_angle.hpp>

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
void Renderer::drawMap(Map* map, const float* view, const float* projection)
{
	glm::mat4 m = map->getModelMatrix();
	lastShader = nullptr;

	// raw
	loadMVPMatrix(defaultShader[INSTANCE_DRAWABLE_WIRED], &m[0][0], view, projection);

	loadVAO(map->getVAO());
	glDrawElements(GL_TRIANGLES, map->getFacesCount(), GL_UNSIGNED_INT, NULL);

	// chunks
	std::vector<glm::ivec2> chunksIndexes = map->getDrawableChunks();
	for (int i = 0; i < chunksIndexes.size(); i++)
	{
		Chunk* chunk = map->getChunk(chunksIndexes[i].x, chunksIndexes[i].y);

		if (chunk->getNeedVBOUpdate())
			chunk->updateVBO();

		loadMVPMatrix(defaultShader[INSTANCE_DRAWABLE_WIRED], chunk->getModelMatrixPtr(), view, projection);

		loadVAO(chunk->getVAO());
		glDrawElements(GL_TRIANGLES, chunk->getFacesCount(), GL_UNSIGNED_INT, NULL);
	}

	instanceDrawn++;
	trianglesDrawn += map->getFacesCount();
}
/*
void Renderer::drawTriangle(const Triangle* triangle, const float* view, const float* projection)
{
	std::map<Shape::ShapeType, std::pair<Mesh*, Shader*> >::iterator it = drawShapeDefinition.find(Shape::TRIANGLE);
	if (it != drawShapeDefinition.end())
	{
		//	Get shader and prepare matrix
		Shader* shaderToUse = it->second.second;
		loadMVPMatrix(shaderToUse, &glm::translate(glm::mat4(1.f), triangle->p1)[0][0], view, projection);
		if (!shaderToUse) return;

		int loc = shaderToUse->getUniformLocation("wired");
		if (loc >= 0) glUniform1i(loc, (renderOption == WIREFRAME) ? 1 : 0);
		loc = shaderToUse->getUniformLocation("vector1");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&(triangle->p2 - triangle->p1)[0]);
		loc = shaderToUse->getUniformLocation("vector2");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&(triangle->p3 - triangle->p1)[0]);

		//	override mesh color
		loc = shaderToUse->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&COLOR_TRIANGLE);

		//	Draw mesh
		loadVAO(it->second.first->getVAO());
		glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else std::cerr << "WARNING : drawTriangle not associated or not yet implemented" << std::endl;
}
*/
//





