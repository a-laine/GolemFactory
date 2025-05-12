#include "Renderer.h"
#include "CameraComponent.h"

#include <Renderer/DrawableComponent.h>
#include <Renderer/Lighting/LightComponent.h>
#include <Renderer/OccluderComponent.h>
#include <Utiles/Debug.h>
#include <Animation/SkeletonComponent.h>
#include <Resources/Material.h>
#include <Resources/Shader.h>
#include <Terrain/TerrainAreaDrawableComponent.h>
#include <Physics/Collision.h>


#define MAX_INSTANCE 32



void Renderer::CollectEntitiesBindLights()
{
	SCOPED_CPU_MARKER("Collect entities");

	vec4f camPos, camFwd, camUp, camRight;
	camera->getFrustrum(camPos, camFwd, camRight, camUp);
	sceneQuery.getResult().clear();
	collector.getResult().clear();

	sceneQuery.Set(camPos, camFwd, camUp, -camRight, camera->getVerticalFieldOfView(), context->getViewportRatio(), m_frustrumFar);
	sceneQuery.maxDepth = m_queryMaxDepth;
	collector.m_flags = (uint64_t)Entity::Flags::Fl_Drawable | (uint64_t)Entity::Flags::Fl_Light;
	if (m_enableOcclusionCulling)
		collector.m_flags |= (uint64_t)Entity::Flags::Fl_Occluder;

	collector.m_exclusionFlags = (uint64_t)Entity::Flags::Fl_Hide;
	world->getSceneManager().getEntities(&sceneQuery, &collector);

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
			if (ok)
				ok &= comp->visible();
#endif

			if (ok)
			{
				vec4f v = object->getWorldPosition() - camPos;
				float d2 = vec4f::dot(v, v);
				if (d2 > m_frustrumFar * m_frustrumFar)
					continue;

				uint32_t d = (uint32_t)(100.f * d2);

				AxisAlignedBox aabb = object->getBoundingBox();
				if (sceneQuery.TestAABB(aabb.min, aabb.max))
					comp->pushDraw(renderQueue, d, false);

				if (comp->castShadow())// && Collision::collide(&shadowAreaBoxes[3], &aabb, nullptr))
				{
					m_hasShadowCaster = true;
					comp->pushDraw(shadowQueue, d, true);
				}
			}
		}

		float distance2 = (object->getWorldPosition() - m_globalMatrices.cameraPosition).getNorm2();
		if (object->getFlags() & (uint64_t)Entity::Flags::Fl_Light)
		{
			LightComponent* comp = object->getComponent<LightComponent>();
			bool ok = comp;

#ifdef USE_IMGUI
			if (ok && m_lightFrustrumCulling)
				ok = sceneQuery.TestSphere(object->getWorldPosition(), comp->getRange());
#else
			if (ok)
				ok = sceneQuery.TestSphere(object->getWorldPosition(), comp->getRange());
#endif

			if (ok)
			{
				float distance = std::sqrt(distance2);
				float power = 2 * comp->m_range / std::max(distance, 0.001f) * comp->m_intensity;
				tmpLights.push_back({ power , comp });
			}
		}  

		if (object->getFlags() & (uint64_t)Entity::Flags::Fl_Occluder)
		{
			OccluderComponent* comp = object->getComponent<OccluderComponent>();
			bool ok = comp && comp->isValid();
			
			if (ok)
			{
				m_occluders.push_back({ distance2, comp });
			}
		}
	}

	// light sorting
	std::sort(tmpLights.begin(), tmpLights.end(), [](std::pair<float, LightComponent*>& a, std::pair<float, LightComponent*>& b) { return a.first > b.first; });

	m_sceneLights.m_lightCount = 0;
	for (int i = 0; i < MAX_OMNILIGHT_SHADOW_COUNT; i++)
		m_OmniShadows.m_omniShadowLightIndexes[i] = 0xFF;
	shadowOmniCaster.clear();
	shadowOmniLayerUniform = -1;

	for (int i = 0; i < tmpLights.size() && i < MAX_LIGHT_COUNT - 1; i++)
	{
		LightComponent* comp = tmpLights[i].second;
		m_sceneLights.m_lights[i].m_color = comp->m_color;
		m_sceneLights.m_lights[i].m_position = comp->getPosition();
		m_sceneLights.m_lights[i].m_direction = comp->isPointLight() ? vec4f(0.f) : comp->getDirection();
		m_sceneLights.m_lights[i].m_range = comp->m_range;
		m_sceneLights.m_lights[i].m_intensity = comp->m_intensity;
		m_sceneLights.m_lights[i].m_inCutOff = cos((float)DEG2RAD * comp->m_innerCutoffAngle);
		m_sceneLights.m_lights[i].m_outCutOff = cos((float)DEG2RAD * comp->m_outerCutoffAngle);
		
		if (comp->castShadow() && shadowOmniCaster.size() < MAX_OMNILIGHT_SHADOW_COUNT)
		{
			int omniIndex = shadowOmniCaster.size();
			m_OmniShadows.m_omniShadowLightIndexes[omniIndex] = i;
			computeOmniShadowProjection(comp, omniIndex);
			shadowOmniCaster.push_back(comp);
		}

		m_sceneLights.m_lightCount++;
	}

	// bind lights and omni proj
	glBindBuffer(GL_UNIFORM_BUFFER, m_lightsID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, m_sceneLights.m_lightCount * sizeof(Light) + 32, &m_sceneLights);

	glBindBuffer(GL_UNIFORM_BUFFER, m_omniShadowsID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, shadowOmniCaster.size() * 6 * sizeof(mat4f) + MAX_OMNILIGHT_SHADOW_COUNT * 8, &m_OmniShadows);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::CollectTerrainQueueData()
{

}

