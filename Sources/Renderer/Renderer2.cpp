#include "Renderer.h"

#include <iostream>
#include <sstream>

#include <glm/gtx/vector_angle.hpp>

#include <EntityComponent/Entity.hpp>
#include <HUD/WidgetManager.h>
#include <Scene/SceneQueryTests.h>
#include <Resources/ResourceManager.h>
#include <Resources/Skeleton.h>
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>



#define COLOR_POINT    glm::vec3(0.f,0.f,0.f)					// black
#define COLOR_SEGMENT  glm::vec3(1.f, 0.f, 1.f)					// magenta
#define COLOR_TRIANGLE glm::vec3(0.992f, 0.415f, 0.008f)		// orange
#define COLOR_ORIENTEDBOX    glm::vec3(0.2f, 0.2f, 0.2f)		// grey
#define COLOR_AXISALIGNEDBOX COLOR_ORIENTEDBOX					// grey
#define COLOR_SPHERE   glm::vec3(0.103f, 0.103f, 0.403f)		// dark blue
#define COLOR_CAPSULE  glm::vec3(0.f, 0.25f, 0.f)				// dark green


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
void Renderer::drawShape(const Shape* Shape, const float* view, const float* projection)
{
	switch (Shape->type)
	{
		case Shape::POINT:
			drawPoint(static_cast<const Point*>(Shape), view, projection);
			break;
		case Shape::SEGMENT:
			drawSegment(static_cast<const Segment*>(Shape), view, projection);
			break;
		case Shape::TRIANGLE:
			drawTriangle(static_cast<const Triangle*>(Shape), view, projection);
			break;
		case Shape::ORIENTED_BOX:
			drawOrientedBox(static_cast<const OrientedBox*>(Shape), view, projection);
			break;
		case Shape::AXIS_ALIGNED_BOX:
			drawAxisAlignedBox(static_cast<const AxisAlignedBox*>(Shape), view, projection);
			break;
		case Shape::SPHERE:
			drawSphere(static_cast<const Sphere*>(Shape), view, projection);
			break;
		case Shape::CAPSULE:
			drawCapsule(static_cast<const Capsule*>(Shape), view, projection);
			break;
		default:
			std::cerr << "WARNING : Shape type not yet supported" << std::endl;
			break;
	}
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
//


//	Shapes drawing functions
void Renderer::drawPoint(const Point* point, const float* view, const float* projection)
{
	std::map<Shape::ShapeType, std::pair<Mesh*, Shader*> >::iterator it = drawShapeDefinition.find(Shape::POINT);
	if (it != drawShapeDefinition.end())
	{
		//	Get shader and prepare matrix
		Shader* shaderToUse = it->second.second;
		loadMVPMatrix(shaderToUse, &glm::translate(glm::mat4(1.f), point->p)[0][0], view, projection);
		if (!shaderToUse) return;
		
		//	override mesh color
		int loc = shaderToUse->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&COLOR_POINT);

		//	Draw mesh
		loadVAO(it->second.first->getVAO());
		glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else std::cerr << "WARNING : drawPoint not associated or not yet implemented" << std::endl;
}
void Renderer::drawSegment(const Segment* segment, const float* view, const float* projection)
{
	std::map<Shape::ShapeType, std::pair<Mesh*, Shader*> >::iterator it = drawShapeDefinition.find(Shape::SEGMENT);
	if (it != drawShapeDefinition.end())
	{
		//	Get shader and prepare matrix
		Shader* shaderToUse = it->second.second;
		loadMVPMatrix(shaderToUse, &glm::translate(glm::mat4(1.f), segment->p1)[0][0], view, projection);
		if (!shaderToUse) return;

		int loc = shaderToUse->getUniformLocation("vector");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&(segment->p2 - segment->p1)[0]);

		//	override mesh color
		loc = shaderToUse->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&COLOR_SEGMENT);

		//	Draw mesh
		loadVAO(it->second.first->getVAO());
		glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else std::cerr << "WARNING : drawSegment not associated or not yet implemented" << std::endl;
}
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
void Renderer::drawOrientedBox(const OrientedBox* box, const float* view, const float* projection)
{
	///	adapted to "Shapes/box.mesh"
	std::map<Shape::ShapeType, std::pair<Mesh*, Shader*> >::iterator it = drawShapeDefinition.find(Shape::ORIENTED_BOX);
	if (it != drawShapeDefinition.end())
	{
		//	Get shader and prepare matrix
		Shader* shaderToUse = nullptr;
		if (renderOption == WIREFRAME) shaderToUse = defaultShader[INSTANCE_DRAWABLE_WIRED];
		else shaderToUse = it->second.second;

		loadMVPMatrix(shaderToUse, &glm::scale(box->base, 0.5f * (box->max - box->min))[0][0], view, projection);
		if (!shaderToUse) return;

		//	override mesh color
		int loc = shaderToUse->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&COLOR_ORIENTEDBOX);

		//	Draw mesh
		loadVAO(it->second.first->getVAO());
		glDrawElements(GL_TRIANGLES, (int)it->second.first->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else std::cerr << "WARNING : drawOrientedBox not associated or not yet implemented" << std::endl;
}
void Renderer::drawAxisAlignedBox(const AxisAlignedBox* box, const float* view, const float* projection)
{
	///	adapted to "Shapes/box.mesh"
	std::map<Shape::ShapeType, std::pair<Mesh*, Shader*> >::iterator it = drawShapeDefinition.find(Shape::AXIS_ALIGNED_BOX);
	if (it != drawShapeDefinition.end())
	{
		//	Get shader and prepare matrix
		Shader* shaderToUse = nullptr;
		if (renderOption == WIREFRAME) shaderToUse = defaultShader[INSTANCE_DRAWABLE_WIRED];
		else shaderToUse = it->second.second;

		glm::mat4 model = glm::translate(glm::mat4(1.f), 0.5f * (box->max + box->min));
		model = glm::scale(model, 0.5f * (box->max - box->min));
		loadMVPMatrix(shaderToUse, &model[0][0], view, projection);
		if (!shaderToUse) return;

		//	override mesh color
		int loc = shaderToUse->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&COLOR_AXISALIGNEDBOX);

		//	Draw mesh
		loadVAO(it->second.first->getVAO());
		glDrawElements(GL_TRIANGLES, (int)it->second.first->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else std::cerr << "WARNING : drawAxisAlignedBox not associated or not yet implemented" << std::endl;
}
void Renderer::drawSphere(const Sphere* sphere, const float* view, const float* projection)
{
	///	adapted to "Shapes/sphere.mesh"
	std::map<Shape::ShapeType, std::pair<Mesh*, Shader*> >::iterator it = drawShapeDefinition.find(Shape::SPHERE);
	if (it != drawShapeDefinition.end())
	{
		//	Get shader and prepare matrix
		Shader* shaderToUse = nullptr;
		if (renderOption == WIREFRAME) shaderToUse = defaultShader[INSTANCE_DRAWABLE_WIRED];
		else shaderToUse = it->second.second;
		loadMVPMatrix(shaderToUse, &glm::translate(sphere->center)[0][0], view, projection);
		if (!shaderToUse) return;

		//	override mesh color
		int loc = shaderToUse->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&COLOR_SPHERE);

		//	Draw mesh
		loadVAO(it->second.first->getVAO());
		glDrawElements(GL_TRIANGLES, (int)it->second.first->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else std::cerr << "WARNING : drawSphere not associated or not yet implemented" << std::endl;
}
void Renderer::drawCapsule(const Capsule* capsule, const float* view, const float* projection)
{
	constexpr unsigned int quadrature = 32;										// capsule mesh was generated using this value
	constexpr unsigned int cylinderFaces = 6 * quadrature;						// number of faces on cylinder part
	constexpr unsigned int hemisphereFaces = 6 * quadrature * quadrature / 4;	// idem
	
	std::map<Shape::ShapeType, std::pair<Mesh*, Shader*> >::iterator it = drawShapeDefinition.find(Shape::CAPSULE);
	if (it != drawShapeDefinition.end())
	{
		//	Get shader and prepare matrix
		Shader* shaderToUse = nullptr;
		if (renderOption == WIREFRAME) shaderToUse = defaultShader[INSTANCE_DRAWABLE_WIRED];
		else shaderToUse = it->second.second;

		glm::vec3 center = 0.5f * (capsule->p1 + capsule->p2);
		glm::mat4 base = glm::translate(glm::mat4(1.f), center);
		glm::vec3 v = glm::cross(glm::vec3(0, 0, 1), capsule->p1 - capsule->p2);
		if (v != glm::vec3(0.f))
			base = base * glm::rotate(glm::angle(glm::vec3(0, 0, 1), glm::normalize(capsule->p1 - capsule->p2)), glm::normalize(v));
		else
			base = base * glm::rotate(glm::angle(glm::vec3(0, 0, 1), glm::normalize(capsule->p1 - capsule->p2)), glm::vec3(1, 0, 0));

		loadMVPMatrix(shaderToUse, &base[0][0], view, projection);
		if (!shaderToUse) return;
		
		//	override mesh color
		int loc = shaderToUse->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&COLOR_CAPSULE);
		
		//	draw mesh
		float l = 0.5f * glm::length(capsule->p1 - capsule->p2);
		loadVAO(it->second.first->getVAO());

		glm::mat4 model = glm::scale(base, glm::vec3(capsule->radius, capsule->radius, l));
		loadMVPMatrix(shaderToUse, &model[0][0], view, projection);
		glDrawElements(GL_TRIANGLES, cylinderFaces, GL_UNSIGNED_SHORT, NULL);
		
		model = glm::translate(base, glm::vec3(0, 0, l));
		model = glm::scale(model, glm::vec3(capsule->radius, capsule->radius, capsule->radius));
		loadMVPMatrix(shaderToUse, &model[0][0], view, projection);
		glDrawElements(GL_TRIANGLES, hemisphereFaces, GL_UNSIGNED_SHORT, (void*)(cylinderFaces * sizeof(unsigned short)));

		model = glm::translate(base, glm::vec3(0, 0, -l));
		model = glm::scale(model, glm::vec3(capsule->radius, capsule->radius, capsule->radius));
		loadMVPMatrix(shaderToUse, &model[0][0], view, projection);
		glDrawElements(GL_TRIANGLES, (int)it->second.first->getFaces()->size(), GL_UNSIGNED_SHORT, (void*)((hemisphereFaces + cylinderFaces) * sizeof(unsigned short)));
		
		//	end
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else std::cerr << "WARNING : drawCapsule not associated or not yet implemented" << std::endl;
}
//





