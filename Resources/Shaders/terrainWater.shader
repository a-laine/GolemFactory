TerrainWater
{
	renderQueue : 2900;//transparent
	transparent : true;
	faceCulling : false;
	
	uniform :
	{
		instanceDataArray : "vec4 array32";
		constantData : "vec4 array32";
		lightClusters : "_globalLightClusters";
		cascadedShadow : "_globalShadowCascades";
		omniShadowArray : "_globalOmniShadow";
		terrainVirtualTexture : "_terrainVirtualTexture";
		skybox : "_globalSkybox";
		omniBaseLayer = "int";
	};
	
	includes : 
	{
		#version 430
		
		struct VertexData
		{
			vec4 normalTerrain;
			vec4 normalWater;
			float height;
			float water;
			uint material;
			bool holeTerrain;
			bool holeWater;
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
			out vec4 fragmentPosition;
			out vec4 fragmentNormal;
			out vec4 fragmentUv;
			out vec4 terrainData0;
			out vec4 terrainData1;
		#endif
		
		VertexData GetVertexData(ivec2 vertexCoord)
		{
			float heightAmplitude = constantData[0].z;
			float seeLevel = constantData[0].w;
			uvec4 data = imageLoad(terrainVirtualTexture, vertexCoord);
			
			VertexData vdata;
			vdata.height = data.x * heightAmplitude - seeLevel;
			vdata.water = data.y * heightAmplitude - seeLevel;
			vdata.normalTerrain = octahedralUnpack(data.z, 7);
			vdata.normalWater = octahedralUnpack(data.w, 5);
			vdata.material = (data.w >> 10) & 0xFF;
			vdata.holeTerrain = (data.z & (1 << 14)) != 0;
			vdata.holeWater = (data.z & (1 << 15)) != 0;
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
		//float rand(vec2 co){ return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453); }
		vec4 hash(float x, float y)
		{
			int n0 = int(x * 311 + y * 113); n0 = (n0 << 13) ^ n0;
			int n1 = int(x * 313 + y * 107); n1 = (n1 << 11) ^ n1;
			int n2 = int(x * 307 + y * 109); n2 = (n2 << 17) ^ n2;
			int n3 = int(x * 317 + y * 103); n3 = (n3 << 19) ^ n3;
			ivec4 n = ivec4(n0, n1, n2, n3);
			n = n * (n * n * ivec4(15731) + ivec4(789221)) + ivec4(1376312589);
			return vec4(n.x & 0x0fffffff, n.y & 0x0fffffff, n.z & 0x0fffffff, n.w & 0x0fffffff) / vec4(0x0fffffff);
		}
		
		// program
		vec3 waveHarmonic[4] = vec3[]( vec3(0.0,0.2, 1.0), vec3(-0.3,0.0, 0.7), vec3(0.9,0.42, 0.5), vec3(-1.0,0.0, 3.0) );
		void main()
		{
			vec4 areaData0 = instanceDataArray[2 * gl_InstanceID];
			vec4 areaData1 = instanceDataArray[2 * gl_InstanceID + 1];
			
			//int lod = int(constantData[0].x);
			//int tileSize = int(constantData[0].y);
			
			vec4 vertexCoordFloat = areaData0.zwzw + uv.xyxy;
			vertexCoordFloat.z = 0.0;
			VertexData vertexData0 = GetVertexData(ivec2(vertexCoordFloat.x , vertexCoordFloat.y));
			vec4 p = vec4(position.x + areaData0.x, vertexData0.water,  position.z + areaData0.y, 1.0);
			float waterDepth = vertexData0.water - vertexData0.height;
			vec4 normal = vertexData0.normalWater;
			
			vec4 delta = p - cameraPosition;
			float camDistance = sqrt(delta.x * delta.x + delta.z * delta.z);
			float morphRatio = ComputeMorphingRatio(camDistance);
			float displacementRatio = 0.0;
			int lod = int(constantData[0].x + 0.2);
			if (lod < 1)
				displacementRatio = 1.0;
			else if (lod == 1)
				displacementRatio = 1.0 - morphRatio;
			if (vertexData0.holeWater)
				displacementRatio = 0.0;
			
			// small movement
			if (displacementRatio > 0.0)
			{
				vec2 dnormal = vec2(0.0);
				float amp = 0.1 * displacementRatio * clamp(waterDepth, 0.1, 1.0);
				for (int i = 0; i < 4; i++)
				{
					float teta = animatedTime * waveHarmonic[i].z + dot(waveHarmonic[i].xy, p.xz);
					p += (amp * sin(teta)) * normal;
					dnormal -= (amp * cos(teta)) * waveHarmonic[i].xy;				
				}
				normal.xz += dnormal;
			}
			normalize(normal);
			
			// morphing
			/*if (uv.z != 0.0 || uv.w != 0.0)
			{
				if (morphRatio > 0.0)
				{
					VertexData vertexData1 = GetVertexData(ivec2(vertexCoordFloat.x + uv.z , vertexCoordFloat.y + uv.w));
					VertexData vertexData2 = GetVertexData(ivec2(vertexCoordFloat.x - uv.z , vertexCoordFloat.y - uv.w));
					vertexCoordFloat.z = morphRatio;
					
					waterDepth = mix(waterDepth, 0.5 * (vertexData1.water + vertexData2.water - vertexData1.height - vertexData2.height), morphRatio);
					
					float uv2pos = 250.0 / 256.0;
					vec4 p0 = vec4(position.x + areaData0.x, 0, position.z + areaData0.y, 1);
					vec4 p1 = vec4(p0.x + uv.w * uv2pos, vertexData1.water,  p0.z + uv.z * uv2pos, 1.0);
					vec4 d1 = hash(p1.x, p1.z);
					p1 += d1 * displacementAmplitude * sin(animatedTime + d1.w + p1.x + 0.1 * p1.z);
					vec4 p2 = vec4(p0.x - uv.w * uv2pos, vertexData2.water,  p0.z - uv.z * uv2pos, 1.0);
					vec4 d2 = hash(p2.x, p2.z);
					p2 += d2 * displacementAmplitude * sin(animatedTime + d2.w + p2.x + 0.1 * p2.z);
					
					p = mix(p, 0.5 * (p1 + p2), morphRatio);
				}
			}*/
			
			// color
			vec4 waterColor = vec4(0.1 , 0.3 , 0.7 , 0.2);
			waterColor.w = mix(0.4 , 1.0 , clamp(0.3 * waterDepth, 0.0, 1.0));
			areaData1 = waterColor;
			
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
		#endif
	};
	fragment :
	{
		#include "UniformBuffers.cginc"
		
		// textures
		//uniform sampler2D albedo;   //sampler unit 0
		//uniform sampler2D emmisive; //sampler unit 1
		//uniform sampler2D metalic;  //sampler unit 2
		
		uniform sampler2DArrayShadow  cascadedShadow;
		uniform samplerCubeArray omniShadowArray;
		uniform samplerCube skybox;
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
		vec4 fragmentColor = vec4(0.0);
		vec4 clusterColor = vec4(0.0);
		vec4 cascadeIndexAll = vec4(0,1,2,3);
		vec4 cascadeColor = vec4(0.0);
		
		// debug		
		VertexData GetVertexData(ivec2 vertexCoord)
		{
			float heightAmplitude = constantData[0].z;
			float seeLevel = constantData[0].w;
			uvec4 data = imageLoad(terrainVirtualTexture, vertexCoord);
			
			VertexData vdata;
			vdata.height = data.x * heightAmplitude - seeLevel;
			vdata.water = data.y * heightAmplitude - seeLevel;
			vdata.normalTerrain = octahedralUnpack(data.z, 7);
			vdata.normalWater = octahedralUnpack(data.w, 5);
			vdata.material = (data.w >> 10) & 0xFF;
			vdata.holeTerrain = (data.z & (1 << 14)) != 0;
			vdata.holeWater = (data.z & (1 << 15)) != 0;
			return vdata;
		}
		float map(float value, float min1, float max1, float min2, float max2)
		{
		  return min2 + (value - min1) * (max2 - min2) / max(max1 - min1, 0.0001);
		}
		float mix3(float a, float b, float x)
		{
			x = x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
			return mix(a, b, x);
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
		
		float GetShadowAttenuation(float bias)
		{
			// only cascade 3 for terrain
			float dist = abs(view * fragmentPosition).z;
			int cascadeIndex = 4;
			if (shadowFarPlanes[3] > dist) cascadeIndex = 3;
			else return 1.0;
			cascadeColor = vec4(1 , 1 , 0 , 1);
			
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
			
			ivec2 corner = ivec2(int(fragmentUv.x), int(fragmentUv.y));
			VertexData data0 = GetVertexData(corner);
			VertexData data1 = GetVertexData(corner + ivec2(0, 1));
			VertexData data2 = GetVertexData(corner + ivec2(1, 0));
			VertexData data3 = GetVertexData(corner + ivec2(1, 1));
			if (data0.holeWater && data1.holeWater && data2.holeWater && data3.holeWater)
				discard;
			
			// compute base color depending on environement light (directional)
			normal = normalize(fragmentNormal);
			vec4 albedoColor = terrainData1;
			vec4 metalicParam = vec4(0.3 , 0.7 , 0 , 0);
			vec4 lightDir = normalize(m_directionalLightDirection);
			float ndotl = dot(normal, -lightDir);
			float shadowAttenuation = ndotl > 0.0 ? 1.0 : 0.0;
			if ((shadingConfiguration & 0x04) != 0 && ndotl > 0.0)
			{
				shadowAttenuation = GetShadowAttenuation(10 * max(0.005 * (1.0 - ndotl), 0.0005));		
			}
			
			vec4 diffuse = clamp(ndotl, 0 , 1 ) * m_directionalLightColor;
			float fragmentDistance = distance(cameraPosition, fragmentPosition);
			vec4 viewDir = normalize(cameraPosition - fragmentPosition);
			vec4 reflectDir = reflect(lightDir, normal);  
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
			vec4 specular = metalicParam.x * spec * m_directionalLightColor;
			reflectDir = reflect(viewDir, normal);
			vec4 reflectionColor = texture(skybox, reflectDir.xyz);
			fragmentColor = (m_ambientColor + shadowAttenuation * (diffuse + specular)) * albedoColor;
			fragmentColor = mix(fragmentColor, reflectionColor, clamp(dot(normal, viewDir), 0.0, 1.0) * metalicParam.y);
			
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
			
			fragmentColor = ApplyFog(fragmentDistance, viewDir, lightDir);
			fragmentColor.w = albedoColor.w;
			
			// debug override
			if ((shadingConfiguration & 0x02) != 0)
				fragColor = 0.5 * fragmentColor + 0.5 * clusterColor;
			else if((shadingConfiguration & 0x08) != 0)
				fragColor = 0.5 * fragmentColor + 0.5 * cascadeColor;
			else
				fragColor = fragmentColor;
				
				
			//fragColor = terrainData0;
			//fragColor = vec4(normal.x, 0, normal.z, 1) ;//+ 0.5*fragmentColor;
			//int lod = int(constantData[0].x); fragColor = lodcolors[lod];
		}
	};
} 