Terrain
{	
	renderQueue : 1000;//opaque
	usePeterPanning = false;
	
	uniform :
	{
		instanceDataArray : "vec4 array1000";
		constantData : "vec4 array16";
		lightClusters : "_globalLightClusters";
		cascadedShadow : "_globalShadowCascades";
		materialCollection : "_globalTerrainMaterialCollection";
		omniShadowArray : "_globalOmniShadow";
		terrainVirtualTexture : "_terrainVirtualTexture";
		shadowOmniLayerUniform = "int";
	};
	
	/*textures : [
		{
			name : "albedo";
			resource : "PolygonDungeon/Dungeons_Texture_01.png";
		},{
			name : "emmisive";
			resource : "PolygonDungeon/Emmisive_01.png";
		},{
			name : "metalic";
			resource : "PolygonDungeon/Dungeons_Crystal_Metallic.png";
		}
	];*/
	
	includes : 
	{
		#version 430
		struct VertexData
		{
			vec4 normalTerrain;
			float height;
			uint material0;
			uint material1;
			float material0weight;
			bool holeTerrain;
		};
		vec4 octahedralUnpack(uint n, uint bits)
		{
			uint mask = (1 << bits) - 1;
			uvec2 d = uvec2(n, n >> bits) & mask;
			vec2 v = vec2(d) / float(mask);
			v = 1.0 - 2.0 * v;
			vec4 nor = vec4(v.x, 1.0 - abs(v.x) - abs(v.y), v.y, 0.0);
			return normalize(nor);
		}
		
		layout(rgba16ui) readonly uniform uimage2D terrainVirtualTexture;	// image unit 1
		
		VertexData GetVertexData(ivec2 vertexCoord, float heightAmplitude, float seeLevel)
		{
			uvec4 data = imageLoad(terrainVirtualTexture, vertexCoord);
			
			VertexData vdata;
			vdata.height = data.x * heightAmplitude - seeLevel;
			vdata.normalTerrain = octahedralUnpack(data.y, 8);
			vdata.material0 = data.z & 0xFF;
			vdata.material1 = (data.z >> 8) & 0xFF;
			vdata.material0weight = (data.w & 0xFF) / 256.0;
			vdata.holeTerrain = (data.w & (1 << 15)) != 0;
			return vdata;
		}
		bool GetVertexHoleData(ivec2 vertexCoord, float heightAmplitude, float seeLevel)
		{
			uvec4 data = imageLoad(terrainVirtualTexture, vertexCoord);
			return (data.w & (1 << 15)) != 0;
		}
		
		#define ALLOW_DEBUG
	};
	vertex :
	{
		#include "UniformBuffers.cginc"
	
		// input
		layout(location = 0) in vec4 position;
		layout(location = 2) in vec4 uv;
		
		#define INSTANCING
		// following cannot exceed 1024
		uniform vec4 instanceDataArray[1000];
		uniform vec4 constantData[16];
		
		// output
		#ifdef WIRED_MODE
			out vec4 fragmentPosition_gs;
			out vec4 fragmentNormal_gs;
			out vec4 fragmentUv_gs;
			flat out vec4 terrainData0_gs;
		#else
			#ifdef SHADOW_PASS
				// nothing to out more than 'gl_Position'
			#else
				out vec4 fragmentPosition;
				out vec4 fragmentNormal;
				out vec4 fragmentUv;
				flat out vec4 terrainData0;
			#endif
		#endif
		
		float ComputeMorphingRatio(float camDistance)
		{
			int meshLod = int(constantData[0].x);
			float allRadius[8] = float[](constantData[1].x, constantData[1].y, constantData[1].z, constantData[1].w, constantData[2].x, constantData[2].y, constantData[2].z, constantData[2].w);
			float morphRange = constantData[3].x;
			for (int i = 0; i < 8; i++)
			{
				if (camDistance < allRadius[i])
				{
					if (meshLod < i)
						return 1.0;
					else if (camDistance > allRadius[i] - morphRange)
						return (camDistance - (allRadius[i] - morphRange)) / morphRange;
					return 0.0;
				}
			}
			return 1.0;
		}
		//float rand(vec2 co){ return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453); }
			
		// program
		void main()
		{
			vec4 areaData0 = instanceDataArray[2 * gl_InstanceID];
			vec4 vertexCoordFloat = areaData0.zwzw + uv.xyxy;
			vertexCoordFloat.z = 0.0;
			VertexData vertexData0 = GetVertexData(ivec2(vertexCoordFloat.x , vertexCoordFloat.y), constantData[0].z, constantData[0].w);
			vec4 p = vec4(position.x + areaData0.x, vertexData0.height,  position.z + areaData0.y, 1.0);
			vec4 normal = vertexData0.normalTerrain;
			
			if (uv.z != 0.0 || uv.w != 0.0)
			{
				vec4 delta = p - cameraPosition;
				float camDistance = sqrt(delta.x * delta.x + delta.z * delta.z);
				float ratio = ComputeMorphingRatio(camDistance);
				
				if (ratio > 0.0)
				{
					VertexData vertexData1 = GetVertexData(ivec2(vertexCoordFloat.x + uv.z , vertexCoordFloat.y + uv.w), constantData[0].z, constantData[0].w);
					VertexData vertexData2 = GetVertexData(ivec2(vertexCoordFloat.x - uv.z , vertexCoordFloat.y - uv.w), constantData[0].z, constantData[0].w);
					vertexCoordFloat.z = ratio;
					
					p.y = mix(vertexData0.height, 0.5 * (vertexData1.height + vertexData2.height), ratio);
					normal = mix(vertexData0.normalTerrain, 0.5 * (vertexData1.normalTerrain + vertexData2.normalTerrain), ratio);
				}
			}
			
			
			#ifdef WIRED_MODE
				fragmentPosition_gs = p;
				gl_Position = projection * view * fragmentPosition_gs;
				fragmentNormal_gs = normal;
				fragmentUv_gs = vertexCoordFloat;
				terrainData0_gs = areaData0;
			#else
				#ifdef SHADOW_PASS
					gl_Position = p;
				#else
					fragmentPosition = p;
					gl_Position = projection * view * fragmentPosition;
					fragmentNormal = normal;
					fragmentUv = vertexCoordFloat;
					terrainData0 = areaData0;
				#endif
			#endif
		}
	};
	geometry : 
	{
		#pragma WIRED_MODE
		#pragma SHADOW_PASS
		
		#include "UniformBuffers.cginc"
		
		#ifdef WIRED_MODE
			layout(triangles) in;
			layout(triangle_strip, max_vertices = 3) out;

			// input
			in vec4 fragmentPosition_gs[];
			in vec4 fragmentNormal_gs[];
			in vec4 fragmentUv_gs[];
			flat in vec4 terrainData0_gs[];

			// output
			out vec4 fragmentNormal;
			out vec4 fragmentPosition;
			out vec4 fragmentUv;
			out vec3 barycentricCoord;
			flat out vec4 terrainData0;

			void main()
			{
				gl_Position = gl_in[0].gl_Position;
				fragmentPosition = fragmentPosition_gs[0];
				fragmentNormal = fragmentNormal_gs[0];
				fragmentUv = fragmentUv_gs[0];
				barycentricCoord = vec3(1.0 , 0.0 , 0.0);
				terrainData0 = terrainData0_gs[0];
				EmitVertex();
				
				gl_Position = gl_in[1].gl_Position;
				fragmentPosition = fragmentPosition_gs[1];
				fragmentNormal = fragmentNormal_gs[1];
				fragmentUv = fragmentUv_gs[1];
				barycentricCoord = vec3(0.0 , 1.0 , 0.0);
				//terrainData0 = terrainData0_gs[1];
				EmitVertex();
				
				gl_Position = gl_in[2].gl_Position;
				fragmentPosition = fragmentPosition_gs[2];
				fragmentNormal = fragmentNormal_gs[2];
				fragmentUv = fragmentUv_gs[2];
				barycentricCoord = vec3(0.0 , 0.0 , 1.0);
				//terrainData0 = terrainData0_gs[2];
				EmitVertex();
				
				EndPrimitive();
			}
		#else
			#ifndef GEOMETRY_INVOCATION
				#define GEOMETRY_INVOCATION 4
			#endif
		
			layout(triangles, invocations = GEOMETRY_INVOCATION) in;
			layout(triangle_strip, max_vertices = 3) out;
			
			uniform int shadowOmniLayerUniform = 0;
			
			void main()
			{
				if ((shadingConfiguration & (1<<4)) != 0)
				{
					for (int i = 0; i < 3; ++i)
					{
						gl_Layer = 6 * shadowOmniLayerUniform + gl_InvocationID;
						gl_Position = omniShadowProjections[ gl_Layer ] * gl_in[i].gl_Position;
						EmitVertex();
					}
					EndPrimitive();
				}
				else
				{
					for (int i = 0; i < 3; ++i)
					{
						gl_Layer = gl_InvocationID;
						vec4 screenPos = shadowCascadeProjections[ gl_Layer ] * gl_in[i].gl_Position;
						screenPos.z = max(screenPos.z, -1.0);
						gl_Position = screenPos;
						EmitVertex();
					}
					EndPrimitive();				
				}
			}
		#endif
	};
	fragment :
	{
		#include "UniformBuffers.cginc"
		
		// input
		in vec4 fragmentPosition;
		in vec4 fragmentUv;
		
		uniform vec4 constantData[16];
		
	#ifdef SHADOW_PASS
		void main()
		{
			vec2 samplinguv = fragmentPosition.xz;
			ivec2 corner = ivec2(int(fragmentUv.x), int(fragmentUv.y));
			bool hole0 = GetVertexHoleData(corner, constantData[0].z, constantData[0].w);
			bool hole1 = GetVertexHoleData(corner + ivec2(0, 1), constantData[0].z, constantData[0].w);
			bool hole2 = GetVertexHoleData(corner + ivec2(1, 0), constantData[0].z, constantData[0].w);
			bool hole3 = GetVertexHoleData(corner + ivec2(1, 1), constantData[0].z, constantData[0].w);
			if (hole0 && hole1 && hole2 && hole3)
				discard;
		}
	#else
		uniform sampler2DArray materialCollection;
		uniform sampler2DArrayShadow cascadedShadow;
		uniform samplerCubeArray omniShadowArray;
		
		// images
		layout(rgba32ui) readonly uniform uimage3D lightClusters;			// image unit 0
		
		// input
		in vec4 fragmentNormal;
		flat in vec4 terrainData0;
			
		#ifdef WIRED_MODE
			in vec3 barycentricCoord;
		#endif

		// output
		layout (location = 0) out vec4 fragColor;
		
		// usefull
		vec4 normal = vec4(0.0);
		vec4 albedoColor = vec4(0.2, 0.7, 0.2, 1.0);
		vec4 metalicParam = vec4(0.0);
		vec4 fragmentColor = vec4(0.0);
		vec4 clusterColor = vec4(0.0);
		vec4 cascadeIndexAll = vec4(0,1,2,3);
		vec4 cascadeColor = vec4(0.0);
		bool isHole = false;
		
		struct MaterialSamplingResult
		{
			vec4 m_albedo;
			vec4 m_normal;
			float m_metalic;
			int m_index;
		};
		
		// debug
		vec4 lodcolors[8] = vec4[]( vec4(0,1,0,1), vec4(1,1,0,1), vec4(1,0.5,0,1), vec4(1,0,0,1), vec4(1,0,1,1), vec4(0,0,1,1), vec4(1,1,1,1), vec4(0.5,0.5,0.5,1) );
		
		float map(float value, float min1, float max1, float min2, float max2)
		{
		  return min2 + (value - min1) * (max2 - min2) / max(max1 - min1, 0.0001);
		}
		float mix3(float a, float b, float x)
		{
			x = x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
			return mix(a, b, x);
		}
		vec4 biLerp(vec4 a, vec4 b, vec4 c, vec4 d, float s, float t)
		{
			vec4 x = mix(a, b, t);
			vec4 y = mix(c, d, t);
			return mix(x, y, s);
		}
		float biLerpf(float a, float b, float c, float d, float s, float t)
		{
			float x = mix(a, b, t);
			float y = mix(c, d, t);
			return mix(x, y, s);
		}
		
		float ComputeMorphingRatio(float camDistance)
		{
			int meshLod = int(constantData[0].x);
			float allRadius[8] = float[](constantData[1].x, constantData[1].y, constantData[1].z, constantData[1].w, constantData[2].x, constantData[2].y, constantData[2].z, constantData[2].w);
			float morphRange = constantData[3].x;
			for (int i = 0; i < 8; i++)
			{
				if (camDistance < allRadius[i])
				{
					if (meshLod < i)
						return 1.0;
					else if (camDistance > allRadius[i] - morphRange)
						return (camDistance - (allRadius[i] - morphRange)) / morphRange;
					return 0.0;
				}
			}
			return 1.0;
		}
		
		ivec3 ComputeClusterIndex()
		{
			ivec3 clusterSize = imageSize(lightClusters);
			vec4 fragmentViewPosition = view * fragmentPosition;
			ivec3 clusterIndex;
			clusterIndex.z = int(log(-fragmentViewPosition.z) * clusterDepthScale - clusterDepthBias);
			float depthBound = pow(far / near, (clusterIndex.z + 1.0) / clusterSize.z) * near;
			vec2 frustrumSize = vec2(tanFovX * depthBound, tanFovY * depthBound);
			vec2 floatIndex= ((fragmentViewPosition.xy + frustrumSize) / (2.0 * frustrumSize)) * clusterSize.xy;
			clusterIndex.xy = ivec2(floatIndex);
			clusterIndex = clamp(clusterIndex, ivec3(0), clusterSize - ivec3(1));
			return clusterIndex;
		}
		float GetOmniShadowAttenuation(int lightIndex, int omniIndex, vec4 rayLight, float currentDistance, float lightNear)
		{
			float range = lights[lightIndex].m_range;
			vec4 dir = abs(rayLight);
			float maxDir = max(dir.x, max(dir.y, dir.z));
			float currentDepth = (1 / maxDir - 1 / lightNear) / (1 / range - 1 / lightNear);
			
			vec3 ray = rayLight.xyz / currentDistance;
			vec3 u = abs(ray.x) > abs(ray.z) ? vec3(ray.y, -ray.x, 0) : vec3(0, ray.z, -ray.y);
			normalize(u);
			vec3 v = cross(ray, u);
			
			float shadow = 0.0;
			float texelSize = 1.0 / textureSize(omniShadowArray, 0).x;
			u *= texelSize;  v *= texelSize;
			for(int i = -2; i <= 2; i++)
				for(int j = -2; j <= 2; j++)
				{
					shadow += texture(omniShadowArray, vec4(ray + i * u + j * v, omniIndex)).r < currentDepth ? 0.0 : 1.0;
				}
			shadow /= 25;
			
			return shadow;
		}
		void ProcessLight(int lightIndex, vec4 albedo, float metalic, vec4 viewDir)
		{
			// search for shadow
			vec4 lightRay = fragmentPosition - lights[lightIndex].m_position;
			float currentDistance = length(lightRay);
			float shadowAttenuation = 1.0;
			for(int i = 0; i < 4; i++)
			{
				if (omniShadowIndexLow[i] == lightIndex)
				{
					shadowAttenuation = GetOmniShadowAttenuation(lightIndex, i, lightRay, currentDistance, omniShadowNearLow[i]);
					break;
				}
				else if (omniShadowIndexHigh[i] == lightIndex)
				{
					shadowAttenuation = GetOmniShadowAttenuation(lightIndex, 4 * i, lightRay, currentDistance, omniShadowNearHigh[i]);
					break;
				}
			}
		
			// point light computation
			float u = currentDistance / lights[lightIndex].m_range;
			
			lightRay /= currentDistance;
			vec4 lightColor = lights[lightIndex].m_color;
			vec4 diffuse = clamp(dot(normal, normalize(-lightRay)), 0 , 1 ) * lightColor;
			float spec = pow(max(dot(viewDir, lightRay), 0.0), 32);
			vec4 specular = metalic * spec * lightColor;
			float attenuation = lights[lightIndex].m_intensity / (1.0 + 25 * u * u)* clamp((1 - u) * 5.0 , 0 , 1);
			
			// spot attenuation
			vec4 lightDirection = lights[lightIndex].m_direction;
			float isSpotMultiplier = 1 - length(lightDirection);
			float outCutoff = lights[lightIndex].m_outCutOff;
			float inCutoff = lights[lightIndex].m_inCutOff;
			float cutoffRange = outCutoff - inCutoff;
			float cutoffClamped = clamp(dot(lightRay, lightDirection), outCutoff, inCutoff);
			float cutoff01 = map(cutoffClamped, outCutoff, inCutoff, 0 , 1);
			float spotAttenuation = max(isSpotMultiplier, cutoff01);
			
			// additive blending
			fragmentColor += (shadowAttenuation * spotAttenuation * attenuation) * ((diffuse + specular) * albedo);
		}
		int ComputeCascadeIndex()
		{
			float dist = abs(view * fragmentPosition).z;
			int index = 4;
			for (int i = 0; i < 4; ++i)
				if (shadowFarPlanes[i] > dist)
				{
					index = i;
					break;
				}
				
			if (index < 3)
				cascadeColor[index] = 1;
			else if (index == 3)
				cascadeColor = vec4(1 , 1 , 0 , 1);
			return index;
		}
		float GetShadowAttenuation(int cascadeIndex, float bias)
		{
			// only cascade 3 for terrain
			//float dist = abs(view * fragmentPosition).z;
			
			if (cascadeIndex >= 4)
				return 1.0;
			//if (shadowFarPlanes[3] > dist) cascadeIndex = 3;
			//else return 1.0;
			//cascadeColor = vec4(1 , 1 , 0 , 1);
			
			vec4 shadowPosition = shadowCascadeProjections[cascadeIndex] * fragmentPosition;
			vec4 shadowCoord = shadowPosition / shadowPosition.w;
			shadowCoord = shadowCoord * 0.5 + 0.5;
			float shadow = 0.0;
			
			// PCF
			vec2 texelSize = 1.0 / vec2(textureSize(cascadedShadow, 0));
			for(int x = -1; x <= 1; ++x)
				for(int y = -1; y <= 1; ++y)
				{
					shadow += texture(cascadedShadow, vec4(shadowCoord.xy + vec2(x, y) * texelSize, cascadeIndex, shadowCoord.z - bias));       
				}
			shadow /= 9.0;
			return shadow;
		}
		
		vec4 SampleQuadMaterials()
		{
			vec2 samplinguv = fragmentPosition.xz;
			ivec2 corner = ivec2(int(fragmentUv.x), int(fragmentUv.y));
			VertexData data[4];
			data[0] = GetVertexData(corner, constantData[0].z, constantData[0].w);
			data[1] = GetVertexData(corner + ivec2(0, 1), constantData[0].z, constantData[0].w);
			data[2] = GetVertexData(corner + ivec2(1, 0), constantData[0].z, constantData[0].w);
			data[3] = GetVertexData(corner + ivec2(1, 1), constantData[0].z, constantData[0].w);
			if (data[0].holeTerrain && data[1].holeTerrain && data[2].holeTerrain && data[3].holeTerrain)
			{
				isHole = true;
				return normal;
			}
			float mipmapLevel = 0.0;
			{
				float d = 0.2 * length(cameraPosition - fragmentPosition);
				mipmapLevel = clamp(log(d), 0.0 , float(textureQueryLevels(materialCollection)));
				float lowermip = floor(mipmapLevel);
				mipmapLevel = mix(lowermip, lowermip + 1.0, smoothstep(0.0, 1.0, 3.0 * fract(mipmapLevel) - 1.0));
			}
			
			MaterialSamplingResult quadMaterials[4];
			MaterialSamplingResult sampresult[8];
			int sampresLength = 0;
			for (int i = 0; i < 4; i++)
			{
				// search and sample if necessary material 0
				int material0 = int(data[i].material0);
				int index = -1;
				for (int j = 0; j < sampresLength; j++)
				{
					if (sampresult[j].m_index == material0)
					{
						index = j;
						break;
					}
				}
				if (index < 0)
				{
					index = sampresLength;
					sampresLength++;
					
					TerrainMaterial tmat = terrainMaterials[material0];
					sampresult[index].m_index = material0;
					sampresult[index].m_metalic = tmat.m_metalic;
					vec2 tmpuv = tmat.m_tiling * samplinguv;
					sampresult[index].m_albedo = textureLod(materialCollection, vec3(tmpuv, tmat.m_albedo), mipmapLevel);
					sampresult[index].m_normal = textureLod(materialCollection, vec3(tmpuv, tmat.m_normal), mipmapLevel);
				}
				quadMaterials[i] = sampresult[index];
				
				
				// if mat 1 is not assign, job done
				int material1 = int(data[i].material1);
				if (material1 == 0xFF)
					continue;
					
				// search and sample if necessary material 0
				index = -1;
				for (int j = 0; j < sampresLength; j++)
				{
					if (sampresult[j].m_index == material1)
					{
						index = j;
						break;
					}
				}
				if (index < 0)
				{
					index = sampresLength;
					sampresLength++;
					
					TerrainMaterial tmat = terrainMaterials[material1];
					sampresult[index].m_index = material1;
					sampresult[index].m_metalic = tmat.m_metalic;
					vec2 tmpuv = tmat.m_tiling * samplinguv;
					sampresult[index].m_albedo = textureLod(materialCollection, vec3(tmpuv, tmat.m_albedo), mipmapLevel);
					sampresult[index].m_normal = textureLod(materialCollection, vec3(tmpuv, tmat.m_normal), mipmapLevel);
				}
				
				// blend vertex materials
				float weight = data[i].material0weight;
				quadMaterials[i].m_metalic = mix(quadMaterials[i].m_metalic, sampresult[index].m_metalic, weight);
				quadMaterials[i].m_albedo = mix(quadMaterials[i].m_albedo, sampresult[index].m_albedo, weight);
				quadMaterials[i].m_normal = mix(quadMaterials[i].m_normal, sampresult[index].m_normal, weight);
			}
			
			vec2 subuv = fragmentUv.xy - vec2(corner.x, corner.y);
			//subuv = smoothstep(vec2(0.0), vec2(1.0), subuv);
			float finalMetalic = biLerpf(quadMaterials[0].m_metalic, quadMaterials[1].m_metalic, quadMaterials[2].m_metalic, quadMaterials[3].m_metalic, subuv.x, subuv.y);
			albedoColor = biLerp(quadMaterials[0].m_albedo, quadMaterials[1].m_albedo, quadMaterials[2].m_albedo, quadMaterials[3].m_albedo, subuv.x, subuv.y);
			vec4 texNormal = biLerp(quadMaterials[0].m_normal, quadMaterials[1].m_normal, quadMaterials[2].m_normal, quadMaterials[3].m_normal, subuv.x, subuv.y);
			
			metalicParam = vec4(finalMetalic, 0, 0, 0);
			texNormal = 2.0 * texNormal - vec4(1.0);
			vec3 tangent = normalize(cross(vec3(1.0, 0.0, 0.0), normal.xyz));
			vec3 bitangent = cross(normal.xyz, tangent);
			vec4 newNormal;
			newNormal.xyz = texNormal.x * bitangent + texNormal.y * tangent + texNormal.z * normal.xyz;
			newNormal.w = 0.0;
			
		#ifdef ALLOW_DEBUG
			if ((floatBitsToInt(constantData[3].z) & 0x08) != 0)//eMaterialSampling
				albedoColor = mix(albedoColor, lodcolors[sampresLength - 1], 0.3);
		#endif
			
			return normalize(newNormal);
		}
		
		vec4 ApplyFog(float fragmentDistance, vec4 viewDir, vec4 lightDir)
		{
			float fogAmount = 1.0 - exp(-fragmentDistance * m_ambientColor.w);
			float sunAmount = max(dot(viewDir, lightDir), 0.0 );
			vec4 fogColor = mix(vec4(0.5 , 0.6 , 0.7 , 1.0), vec4(1.0 , 0.9 , 0.7 , 1.0), pow(sunAmount,8.0)) * m_directionalLightColor;
			return mix(fragmentColor, fogColor, fogAmount);
		}
		
		// program
		void main()
		{
			#ifdef WIRED_MODE
				vec3 a3 = smoothstep(vec3(0.0), fwidth(barycentricCoord) * wireframeEdgeFactor , barycentricCoord);
				float edgeFactor = min(min(a3.x, a3.y), a3.z);
				if(edgeFactor >= 1.0)
					discard;
			#endif
			
			// compute base color depending on environement light (directional)
			normal = normalize(fragmentNormal);
			vec4 lightDir = normalize(m_directionalLightDirection);
			float ndotl = dot(normal, -lightDir);
			float shadowAttenuation = ndotl > 0.0 ? 1.0 : 0.0;
			if ((shadingConfiguration & 0x04) != 0 && ndotl >= 0.0)
			{
				int cascadeIndex = ComputeCascadeIndex();
				shadowAttenuation = GetShadowAttenuation(cascadeIndex, 10 * max(0.005 * (1.0 - ndotl), 0.0005));
			}
			
			normal = SampleQuadMaterials();
			if (isHole) discard;
			
			ndotl = ndotl < 0.001 ? ndotl : dot(normal, -lightDir);
			vec4 diffuse = clamp(ndotl, 0 , 1 ) * m_directionalLightColor;
			float fragmentDistance = distance(cameraPosition, fragmentPosition);
			vec4 viewDir = normalize(cameraPosition - fragmentPosition);
			vec4 reflectDir = reflect(lightDir, normal);  
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
			vec4 specular = metalicParam.x * spec * m_directionalLightColor;  
			fragmentColor = (m_ambientColor + shadowAttenuation * (diffuse + specular)) * albedoColor;
			
			// process cluster data
			if (lightCount > 0)
			{
				ivec3 clusterIndex = ComputeClusterIndex();
				uvec4 clusterData = imageLoad(lightClusters, clusterIndex);
				int lightSum = 0;
				
				if ((shadingConfiguration & 0x01) != 0)
				{
					for (int i = 0; i < 4; i++)
					{
						int l0 =  int(clusterData[i] & 0xFF);
						if (l0 == 0xFF) break;
						lightSum++;
						ProcessLight(l0, albedoColor, metalicParam.x, viewDir);
							
						int l1 = int((clusterData[i] >> 8) & 0xFF);
						if (l1 == 0xFF) break;
						lightSum++;
						ProcessLight(l1, albedoColor, metalicParam.x, viewDir);
							
						int l2 = int((clusterData[i] >> 16) & 0xFF);
						if (l2 == 0xFF) break;
						lightSum++;
						ProcessLight(l2, albedoColor, metalicParam.x, viewDir);
							
						int l3 = int((clusterData[i] >> 24) & 0xFF);
						if (l3 == 0xFF) break;
						lightSum++;
						ProcessLight(l3, albedoColor, metalicParam.x, viewDir);
					}					
				}
				else
				{
					for (int i = 0; i < lightCount; i++)
					{
						lightSum++;
						ProcessLight(i, albedoColor, metalicParam.x, viewDir);
					}
				}
				
				#ifdef ALLOW_DEBUG
					if (shadingConfiguration != 0)
					{
						if ((shadingConfiguration & 0x02) != 0)
						{
							if (lightSum == 0.0)
								clusterColor = vec4(0.0);
							else if (lightSum <= 8)
								clusterColor = vec4(0.0 , 1.0 , 0.0 , 0.0);
							else if (lightSum <= 14)
								clusterColor = vec4(1.0 , 1.0 , 0.0 , 0.0);
							else
								clusterColor = vec4(1.0 , 0.0 , 0.0 , 0.0);				
						}
					}
				#endif
			}

			// fog
			fragmentColor = ApplyFog(fragmentDistance, viewDir, lightDir);
			
			// debug override
			#ifdef ALLOW_DEBUG
				if ((shadingConfiguration & 0x02) != 0)
					fragColor = mix(fragmentColor, clusterColor, 0.5);
				else if((shadingConfiguration & 0x08) != 0)
					fragColor = mix(fragmentColor, cascadeColor, 0.5);
				else if ((floatBitsToInt(constantData[3].z) & 0x01) != 0)//eDrawCheckboardPatern
				{
					vec2 areaCorner = vec2(terrainData0.xy + 125.0) / 250.0;
					int odd = ((int(areaCorner.x) + int(areaCorner.y))+200) & 0x000001;
					fragColor = mix(fragmentColor, vec4(odd, 0, 0 , 1.0), 0.2);
				}
				else if ((floatBitsToInt(constantData[3].z) & 0x02) != 0)//eDrawLodColor
				{
					int lod = int(constantData[0].x);
					fragColor = mix(fragmentColor, lodcolors[lod], 0.3);
				}
				else if ((floatBitsToInt(constantData[3].z) & 0x04) != 0)//eShowMorphingBand
				{
					vec4 delta = cameraPosition - fragmentPosition;
					float camDistance = sqrt(delta.x * delta.x + delta.z * delta.z);
					float ratio = ComputeMorphingRatio(camDistance);
					float band = smoothstep(0.95,1.0,ratio);
					vec4 col = mix(vec4(ratio, 0, 0, 1), vec4(1, 1, 1, 1), 2*band*(1.0-band));
					fragColor = mix(fragmentColor, col, 0.5);
				}
				else
					fragColor = fragmentColor;
			#else
				fragColor = fragmentColor;
			#endif
			
								
			//fragColor.xyz = normal.xyz - newnormal.xyz;
			fragColor.w = 1.0;
				
			//fragColor = terrainData0;
			//fragColor = vec4(normal.x, 0, normal.z, 1) ;//+ 0.5*fragmentColor;
			//int lod = int(constantData[0].x); fragColor = lodcolors[lod];
		}
	#endif
	};
} 