Default
{	
	renderQueue : 1000;//opaque
	
	uniform :
	{
		matrixArray : "struct array32";
		overrideColor : "vec4";
		lightClusters : "_globalLightClusters";
		cascadedShadow : "_globalShadowCascades";
		omniShadowArray : "_globalOmniShadow";
	};
	
	includes : 
	{
		#version 430
	};
	vertex : 
	{
		#include "UniformBuffers.cginc"
		
		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in vec4 uv;
		
		#ifdef INSTANCING
			#define MAX_INSTANCE 32
			#define MAX_MATRIX (2 * MAX_INSTANCE)
			uniform mat4 matrixArray[MAX_MATRIX];
		#else
			uniform mat4 matrixArray[2];
		#endif
		

		// output
		#ifdef WIRED_MODE
			out vec4 fragmentPosition_gs;
			out vec4 fragmentNormal_gs;
			out vec4 fragmentUv_gs;
		#else
			out vec4 fragmentPosition;
			out vec4 fragmentNormal;
			out vec4 fragmentUv;
		#endif
		
		// program
		void main()
		{
			mat4 model = matrixArray[2 * gl_InstanceID];
			mat4 normalMatrix = matrixArray[2 * gl_InstanceID + 1];
		
			#ifdef WIRED_MODE
				fragmentPosition_gs = model * position;
				gl_Position = projection * view * fragmentPosition_gs;
				fragmentUv_gs = uv;
				fragmentNormal_gs = normalize(normalMatrix * normal);
			#else
				fragmentPosition = model * position;
				gl_Position = projection * view * fragmentPosition;
				fragmentNormal = normalize(normalMatrix * normal);
				fragmentUv = uv;
				
			#endif
		}
	};
	geometry : 
	{
		#include "UniformBuffers.cginc"
		
		#pragma WIRED_MODE
		
		layout(triangles) in;
		layout(triangle_strip, max_vertices = 3) out;

		// input
		in vec4 fragmentPosition_gs[];
		in vec4 fragmentNormal_gs[];
		in vec4 fragmentUv_gs[];

		// output
		out vec4 fragmentNormal;
		out vec4 fragmentPosition;
		out vec4 fragmentUv;
		out vec3 barycentricCoord;

		void main()
		{
			gl_Position = gl_in[0].gl_Position;
			fragmentPosition = fragmentPosition_gs[0];
			fragmentNormal = fragmentNormal_gs[0];
			fragmentUv = fragmentUv_gs[0];
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = gl_in[1].gl_Position;
			fragmentPosition = fragmentPosition_gs[1];
			fragmentNormal = fragmentNormal_gs[1];
			fragmentUv = fragmentUv_gs[1];
			barycentricCoord = vec3(0.0 , 1.0 , 0.0);
			EmitVertex();
			
			gl_Position = gl_in[2].gl_Position;
			fragmentPosition = fragmentPosition_gs[2];
			fragmentNormal = fragmentNormal_gs[2];
			fragmentUv = fragmentUv_gs[2];
			barycentricCoord = vec3(0.0 , 0.0 , 1.0);
			EmitVertex();
			
			EndPrimitive();
		}
	};
	fragment : 
	{
		#include "UniformBuffers.cginc"
		
		// images and uniforms
		layout(rgba32ui, binding = 3) readonly uniform uimage3D lightClusters;
		uniform vec4 overrideColor = vec4(-1.0 , 0.0 , 0.0 , 0.0);
		
		// input
		in vec4 fragmentPosition;
		in vec4 fragmentNormal;
		in vec4 fragmentUv;
			
		#ifdef WIRED_MODE
			in vec3 barycentricCoord;
		#endif

		// output
		layout (location = 0) out vec4 fragColor;
		
		// usefull
		vec4 fragmentColor= vec4(0.0);
		vec4 clusterColor = vec4(0.0);

		float map(float value, float min1, float max1, float min2, float max2)
		{
		  return min2 + (value - min1) * (max2 - min2) / max(max1 - min1, 0.0001);
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
		void ProcessLight(int lightIndex, vec4 albedo, float metalic, vec4 viewDir)
		{
			//if (lightIndex >= lightCount) return; // useless if clustering is well done
		
			// point light computation
			vec4 lightRay = fragmentPosition - lights[lightIndex].m_position;
			float d = length(lightRay);
			float u = d / lights[lightIndex].m_range;
			//if (u > 1.0) return; // not that usefull if clustering is well done
			
			lightRay /= d;
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
			fragmentColor += (spotAttenuation * attenuation) * ((diffuse + specular) * albedo);
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
			vec4 albedoColor = vec4(0.9 , 0.9 , 0.9 , 1.0);
			float diffuseDot = clamp(dot(normalize(fragmentNormal), normalize(-m_directionalLightDirection)), 0 , 1 );
			vec4 diffuse = diffuseDot * m_directionalLightColor;
			vec4 metalicParam = vec4(0.0 , 0.0 , 0.0 , 0.0);
			
			vec4 viewDir = normalize(cameraPosition - fragmentPosition);
			vec4 reflectDir = reflect(normalize(m_directionalLightDirection), fragmentNormal);  
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
			vec4 specular = metalicParam.x * spec * m_directionalLightColor;  
			fragmentColor = (diffuse + m_ambientColor + specular) * albedoColor;
			
			// process cluster data
			if (lightCount > 0 && overrideColor.x < 0.0)
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
			else if (overrideColor.x >= 0.0)
			{
				fragmentColor = (0.5 * diffuseDot + 0.3) * overrideColor;
				fragmentColor.w = 1.0;
			}
			
			if ((shadingConfiguration & 0x02) != 0)
				fragColor = 0.5 * fragmentColor + 0.5 * clusterColor;
			else
				fragColor = fragmentColor;
		}
	};
} 
