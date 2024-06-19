SkinnedTextured
{
	uniform :
	{
		matrixArray : "struct array32";
		skeletonPose : "mat4 array32";
		inverseBindPose : "mat4 array32";
		lightClusters : "_globalLightClusters";
		cascadedShadow : "_globalShadowCascades";
		omniShadowArray : "_globalOmniShadow";
		shadowOmniLayerUniform = "int"
	};
	
	textures : [
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
	];
	
	includes :
	{
		#version 430
	};
	vertex :
	{
		#include "UniformBuffers.cginc"
		
		#define MAX_SKELETON_BONE 64

		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in vec4 uv;
		layout(location = 3) in ivec4 boneIDs;
		layout(location = 4) in vec4 weights;


		uniform mat4 matrixArray[2];
		uniform mat4 skeletonPose[MAX_SKELETON_BONE]; 	// bone matrix
		uniform mat4 inverseBindPose[MAX_SKELETON_BONE]; 		// vertex to bone transformation matrix

		// output
		#ifdef SHADOW_PASS
			// nothing to out more than 'gl_Position'
		#else
			out vec4 fragmentPosition;
			out vec4 fragmentNormal;
			out vec4 fragmentUv;
		#endif

		// program
		void main()
		{
			// compute vertex position
			mat4 boneTransform  = (skeletonPose[boneIDs.x] * inverseBindPose[boneIDs.x]) * weights.x;
			if (weights.y > 0)
			{
				boneTransform += (skeletonPose[boneIDs.y] * inverseBindPose[boneIDs.y]) * weights.y;					
				if (weights.z > 0)
				{
					boneTransform += (skeletonPose[boneIDs.z] * inverseBindPose[boneIDs.z]) * weights.z;
					if (weights.w > 0)
						boneTransform += (skeletonPose[boneIDs.w] * inverseBindPose[boneIDs.w]) * weights.w;	
				}
			}

			vec4 transformPosition = boneTransform * position;
			vec4 transformNormal = boneTransform * normal;
			
			// end
			mat4 model = matrixArray[2 * gl_InstanceID];
			mat4 normalMatrix = matrixArray[2 * gl_InstanceID + 1];

			#ifdef SHADOW_PASS
				gl_Position = model * transformPosition;
			#else
				fragmentPosition = model * transformPosition;
				gl_Position = projection * view * fragmentPosition;
				fragmentNormal = normalize(normalMatrix * transformNormal);
				fragmentUv = uv;
			#endif
		}
	};
	geometry : 
	{
		#pragma SHADOW_PASS
		
		#include "UniformBuffers.cginc"
		
			#ifndef GEOMETRY_INVOCATION
				#define GEOMETRY_INVOCATION 4
			#endif
		
			layout(triangles, invocations = GEOMETRY_INVOCATION) in;
			layout(triangle_strip, max_vertices = 3) out;
			
			uniform int shadowOmniLayerUniform;
			
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
						gl_Position = shadowCascadeProjections[ gl_InvocationID ] * gl_in[i].gl_Position;
						gl_Layer = gl_InvocationID;
						EmitVertex();
					}
					EndPrimitive();				
				}
			}
	};
	fragment : 
	{
	#ifdef SHADOW_PASS
		void main() { }
	#else
	
		#include "UniformBuffers.cginc"
		
		// textures
		uniform sampler2D albedo;   //sampler unit 0
		uniform sampler2D emmisive; //sampler unit 1
		uniform sampler2D metalic;  //sampler unit 2
		
		uniform sampler2DArrayShadow  cascadedShadow;
		uniform samplerCubeArray omniShadowArray;
		
		// images
		layout(rgba32ui) readonly uniform uimage3D lightClusters;	// image unit 0
		
		// input
		in vec4 fragmentPosition;
		in vec4 fragmentNormal;
		in vec4 fragmentUv;

		// output
		layout (location = 0) out vec4 fragColor;
		
		// usefull
		vec4 fragmentColor= vec4(0.0);
		vec4 clusterColor = vec4(0.0);
		vec4 cascadeIndexAll = vec4(0,1,2,3);
		vec4 cascadeColor = vec4(0.0);


		// program
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
			vec4 diffuse = clamp(dot(normalize(fragmentNormal), normalize(-lightRay)), 0 , 1 ) * lightColor;
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

		// program
		void main()
		{
			// compute base color depending on environement light (directional)
			vec4 albedoColor = texture(albedo, vec2(fragmentUv.x, fragmentUv.y));
			vec4 emmisiveColor = texture(emmisive, fragmentUv.xy);
			float ndotl = dot(normalize(fragmentNormal), normalize(-m_directionalLightDirection));
			vec4 diffuse = clamp(ndotl, 0 , 1 ) * m_directionalLightColor;
			vec4 metalicParam = texture(metalic, vec2(fragmentUv.x, fragmentUv.y));
			
			float shadowAttenuation = 1.0;
			if ((shadingConfiguration & 0x04) != 0 && ndotl > 0.0)
			{
				int cascadeIndex = ComputeCascadeIndex();
				shadowAttenuation = GetShadowAttenuation(cascadeIndex, max(0.005 * (1.0 - ndotl), 0.0005));			
			}
			
			vec4 viewDir = normalize(cameraPosition - fragmentPosition);
			vec4 reflectDir = reflect(normalize(m_directionalLightDirection), fragmentNormal);  
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
			vec4 specular = metalicParam.x * spec * m_directionalLightColor;  
			fragmentColor = (m_ambientColor + shadowAttenuation * (diffuse + specular)) * albedoColor + emmisiveColor;
			
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
		}
	#endif
	};
}