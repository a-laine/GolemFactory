#include "Renderer.h"
#include "CameraComponent.h"

#include <Renderer/DrawableComponent.h>
#include <Renderer/Lighting/LightComponent.h>
#include <Renderer/OccluderComponent.h>
#include <Utiles/Debug.h>


#define MAX_INSTANCE 32



void Renderer::CollectEntitiesBindLights()
{
	vec4f camPos, camFwd, camUp, camRight;
	camera->getFrustrum(camPos, camFwd, camRight, camUp);
	sceneQuery.getResult().clear();
	collector.getResult().clear();

	sceneQuery.Set(camPos, camFwd, camUp, -camRight, camera->getVerticalFieldOfView(), context->getViewportRatio(), m_sceneLights.m_far);
	collector.m_flags = (uint64_t)Entity::Flags::Fl_Drawable | (uint64_t)Entity::Flags::Fl_Light;
	if (m_enableOcclusionCulling)
		collector.m_flags |= (uint64_t)Entity::Flags::Fl_Occluder;

	collector.m_exclusionFlags = (uint64_t)Entity::Flags::Fl_Hide;
	world->getSceneManager().getEntities(&sceneQuery, &collector);

	uint64_t transparentMask = 1ULL << 63;
	uint64_t faceCullingMask = 1ULL << 62;
	m_hasInstancingShaders = false;
	m_hasShadowCaster = false;
	renderQueue.clear();
	shadowQueue.clear();
	m_occluders.clear();
	std::vector<std::pair<float, LightComponent*>> tmpLights;

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
				m_hasInstancingShaders |= comp->getShader()->supportInstancing();
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
				renderQueue.push_back({ hash, object, nullptr });

				if ((queue & transparentMask) == 0 && comp->castShadow())
				{
					m_hasShadowCaster = true;
					float projected = vec4f::dot(object->getWorldPosition(), camFwd);
					shadowQueue.push_back({ projected, object, nullptr });
				}
			}
		}

		if ((object->getFlags() & (uint64_t)Entity::Flags::Fl_Light))
		{
			LightComponent* comp = object->getComponent<LightComponent>();
			bool ok = comp && m_sceneLights.m_lightCount < MAX_LIGHT_COUNT;

#ifdef USE_IMGUI
			if (ok && m_lightFrustrumCulling)
				ok = sceneQuery.TestSphere(object->getWorldPosition(), comp->getRange());
#else
			if (ok)
				ok = sceneQuery.TestSphere(object->getWorldPosition(), comp->getRange());
#endif

			if (ok)
			{
				float distance = (comp->getPosition() - m_globalMatrices.cameraPosition).getNorm();
				float power = 2 * comp->m_range / std::max(distance, 0.001f) * comp->m_intensity;
				tmpLights.push_back({ power , comp });
			}
		}

		if ((object->getFlags() & (uint64_t)Entity::Flags::Fl_Occluder))
		{
			OccluderComponent* comp = object->getComponent<OccluderComponent>();
			bool ok = comp && comp->isValid();
			
			if (ok)
			{
				float distance2 = (object->getWorldPosition() - m_globalMatrices.cameraPosition).getNorm2();
				m_occluders.push_back({ distance2, comp });
			}
		}
	}

	// light sorting
	std::sort(tmpLights.begin(), tmpLights.end(), [](std::pair<float, LightComponent*>& a, std::pair<float, LightComponent*>& b) { return a.first > b.first; });
	m_sceneLights.m_lightCount = 0;
	for (int i = 0; i < tmpLights.size() && i < 128; i++)
	{
		auto comp = tmpLights[i].second;
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

	// bind lights
	glBindBuffer(GL_UNIFORM_BUFFER, m_lightsID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, m_sceneLights.m_lightCount * sizeof(Light) + 32, &m_sceneLights);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::LightClustering()
{
	lightClustering->enable();

	GLint lightClusterLocation = glGetUniformLocation(lightClustering->getProgram(), "lightClusters");
	if (lightClusterLocation >= 0)
	{
		glUniform1i(lightClusterLocation, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindImageTexture(0, lightClusterTexture.getTextureId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32UI);
	}

	glDispatchCompute(16, 12, 16);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	lastShader = nullptr;
	glUseProgram(0);
}

void Renderer::OcclusionCulling()
{
	double startTime = glfwGetTime();
	occluderTriangles = occluderRasterizedTriangles = occluderPixelsTest = 0;

	float depthMax = -m_sceneLights.m_far;
	int size = m_occlusionBufferSize.x * m_occlusionBufferSize.y;
	for (int i = 0; i < size; ++i)
	{
		m_occlusionDepth[i] = depthMax;
		m_occlusionDepthColor[i] = 0;
	}

	std::sort(m_occluders.begin(), m_occluders.end(), [](std::pair<float, OccluderComponent*>& a, std::pair<float, OccluderComponent*>& b)
		{ return a.first < b.first; });

	mat4f VP = m_globalMatrices.projection * m_globalMatrices.view;
	vec4f screenMin = vec4f(0, 0, 0, 0);
	vec4f screenMax = vec4f(1, 1, depthMax, 0);
	vec2i iclamp = vec2i(m_occlusionBufferSize.x - 1, m_occlusionBufferSize.y - 1);

	for (int m = 0; m < m_occluders.size(); m++)
	{
		Mesh* mesh = m_occluders[m].second->getMesh();
		mat4f MVP = VP * m_occluders[m].second->getParentEntity()->getWorldTransformMatrix();
		bool doBackfaceCulling = m_occluders[m].second->backFaceCulling();

		vec4f v;
		occluderScreenVertices.clear();
		const std::vector<vec4f>& vertices = *mesh->getVertices();
		for (int j = 0; j < vertices.size(); j++)
		{
			v = MVP * vertices[j];
			float w = -v.w;
			v.x = -v.x / w * 0.5f + 0.5f;
			v.y = v.y / w * 0.5f + 0.5f;
			v.z = w ;
			v.w = 1.f;
			occluderScreenVertices.push_back(v);
		}

		vec2f min, max;
		vec2i imin = vec2i::zero;
		vec2i imax = vec2i::zero;
		const std::vector<unsigned short>& faces = *mesh->getFaces();
		for (int f = 0; f < faces.size(); f += 3)
		{
			occluderTriangles++;

			vec4f p1 = occluderScreenVertices[faces[f]];
			vec4f p2 = occluderScreenVertices[faces[f + 1]];
			vec4f p3 = occluderScreenVertices[faces[f + 2]];

			float depthMin = std::min(p1.z, std::min(p2.z, p3.z));
			float depthMax = std::max(p1.z, std::max(p2.z, p3.z));

			if (depthMax > 0.f)
				continue;

			// back face culling
			vec4f p1p2 = p2 - p1;
			vec4f p1p3 = p3 - p1;
			if (doBackfaceCulling && p1p2.x * p1p3.y - p1p3.x * p1p2.y < 0.f)
				continue;

			// triangle min and max patch coordinates
			min.x = p1.x < p2.x ? p1.x : p2.x; min.x = min.x < p3.x ? min.x : p3.x;
			min.y = p1.y < p2.y ? p1.y : p2.y; min.y = min.y < p3.y ? min.y : p3.y;
			max.x = p1.x > p2.x ? p1.x : p2.x; max.x = max.x > p3.x ? max.x : p3.x;
			max.y = p1.y > p2.y ? p1.y : p2.y; max.y = max.y > p3.y ? max.y : p3.y;

			// as pixel index
			imin.x = (int)(min.x * m_occlusionBufferSize.x);
			imin.y = (int)(min.y * m_occlusionBufferSize.y);
			imax.x = (int)(max.x * m_occlusionBufferSize.x);
			imax.y = (int)(max.y * m_occlusionBufferSize.y);

			// out of screen
			if (imin.x >= m_occlusionBufferSize.x || imin.y >= m_occlusionBufferSize.y || imax.x < 0 || imax.y < 0) 
				continue;

			// clamp on screen
			imin.x = imin.x < 0 ? 0 : (imin.x > iclamp.x ? iclamp.x : imin.x);
			imin.y = imin.y < 0 ? 0 : (imin.y > iclamp.y ? iclamp.y : imin.y);
			imax.x = imax.x < 0 ? 0 : (imax.x > iclamp.x ? iclamp.x : imax.x);
			imax.y = imax.y < 0 ? 0 : (imax.y > iclamp.y ? iclamp.y : imax.y);

			// occlusion culling
			bool occluded = true;
			for (int i = imin.x; i <= imax.x && occluded; i++)
				for (int j = imin.y; j <= imax.y && occluded; j++)
				{
					int id = j * m_occlusionBufferSize.x + i;
					if (depthMax > m_occlusionDepth[id])
					{
						occluded = false;
						break;
					}
				}

			if (occluded)  
				continue;

			// barycentric params
			float d00 = p1p2.x * p1p2.x + p1p2.y * p1p2.y;
			float d01 = p1p2.x * p1p3.x + p1p2.y * p1p3.y;
			float d11 = p1p3.x * p1p3.x + p1p3.y * p1p3.y;
			float invdenom = 1.f / (d00 * d11 - d01 * d01);
			occluderRasterizedTriangles++;

			for (int i = imin.x; i <= imax.x; i++)
				for (int j = imin.y; j <= imax.y; j++)
				{
					int id = j * m_occlusionBufferSize.x + i;

					// compute barycentric coordinates
					float u, v, w;
					{
						float d20 = (m_occlusionCenterX[id] - p1.x) * p1p2.x + (m_occlusionCenterY[id] - p1.y) * p1p2.y;
						float d21 = (m_occlusionCenterX[id] - p1.x) * p1p3.x + (m_occlusionCenterY[id] - p1.y) * p1p3.y;
						v = (d11 * d20 - d01 * d21) * invdenom;
						w = (d00 * d21 - d01 * d20) * invdenom;
						u = 1.0f - v - w;
					}

					// out of triangle
					if (u < 0.f || v < 0.f || w < 0.f || u > 1.f || v > 1.f)
						continue;

					// pixel interpolated depth
					float depth =  p1.z * u + p2.z * v + p3.z * w;
					occluderPixelsTest++;
					float pxDepth = m_occlusionDepth[id];
					if (depth > m_occlusionDepth[id] && depth < -0.1f)
					{
						// write pixel data
						m_occlusionDepth[id] = depth;
						m_occlusionDepthColor[id] = -depth;
					}
				}
		}
	}

	occlusionTexture.update(m_occlusionDepthColor, GL_RGBA, GL_UNSIGNED_BYTE);

	for (auto& it : renderQueue)
	{
		DrawableComponent* comp = it.entity->getComponent<DrawableComponent>();
		Mesh* mesh = comp->getMesh();
		mat4f MVP = VP * it.entity->getWorldTransformMatrix();

		bool visible = false;
		vec4f v;
		occluderScreenVertices.clear();
		const std::vector<vec4f>& vertices = *mesh->getBBoxVertices();
		for (int j = 0; j < vertices.size(); j++)
		{
			v = MVP * vertices[j];
			float w = -v.w;
			v.x = -v.x / w * 0.5f + 0.5f;
			v.y = v.y / w * 0.5f + 0.5f;
			v.z = w;
			v.w = 1.f;
			occluderScreenVertices.push_back(v);
		}

		vec2f min, max;
		vec2i imin = vec2i::zero;
		vec2i imax = vec2i::zero;
		const std::vector<unsigned short>& faces = *mesh->getBBoxFaces();
		for (int f = 0; f < faces.size(); f += 3)
		{
			vec4f p1 = occluderScreenVertices[faces[f]];
			vec4f p2 = occluderScreenVertices[faces[f + 1]];
			vec4f p3 = occluderScreenVertices[faces[f + 2]];

			float depthMin = std::min(p1.z, std::min(p2.z, p3.z));
			float depthMax = std::max(p1.z, std::max(p2.z, p3.z));

			if (depthMax > 0.f)
				continue;

			// back face culling
			vec4f p1p2 = p2 - p1;
			vec4f p1p3 = p3 - p1;
			if (p1p2.x * p1p3.y - p1p3.x * p1p2.y < 0.f)
				continue;

			// triangle min and max patch coordinates
			min.x = p1.x < p2.x ? p1.x : p2.x; min.x = min.x < p3.x ? min.x : p3.x;
			min.y = p1.y < p2.y ? p1.y : p2.y; min.y = min.y < p3.y ? min.y : p3.y;
			max.x = p1.x > p2.x ? p1.x : p2.x; max.x = max.x > p3.x ? max.x : p3.x;
			max.y = p1.y > p2.y ? p1.y : p2.y; max.y = max.y > p3.y ? max.y : p3.y;

			// as pixel index
			imin.x = (int)(min.x * m_occlusionBufferSize.x);
			imin.y = (int)(min.y * m_occlusionBufferSize.y);
			imax.x = (int)(max.x * m_occlusionBufferSize.x);
			imax.y = (int)(max.y * m_occlusionBufferSize.y);

			// out of screen
			if (imin.x >= m_occlusionBufferSize.x || imin.y >= m_occlusionBufferSize.y || imax.x < 0 || imax.y < 0) 
				continue;

			// clamp on screen
			imin.x = imin.x < 0 ? 0 : (imin.x > iclamp.x ? iclamp.x : imin.x);
			imin.y = imin.y < 0 ? 0 : (imin.y > iclamp.y ? iclamp.y : imin.y);
			imax.x = imax.x < 0 ? 0 : (imax.x > iclamp.x ? iclamp.x : imax.x);
			imax.y = imax.y < 0 ? 0 : (imax.y > iclamp.y ? iclamp.y : imax.y);

			// occlusion culling
			bool occluded = true;
			for (int i = imin.x; i <= imax.x && occluded; i++)
				for (int j = imin.y; j <= imax.y && occluded; j++)
				{
					int id = j * m_occlusionBufferSize.x + i;
					if (depthMax > m_occlusionDepth[id])
					{
						occluded = false;
						break;
					}
				}

			if (!occluded)
			{
				visible = true;
				break;
			}
		}

		if (!visible)
		{
			it.entity = nullptr;
			occlusionCulledInstances++;
		}
	}


	m_OcclusionElapsedTime = 1000.0f * (float)(glfwGetTime() - startTime);
	m_OcclusionAvgTime = 0.95f * m_OcclusionAvgTime + 0.05f * m_OcclusionElapsedTime;
}

void Renderer::DynamicBatching()
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
		if (!it.entity)
			continue;
		DrawableComponent* comp = it.entity->getComponent<DrawableComponent>();
		if (!comp->getShader()->supportInstancing())
			continue;

		Shader* shader = comp->getShader()->getVariant(variantCode);
		Mesh* mesh = comp->getMesh();

		// search insertion batch
		Batch* batch;
		bool isNewBatch = false;
		std::pair<Shader*, Mesh*> key = { shader, mesh };
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
		batch->models.push_back({ it.entity->getWorldTransformMatrix() , it.entity->getNormalMatrix() });
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

void Renderer::ShadowCasting()
{
	std::sort(shadowQueue.begin(), shadowQueue.end(), [](ShadowDrawElement& a, ShadowDrawElement& b) { return a.distance < b.distance; });
	int shadowCode = Shader::computeVariantCode(false, true, false);

	// batching
	if (m_enableInstancing && m_hasInstancingShaders)
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

		int instancedShadowCode = Shader::computeVariantCode(true, true, false);

		for (auto& it : shadowQueue)
		{
			if (!it.entity)
				continue;
			DrawableComponent* comp = it.entity->getComponent<DrawableComponent>();
			if (!comp->getShader()->supportInstancing())
				continue;

			Shader* shader = comp->getShader()->getVariant(instancedShadowCode);
			Mesh* mesh = comp->getMesh();

			// search insertion batch
			Batch* batch;
			bool isNewBatch = false;
			std::pair<Shader*, Mesh*> key = { shader, mesh };
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
			batch->models.push_back({ it.entity->getWorldTransformMatrix() , it.entity->getNormalMatrix() });
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

	// draw shadows
	glViewport(0, 0, shadowCascadeTexture.size.x, shadowCascadeTexture.size.y);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D_ARRAY, shadowCascadeTexture.getTextureId(), 0);
	//glClearBufferfv()
	glClear(GL_DEPTH_BUFFER_BIT);

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);  // peter panning

	//	draw instance list
	for (const auto& it : shadowQueue)
	{
		// skip batched entities
		if (!it.entity)
			continue;

		if (it.batch)
			drawInstancedObject(it.batch->shader, it.batch->mesh, it.batch->models);
		else
		{
			DrawableComponent* drawableComp = it.entity->getComponent<DrawableComponent>();
			drawObject(it.entity, drawableComp->getShader()->getVariant(shadowCode));
		}
	}

	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
