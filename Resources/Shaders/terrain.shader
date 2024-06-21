Terrain
{	
	renderQueue : 1000;//opaque
	
	uniform :
	{
		instanceDataArray : "vec4 array32";
		constantData : "vec4 array32";
		lightClusters : "_globalLightClusters";
		cascadedShadow : "_globalShadowCascades";
		materialCollection : "_globalTerrainMaterialCollection";
		omniShadowArray : "_globalOmniShadow";
		terrainVirtualTexture : "_terrainVirtualTexture";
		omniBaseLayer = "int";
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
	};
	vertex :
	{
		#include "UniformBuffers.cginc"
	
		// input
		layout(location = 0) in vec4 position;
		layout(location = 2) in vec4 uv;
		
		layout(rgba16ui) readonly uniform uimage2D terrainVirtualTexture;	// image unit 1
		
		#define INSTANCING
		// following cannot exceed 1024
		uniform vec4 instanceDataArray[1000];
		uniform vec4 constantData[16];
		
		// output
		#ifdef WIRED_MODE
			out vec4 fragmentPosition_gs;
			out vec4 fragmentNormal_gs;
			out vec4 fragmentUv_gs;
			out vec4 terrainData0_gs;
			out vec4 terrainData1_gs;
		#else
			#ifdef SHADOW_PASS
				// nothing to out more than 'gl_Position'
			#else
				out vec4 fragmentPosition;
				out vec4 fragmentNormal;
				out vec4 fragmentUv;
				out vec4 terrainData0;
				out vec4 terrainData1;
			#endif
		#endif
		
		struct VertexData
		{
			float height;
			float water;
			vec2 normal_xz;
			uint material;
			uint hole;
		};
		
		VertexData GetVertexData(ivec2 vertexCoord)
		{
			float heightAmplitude = constantData[0].z;
			float seeLevel = constantData[0].w;
			
			uvec4 data = imageLoad(terrainVirtualTexture, vertexCoord);
			
			VertexData vdata;
			vdata.height = data.x * heightAmplitude - seeLevel;
			vdata.water = data.y * heightAmplitude - seeLevel;
			
			uint normalx = (data.z & 0x7FF);
			uint normalz = (data.z >> 11) | ((data.w & 0x3F) << 5);
			vdata.normal_xz = vec2((normalx / 1024.0) - 1.0, (normalz / 1024.0) - 1.0);
			vdata.material = (data.w >> 6) & 0xFF;
			vdata.hole = (data.w >> 14) & 0x03;
			return vdata;
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
		float rand(vec2 co){ return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453); }
			
		// program
		void main()
		{
			vec4 areaData0 = instanceDataArray[2 * gl_InstanceID];
			vec4 areaData1 = instanceDataArray[2 * gl_InstanceID + 1];
			
			//int lod = int(constantData[0].x);
			//int tileSize = int(constantData[0].y);
			
			vec4 vertexCoordFloat = areaData0.zwzw + uv.xyxy;
			vertexCoordFloat.z = 0.0;
			VertexData vertexData0 = GetVertexData(ivec2(vertexCoordFloat.x , vertexCoordFloat.y));
			vec4 p = vec4(position.x + areaData0.x, vertexData0.height,  position.z + areaData0.y, 1.0);
			vec2 n = vertexData0.normal_xz;
			
			//areaData0 = vec4(1.0, 1.0, 1.0, 1.0);
			
			if (uv.z != 0.0 || uv.w != 0.0)
			{
				vec4 delta = p - cameraPosition;
				float camDistance = sqrt(delta.x * delta.x + delta.z * delta.z);
				float ratio = ComputeMorphingRatio(camDistance);
				
				if (ratio > 0.0)
				{
					VertexData vertexData1 = GetVertexData(ivec2(vertexCoordFloat.x + uv.z , vertexCoordFloat.y + uv.w));
					VertexData vertexData2 = GetVertexData(ivec2(vertexCoordFloat.x - uv.z , vertexCoordFloat.y - uv.w));
					vertexCoordFloat.z = ratio;
					
					p.y = mix(vertexData0.height, 0.5 * (vertexData1.height + vertexData2.height), ratio);
					n = mix(vertexData0.normal_xz, 0.5 * (vertexData1.normal_xz + vertexData2.normal_xz), ratio);
					
					/*if (uv.z == -1.0 && uv.w == 1.0)
						areaData0 = mix(areaData0, vec4(1.0,0.0,0.0,1.0), ratio);
					else if (uv.z == 1.0 && uv.w == 1.0)
						areaData0 = mix(areaData0, vec4(0.0,1.0,0.0,1.0), ratio);
					else if (uv.z == 0.0 && uv.w == 1.0)
						areaData0 = mix(areaData0, vec4(0.2,0.2,1.0,1.0), ratio);
					else
						areaData0 = mix(areaData0, vec4(1.0,1.0,0.0,1.0), ratio);*/
				}
			}
			
			float nn = min(n.x * n.x + n.y * n.y, 1.0);
			float y = sqrt(1.0 - nn);
			vec4 normal = vec4(-n.x, y, -n.y , 0.0);
			
			#ifdef WIRED_MODE
				fragmentPosition_gs = p;
				gl_Position = projection * view * fragmentPosition_gs;
				fragmentNormal_gs = normal;
				fragmentUv_gs = vertexCoordFloat;
				terrainData0_gs = areaData0;
				terrainData1_gs = areaData1;
			#else
				#ifdef SHADOW_PASS
					gl_Position = p;
				#else
					fragmentPosition = p;
					gl_Position = projection * view * fragmentPosition;
					fragmentNormal = normal;
					fragmentUv = vertexCoordFloat;
					terrainData0 = areaData0;
					terrainData1 = areaData1;
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
			in vec4 terrainData0_gs[];
			in vec4 terrainData1_gs[];

			// output
			out vec4 fragmentNormal;
			out vec4 fragmentPosition;
			out vec4 fragmentUv;
			out vec3 barycentricCoord;
			out vec4 terrainData0;
			out vec4 terrainData1;

			void main()
			{
				gl_Position = gl_in[0].gl_Position;
				fragmentPosition = fragmentPosition_gs[0];
				fragmentNormal = fragmentNormal_gs[0];
				fragmentUv = fragmentUv_gs[0];
				barycentricCoord = vec3(1.0 , 0.0 , 0.0);
				terrainData0 = terrainData0_gs[0];
				terrainData1 = terrainData1_gs[0];
				EmitVertex();
				
				gl_Position = gl_in[1].gl_Position;
				fragmentPosition = fragmentPosition_gs[1];
				fragmentNormal = fragmentNormal_gs[1];
				fragmentUv = fragmentUv_gs[1];
				barycentricCoord = vec3(0.0 , 1.0 , 0.0);
				terrainData0 = terrainData0_gs[1];
				terrainData1 = terrainData1_gs[1];
				EmitVertex();
				
				gl_Position = gl_in[2].gl_Position;
				fragmentPosition = fragmentPosition_gs[2];
				fragmentNormal = fragmentNormal_gs[2];
				fragmentUv = fragmentUv_gs[2];
				barycentricCoord = vec3(0.0 , 0.0 , 1.0);
				terrainData0 = terrainData0_gs[2];
				terrainData1 = terrainData1_gs[2];
				EmitVertex();
				
				EndPrimitive();
			}
		#else
			#ifndef GEOMETRY_INVOCATION
				#define GEOMETRY_INVOCATION 4
			#endif
		
			layout(triangles, invocations = GEOMETRY_INVOCATION) in;
			layout(triangle_strip, max_vertices = 3) out;
			
			uniform int omniBaseLayer = 0;
			
			void main()
			{
				if ((shadingConfiguration & (1<<4)) != 0)
				{
					for (int i = 0; i < 3; ++i)
					{
						gl_Layer = 6 * omniBaseLayer + gl_InvocationID;
						gl_Position = omniShadowProjections[ gl_Layer ] * gl_in[i].gl_Position;
						EmitVertex();
					}
					EndPrimitive();
				}
				else
				{
					for (int i = 0; i < 3; ++i)
					{
						gl_Position = shadowCascadeProjections[ gl_InvocationID ] * gl_in[i].gl_Position;
						gl_Layer = gl_InvocationID;
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
		
	#ifdef SHADOW_PASS
		void main() { }
	#else
		uniform sampler2DArray materialCollection;
		uniform sampler2DArrayShadow cascadedShadow;
		uniform samplerCubeArray omniShadowArray;
		uniform vec4 constantData[16];
		
		// images
		layout(rgba32ui) readonly uniform uimage3D lightClusters;			// image unit 0
		layout(rgba16ui) readonly uniform uimage2D terrainVirtualTexture;	// image unit 1
		
		// input
		in vec4 fragmentPosition;
		in vec4 fragmentNormal;
		in vec4 fragmentUv;
		in vec4 terrainData0;
		in vec4 terrainData1;
			
		#ifdef WIRED_MODE
			in vec3 barycentricCoord;
		#endif

		// output
		layout (location = 0) out vec4 fragColor;
		
		// usefull
		vec4 normal = vec4(0.0);
		vec4 albedoColor = vec4(0.0);
		vec4 metalicParam = vec4(0.0);
		vec4 fragmentColor = vec4(0.0);
		vec4 clusterColor = vec4(0.0);
		vec4 cascadeIndexAll = vec4(0,1,2,3);
		vec4 cascadeColor = vec4(0.0);
		
		// debug
		vec4 lodcolors[8] = vec4[]( vec4(1,1,1,1), vec4(1,0,0,1), vec4(1,1,0,1), vec4(0,1,0,1), vec4(0,1,1,1), vec4(0,0,1,1), vec4(1,0,1,1), vec4(1,1,1,1) );
		
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
			if (cascadeIndex >= 4)
				return 1.0;
				
			vec4 shadowPosition = shadowCascadeProjections[cascadeIndex] * fragmentPosition;
			vec4 shadowCoord = shadowPosition / shadowPosition.w;
			shadowCoord = shadowCoord * 0.5 + 0.5;
			
			float shadow = texture(cascadedShadow, vec4(shadowCoord.xy, cascadeIndex, shadowCoord.z - bias));
			
			return shadow;
		}
		
		struct VertexData
		{
			float height;
			float water;
			vec2 normal_xz;
			uint material;
			uint hole;
		};
		VertexData GetVertexData(ivec2 vertexCoord)
		{
			float heightAmplitude = constantData[0].z;
			float seeLevel = constantData[0].w;
			
			uvec4 data = imageLoad(terrainVirtualTexture, vertexCoord);
			
			VertexData vdata;
			vdata.height = data.x * heightAmplitude - seeLevel;
			vdata.water = data.y * heightAmplitude - seeLevel;
			
			uint normalx = (data.z & 0x7FF);
			uint normalz = (data.z >> 11) | ((data.w & 0x3F) << 5);
			vdata.normal_xz = vec2((normalx / 1024.0) - 1.0, (normalz / 1024.0) - 1.0);
			vdata.material = (data.w >> 6) & 0xFF;
			vdata.hole = (data.w >> 14) & 0x03;
			return vdata;
		}
		vec4 SampleQuadMaterials()
		{
			vec2 samplinguv = terrainData0.xy + fragmentUv.xy;			
			VertexData data0 = GetVertexData(ivec2(int(fragmentUv.x), int(fragmentUv.y)));
			VertexData data1 = GetVertexData(ivec2(int(fragmentUv.x), int(fragmentUv.y + 1)));
			VertexData data2 = GetVertexData(ivec2(int(fragmentUv.x + 1), int(fragmentUv.y)));
			VertexData data3 = GetVertexData(ivec2(int(fragmentUv.x + 1), int(fragmentUv.y + 1)));
			
			float inter_metalic;
			vec4 inter_albedo;
			vec4 inter_normal;
			TerrainMaterial t0 = terrainMaterials[data0.material];
			//line = 277
			float metalic0 = t0.m_metalic;
			vec4 albedo0 = texture(materialCollection, vec3(t0.m_tiling * samplinguv, t0.m_albedo));
			vec4 normal0 = texture(materialCollection, vec3(t0.m_tiling * samplinguv, t0.m_normal));
			
			if (data0.material == data1.material && data0.material == data2.material && data0.material == data3.material)
			{
				inter_metalic = metalic0;
				inter_albedo = albedo0;
				inter_normal = normal0;
			}
			else
			{
				float metalic1, metalic2, metalic3;
				vec4 albedo1, albedo2, albedo3;
				vec4 normal1, normal2, normal3;
				
				if (data1.material == data0.material)
				{
					metalic1 = metalic0;
					albedo1 = albedo0;
					normal1 = normal0;
				}
				else
				{
					TerrainMaterial t1 = terrainMaterials[data1.material];
					metalic1 = t1.m_metalic;
					albedo1 = texture(materialCollection, vec3(t1.m_tiling * samplinguv, t1.m_albedo));
					normal1 = texture(materialCollection, vec3(t1.m_tiling * samplinguv, t1.m_normal));
				}
				
				if (data2.material == data0.material)
				{
					metalic2 = metalic0;
					albedo2 = albedo0;
					normal2 = normal0;
				}
				else if (data2.material == data1.material)
				{
					metalic2 = metalic1;
					albedo2 = albedo1;
					normal2 = normal1;
				}
				else
				{
					TerrainMaterial t2 = terrainMaterials[data2.material];
					metalic2 = t2.m_metalic;
					albedo2 = texture(materialCollection, vec3(t2.m_tiling * samplinguv, t2.m_albedo));
					normal2 = texture(materialCollection, vec3(t2.m_tiling * samplinguv, t2.m_normal));
				}
				
				if (data3.material == data0.material)
				{
					metalic3 = metalic0;
					albedo3 = albedo0;
					normal3 = normal0;
				}
				else if (data3.material == data1.material)
				{
					metalic3 = metalic1;
					albedo3 = albedo1;
					normal3 = normal1;
				}
				else if (data3.material == data2.material)
				{
					metalic3 = metalic2;
					albedo3 = albedo2;
					normal3 = normal2;
				}
				else
				{
					TerrainMaterial t3 = terrainMaterials[data3.material];
					metalic3 = t3.m_metalic;
					albedo3 = texture(materialCollection, vec3(t3.m_tiling * samplinguv, t3.m_albedo));
					normal3 = texture(materialCollection, vec3(t3.m_tiling * samplinguv, t3.m_normal));
				}
				
				vec2 subuv = fragmentUv.xy - vec2(int(fragmentUv.x), int(fragmentUv.y));
				inter_metalic = biLerpf(metalic0, metalic1, metalic2, metalic3, subuv.x, subuv.y);
				inter_albedo = biLerp(albedo0, albedo1, albedo2, albedo3, subuv.x, subuv.y);
				inter_normal = biLerp(normal0, normal1, normal2, normal3, subuv.x, subuv.y);
			}
		
			// normal mapping and end
			albedoColor = inter_albedo;
			metalicParam = vec4(inter_metalic, 0, 0, 0);
			vec3 tangent = normalize(cross(normal.xyz, vec3(1.0, 0.0, 0.0)));
			vec3 bitangent = cross(normal.xyz, tangent);
			vec4 newNormal;
			newNormal.xyz = inter_normal.x * tangent + inter_normal.y * bitangent + inter_normal.z * normal.xyz;
			newNormal.w = 0.0;
			return normalize(newNormal);
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
			normal = SampleQuadMaterials();
			float ndotl = dot(normal, normalize(-m_directionalLightDirection));
			vec4 diffuse = clamp(ndotl, 0 , 1 ) * m_directionalLightColor;
			
			float shadowAttenuation = 1.0;
			if ((shadingConfiguration & 0x04) != 0 && ndotl > 0.0)
			{
				int cascadeIndex = ComputeCascadeIndex();
				shadowAttenuation = GetShadowAttenuation(cascadeIndex, max(0.005 * (1.0 - ndotl), 0.0005));			
			}
			
			vec4 viewDir = normalize(cameraPosition - fragmentPosition);
			vec4 reflectDir = reflect(normalize(m_directionalLightDirection), normal);  
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
			}
			
			if ((shadingConfiguration & 0x02) != 0)
				fragColor = 0.5 * fragmentColor + 0.5 * clusterColor;
			else if((shadingConfiguration & 0x08) != 0)
				fragColor = 0.5 * fragmentColor + 0.5 * cascadeColor;
			else
				fragColor = fragmentColor;
				
			//fragColor.xyz = normal.xyz;
				
			//fragColor = terrainData0;
			//fragColor = vec4(normal.x, 0, normal.z, 1) ;//+ 0.5*fragmentColor;
			//int lod = int(constantData[0].x); fragColor = lodcolors[lod];
		}
	#endif
	};
} 