void Renderer::LightClustering()
{
	SCOPED_CPU_MARKER("Light clustering");

	m_lightClustering->enable();

	GLint lightClusterLocation = glGetUniformLocation(m_lightClustering->getProgram(), "lightClusters");
	if (lightClusterLocation >= 0)
	{
		glUniform1i(lightClusterLocation, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindImageTexture(0, m_lightClusterTexture.getTextureId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32UI);
	}

	glDispatchCompute(16, 12, 16);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	lastShader = nullptr;
	glUseProgram(0);
}

void Renderer::AtmosphericScattering()
{
	m_atmosphericScattering->enable();

	GLint lightClusterLocation = glGetUniformLocation(m_atmosphericScattering->getProgram(), "skybox");
	if (lightClusterLocation >= 0)
	{
		glUniform1i(lightClusterLocation, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindImageTexture(0, m_skyboxTexture->getTextureId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	}

	glDispatchCompute((int)m_skyboxTexture->size.x, (int)m_skyboxTexture->size.y, 6);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	lastShader = nullptr;
	glUseProgram(0);
}

void Renderer::OcclusionCulling()
{
	SCOPED_CPU_MARKER("Occlusion culling");

	double startTime = glfwGetTime();
	occluderTriangles = occluderRasterizedTriangles = occluderPixelsTest = 0;

	float depthMax = -m_sceneLights.m_far;
	int size = m_occlusionBufferSize.x * m_occlusionBufferSize.y;
	for (int i = 0; i < size; ++i)
		m_occlusionDepth[i] = depthMax;

	std::sort(m_occluders.begin(), m_occluders.end(), [](std::pair<float, OccluderComponent*>& a, std::pair<float, OccluderComponent*>& b)
		{ return a.first < b.first; });

	mat4f VP = m_globalMatrices.projection * m_globalMatrices.view;
	vec2i iclamp = vec2i(m_occlusionBufferSize.x - 1, m_occlusionBufferSize.y - 1);

	const auto& clipEdge = [](const vec4f& a, const vec4f& b, float va, float vb)
	{
		float t = va / (va - vb);
		return (1.f - t) * a + t * b;
	};
	const auto& perspectiveDivide = [](vec4f v)
	{
		v.z = -v.w;
		v.w = std::abs(v.w) < 0.0001f ? 1.f : 1.f / v.w;
		v.x = v.x * v.w * 0.5f + 0.5f;
		v.y = -v.y * v.w * 0.5f + 0.5f;
		return v;
	};
	const auto& occlusionTest = [&](const mat4f& MVP, const std::vector<vec4f>& vertices)
	{
		vec4f min = vec4f(FLT_MAX);
		vec4f max = -min;
		for (const vec4f& v : vertices)
		{
			vec4f p = perspectiveDivide(MVP * v);
			min = vec4f::min(min, p);
			max = vec4f::max(max, p);
		}

		// obj is behind
		if (min.z > 0.f)
			return true;

		// as pixel index
		vec2i imin = vec2i(min.x * m_occlusionBufferSize.x, min.y * m_occlusionBufferSize.y);
		vec2i imax = vec2i(max.x * m_occlusionBufferSize.x, max.y * m_occlusionBufferSize.y);

		// out of screen
		if (imin.x >= m_occlusionBufferSize.x || imin.y >= m_occlusionBufferSize.y || imax.x < 0 || imax.y < 0)
			return true;

		// clamp on screen
		imin.x = imin.x < 0 ? 0 : (imin.x > iclamp.x ? iclamp.x : imin.x);
		imin.y = imin.y < 0 ? 0 : (imin.y > iclamp.y ? iclamp.y : imin.y);
		imax.x = imax.x < 0 ? 0 : (imax.x > iclamp.x ? iclamp.x : imax.x);
		imax.y = imax.y < 0 ? 0 : (imax.y > iclamp.y ? iclamp.y : imax.y);

		// test occlusion
		for (int i = imin.x; i <= imax.x; i++)
			for (int j = imin.y; j <= imax.y; j++)
			{
				int id = j * m_occlusionBufferSize.x + i;
				if (max.z > m_occlusionDepth[id])
					return false;
			}
		return true;
	};

	for (auto& it : m_occluders)
	{
		const OccluderComponent* ocluderComp = it.second;
		Mesh* mesh = ocluderComp->getMesh();
		mat4f MVP = VP * ocluderComp->getParentEntity()->getWorldTransformMatrix();
		if (occlusionTest(MVP, *mesh->getBBoxVertices()))
			continue;

		bool doBackfaceCulling = ocluderComp->backFaceCulling();
		vec4f scale = ocluderComp->getParentEntity()->getWorldScale();
		bool flipTest = scale.x * scale.y * scale.z < 0.f;

		vec4f v, v01, v02, v12;
		occluderScreenVertices.clear();
		unsigned int indiceCount = mesh->getNumberIndices();
		const std::vector<vec4f>& vertices = *mesh->getVertices();
		for (unsigned int i = 0; i < indiceCount; i += 3)
		{
			vec4f v0 = MVP * vertices[mesh->getFaceIndiceAt(i)];
			vec4f v1 = MVP * vertices[mesh->getFaceIndiceAt(i + 1)];
			vec4f v2 = MVP * vertices[mesh->getFaceIndiceAt(i + 2)];

			float dot0 = v0.w;
			float dot1 = v1.w;
			float dot2 = v2.w;
			std::uint8_t mask = (dot0 < 0.f ? 1 : 0) | (dot1 < 0.f ? 2 : 0) | (dot2 < 0.f ? 4 : 0);

			switch (mask)
			{
				case 0b000:
					occluderScreenVertices.push_back(v0);
					occluderScreenVertices.push_back(v1);
					occluderScreenVertices.push_back(v2);
					break;

				case 0b001:
					v01 = clipEdge(v0, v1, dot0, dot1);
					v02 = clipEdge(v0, v2, dot0, dot2);
					occluderScreenVertices.push_back(v01);
					occluderScreenVertices.push_back(v1);
					occluderScreenVertices.push_back(v2);
					occluderScreenVertices.push_back(v01);
					occluderScreenVertices.push_back(v2);
					occluderScreenVertices.push_back(v02);
					break;

				case 0b010:
					v01 = clipEdge(v1, v0, dot1, dot0);
					v12 = clipEdge(v1, v2, dot1, dot2);
					occluderScreenVertices.push_back(v0);
					occluderScreenVertices.push_back(v01);
					occluderScreenVertices.push_back(v2);
					occluderScreenVertices.push_back(v2);
					occluderScreenVertices.push_back(v01);
					occluderScreenVertices.push_back(v12);
					break;

				case 0b011:
					v02 = clipEdge(v0, v2, dot0, dot2);
					v12 = clipEdge(v2, v1, dot2, dot1);
					occluderScreenVertices.push_back(v02);
					occluderScreenVertices.push_back(v12);
					occluderScreenVertices.push_back(v2);
					break;

				case 0b100:
					v02 = clipEdge(v2, v0, dot2, dot0);
					v12 = clipEdge(v2, v1, dot2, dot1);
					occluderScreenVertices.push_back(v0);
					occluderScreenVertices.push_back(v1);
					occluderScreenVertices.push_back(v02);
					occluderScreenVertices.push_back(v02);
					occluderScreenVertices.push_back(v1);
					occluderScreenVertices.push_back(v12);
					break;

				case 0b101:
					v01 = clipEdge(v0, v1, dot0, dot1);
					v12 = clipEdge(v2, v1, dot2, dot1);
					occluderScreenVertices.push_back(v01);
					occluderScreenVertices.push_back(v1);
					occluderScreenVertices.push_back(v12);
					break;

				case 0b110:
					v01 = clipEdge(v0, v1, dot0, dot1);
					v02 = clipEdge(v0, v2, dot0, dot2);
					occluderScreenVertices.push_back(v0);
					occluderScreenVertices.push_back(v01);
					occluderScreenVertices.push_back(v02);
					break;

				default:
					break;
			}
		}

		vec2f min, max;
		vec2i imin = vec2i::zero;
		vec2i imax = vec2i::zero;
		for (unsigned int f = 0; f < occluderScreenVertices.size(); f += 3)
		{
			occluderTriangles++;
			vec4f p1 = perspectiveDivide(occluderScreenVertices[f]);
			vec4f p2 = perspectiveDivide(occluderScreenVertices[f + 1]);
			vec4f p3 = perspectiveDivide(occluderScreenVertices[f + 2]);
			depthMax = p1.z > p2.z ? p1.z : p2.z; depthMax = depthMax > p3.z ? depthMax : p3.z;

			// back face culling
			vec4f p1p2 = p2 - p1;
			vec4f p1p3 = p3 - p1;
			if (doBackfaceCulling)
			{
				if (flipTest)
				{
					if (p1p2.x * p1p3.y - p1p3.x * p1p2.y > 0.f)
						continue;
				}
				else
				{
					if (p1p2.x * p1p3.y - p1p3.x * p1p2.y < 0.f)
						continue;
				}
			}

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
					if (depth > m_occlusionDepth[id])
					{
						// write pixel data
						m_occlusionDepth[id] = depth;
					}
				}
		}
	}

#ifdef USE_IMGUI
	occlusionTexture.update(m_occlusionDepth, GL_RED, GL_FLOAT);
#endif

	for (auto& it : renderQueue)
	{
		//continue;
		DrawableComponent* comp = it.entity->getComponent<DrawableComponent>();
		Mesh* mesh = comp->getMesh();
		if (mesh->hasSkeleton())
			continue;

		mat4f MVP = VP * it.entity->getWorldTransformMatrix();
		const std::vector<vec4f>& vertices = *mesh->getBBoxVertices();
		if (occlusionTest(MVP, vertices))
		{
			it.entity = nullptr;
			occlusionCulledInstances++;
		}
	}

	m_OcclusionElapsedTime = 1000.0f * (float)(glfwGetTime() - startTime);
	m_OcclusionAvgTime = 0.95f * m_OcclusionAvgTime + 0.05f * m_OcclusionElapsedTime;
}

void Renderer::CreateBatches(std::vector<DrawElement>& _queue, int _shadowMode)
{
	SCOPED_CPU_MARKER("Dynamic batching");

	// clear batches containers
	for (auto it : batchClosedPool)
	{
		it->matrices.clear();
		batchFreePool.push_back(it);
	}
	batchClosedPool.clear();
	for (auto it : batchOpened)
	{
		it.second->matrices.clear();
		batchFreePool.push_back(it.second);
	}
	batchOpened.clear();

	// second pass of renderqueue
	int variantCode = Shader::computeVariantCode(true, _shadowMode, renderOption == RenderOption::WIREFRAME);

	for (auto& it : _queue)
	{
		if (!it.entity)
			continue;

		DrawableComponent* comp = it.entity->getComponent<DrawableComponent>();
		Material* material = it.material;
		Shader* shader = material->getShader();
		if (!shader->supportInstancing())
			continue;

		shader = shader->getVariant(variantCode);
		Mesh* mesh = it.mesh;

		// search insertion batch
		Batch* batch;
		bool isNewBatch = false;
		BatchKey key = BatchKey(mesh, shader, material, comp->isClockWise());
		auto it2 = batchOpened.find(key);
		if (it2 == batchOpened.end())
		{
			if (batchFreePool.empty())
			{
				batch = new Batch();
				batch->instanceDatas = new vec4f[m_maxUniformSize];
			}
			else
			{
				batch = batchFreePool.back();
				batchFreePool.pop_back();
			}

			isNewBatch = true;
			batch->shader = shader;
			batch->material = material;
			batch->mesh = mesh;
			batch->instanceCount = 0;
			batch->clockwise = comp->isClockWise();
			batch->dataSize = comp->getInstanceDataSize();
			batch->pushMatrices = shader->getUniformLocation("matrixArray") >= 0;
			batch->constantDataReference = comp->hasConstantData() ? comp : nullptr;

			if (batch->dataSize)
			{
				batch->maxInstanceCount = m_maxUniformSize / batch->dataSize;
				if (batch->pushMatrices)
					batch->maxInstanceCount = std::min((int)batch->maxInstanceCount, MAX_INSTANCE);
			}
			else batch->maxInstanceCount = MAX_INSTANCE;

			batchOpened[key] = batch;
		}
		else
			batch = it2->second;

		// insert object
		comp->writeInstanceData(batch->instanceDatas + (uint64_t)batch->dataSize * batch->instanceCount);
		if (batch->pushMatrices)
		{
			batch->matrices.push_back(it.entity->getWorldTransformMatrix());
			batch->matrices.push_back(it.entity->getNormalMatrix());
		}
		batch->instanceCount++;
		it.batch = batch;

		if (!isNewBatch)
		{
			it.entity = nullptr;
			if (batch->instanceCount >= batch->maxInstanceCount)
			{
				batchClosedPool.push_back(batch);
				batchOpened.erase(it2);
			}
		}
	}
}

void Renderer::ShadowCasting()
{
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);// peter panning
	bool ccw = true;
	auto PeterPanningSwitch = [&ccw](bool _ccw)
	{
		if (_ccw && !ccw)
			glFrontFace(GL_CCW);
		else if (!_ccw && ccw)
			glFrontFace(GL_CW);
		ccw = _ccw;
	};

	{
		SCOPED_CPU_MARKER("Shadow cascades");

		std::sort(shadowQueue.begin(), shadowQueue.end(), [](DrawElement& a, DrawElement& b) { return a.hash < b.hash; });
		int shadowCode = Shader::computeVariantCode(false, 1, false);
		Sphere shadowSphere = Sphere(shadowAreaBoxes[3].base[3], shadowAreaBoxes[3].max.x);

		// draw shadows
		glViewport(0, 0, shadowCascadeTexture.size.x, shadowCascadeTexture.size.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowCascadeFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		// batching cascade
		if (m_enableInstancing)
			CreateBatches(shadowQueue, 1);

		//	draw instance list
		for (const auto& it : shadowQueue)
		{
			// skip batched entities
			if (!it.entity)
				continue;

			DrawableComponent* drawableComp = it.entity->getComponent<DrawableComponent>();
			shadowCascadeMax = it.material->getMaxShadowCascade();

			if (it.batch)
			{
				bool peterWindingOrder = it.batch->shader->usePeterPanning();

				if ((it.hash & CullingModeMask) == 0)
					peterWindingOrder = !peterWindingOrder;
				PeterPanningSwitch(peterWindingOrder);

				drawInstancedObject(it.batch->material, it.batch->shader, it.batch->mesh, (float*)it.batch->matrices.data(), it.batch->instanceDatas,
					it.batch->dataSize, it.batch->instanceCount, it.batch->constantDataReference);
				shadowDrawCalls++;
			}
			else
			{
				const AxisAlignedBox aabb = it.entity->getBoundingBox();
				if (!Collision::collide(&shadowSphere, &aabb, nullptr))
					continue;

				bool peterWindingOrder = it.material->getShader()->usePeterPanning();
				if ((it.hash & CullingModeMask) == 0)
					peterWindingOrder = !peterWindingOrder;
				PeterPanningSwitch(peterWindingOrder);

				drawObject(it.entity, it.material->getShader()->getVariant(shadowCode));
				shadowDrawCalls++;
			}
		}
	}


	// omni shadows
	shadowCascadeMax = -1;
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);// peter panning
	ccw = true;
	if (!shadowOmniCaster.empty())
	{
		SCOPED_CPU_MARKER("Omnidirectional shadows");

		int shadowCode = Shader::computeVariantCode(false, 2, false);
		unsigned int omniShadowPassMask = 1 << eOmniShadowPass;
		m_sceneLights.m_shadingConfiguration |= omniShadowPassMask;
		glBindBuffer(GL_UNIFORM_BUFFER, m_lightsID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 8, &m_sceneLights);

		glViewport(0, 0, shadowOmniTextures.size.x, shadowOmniTextures.size.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowOmniFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < shadowOmniCaster.size(); i++)
		{
			shadowOmniLayerUniform = i;
			lastShader = nullptr; // in order to force a new baseLayerUniform

			vec4f center = shadowOmniCaster[i]->getPosition();
			vec4f hsize = vec4f(shadowOmniCaster[i]->getRange());
			hsize.w = 0;

			omniLightQuery.getResult().clear();
			omniLightQuery.Set(center - hsize, center + hsize);
			omniLightCollector.getResult().clear();
			world->getSceneManager().getEntities(&omniLightQuery, &omniLightCollector);

			// queue creation
			shadowQueue.clear();
			for (Entity* object : collector.getResult())
			{
				if (!omniLightQuery.TestAABB(object->getBoundingBox().min, object->getBoundingBox().max))
					continue;

				if (object->getFlags() & (uint64_t)Entity::Flags::Fl_Drawable)
				{
					DrawableComponent* drawableComp = object->getComponent<DrawableComponent>();
					if (drawableComp && drawableComp->isValid() && drawableComp->castShadow())
					{
						vec4f v = object->getWorldPosition() - center;
						uint32_t d = (uint32_t)(100.f * vec4f::dot(v, v));
						drawableComp->pushDraw(shadowQueue, d, true);
					}
				}
			}

			if (m_enableInstancing)
				CreateBatches(shadowQueue, 2);

			// draw
			for (const auto& it : shadowQueue)
			{
				// skip batched entities
				if (!it.entity)
					continue;

				DrawableComponent* drawableComp = it.entity->getComponent<DrawableComponent>();
				shadowCascadeMax = it.material->getMaxShadowCascade();

				if (it.batch)
				{
					bool peterWindingOrder = it.batch->shader->usePeterPanning();
					if ((it.hash & CullingModeMask) == 0)
						peterWindingOrder = !peterWindingOrder;
					PeterPanningSwitch(peterWindingOrder);

					drawInstancedObject(it.batch->material, it.batch->shader, it.batch->mesh, (float*)it.batch->matrices.data(), it.batch->instanceDatas,
						it.batch->dataSize, it.batch->instanceCount, it.batch->constantDataReference);
					shadowDrawCalls++;
				}
				else
				{
					bool peterWindingOrder = it.material->getShader()->usePeterPanning();
					if ((it.hash & CullingModeMask) == 0)
						peterWindingOrder = !peterWindingOrder;
					PeterPanningSwitch(peterWindingOrder);

					drawObject(it.entity, it.material->getShader()->getVariant(shadowCode));
					shadowDrawCalls++;
				}
			}
		}

		shadowOmniLayerUniform = -1;
		m_sceneLights.m_shadingConfiguration &= ~omniShadowPassMask;
		glBindBuffer(GL_UNIFORM_BUFFER, m_lightsID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 8, &m_sceneLights);
	}

	glFrontFace(GL_CW);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
