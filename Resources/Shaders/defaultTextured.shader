DefaultTextured
{	
	renderQueue : 1000;//opaque
	
	uniform :
	{
		matrixArray : "struct array32"
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
		
		layout(std140, binding = 0) uniform GlobalMatrices
		{
			mat4 view;
			mat4 projection;
			vec4 cameraPosition;
		};
		layout(std140, binding = 1) uniform EnvironementLighting
		{
			vec4 m_backgroundColor;
			vec4 m_ambientColor;
			vec4 m_directionalLightDirection;
			vec4 m_directionalLightColor;
		};
		
		struct Light
		{
			vec4 m_position;
			vec4 m_direction;
			vec4 m_color;
			float m_range;
			float m_intensity;
			float m_inCutOff;
			float m_outCutOff;
		};
		layout(std140, binding = 2) uniform Lights
		{
		    int lightCount;
			int pading0;
			float clusterDepthScale;
			float clusterDepthBias;
			float near;
			float far;
			float tanFovX;
			float tanFovY;
			Light lights[128];
		};
	};
	vertex :
	{
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
				fragmentNormal_gs = normalize(normalMatrix * normal);
				fragmentUv_gs = uv;
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
		//	uniform
		uniform sampler2D albedo;   //texture unit 0
		uniform sampler2D emmisive; //texture unit 1
		uniform sampler2D metalic;  //texture unit 2
		
		
		
		//uniform usampler3D lightClusters;
		layout(rgba32ui, binding = 3) readonly uniform uimage3D lightClusters;
		
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
			
			// show light count
			#if 1
			float sum = 0;
				uvec4 clusterData = imageLoad(lightClusters, clusterIndex);
				for (int i = 0; i < 4; i++)
					for (int j = 0; j < 32; j++)
					{
						if ((clusterData[i] & (1 << j)) != 0)
						{
							clusterColor[i] += 0.1;
							sum += 0.1;
						}
					}
				//clusterColor = vec4(sum);
			
			// raw data from cluster
			#elif 0 
				uvec4 clusterData = imageLoad(lightClusters, clusterIndex);
				clusterColor.x = float(clusterData.x) / clusterSize.x;
				//clusterColor.y = float(clusterData.y)/clusterSize.y;
				//clusterColor.z = float(clusterData.z)/clusterSize.z;
				
			// show clusters
			#elif 0 
				clusterColor.x = mix(0.0 , 1.0 , clusterIndex.x / (clusterSize.x - 1.0));
				clusterColor.y = mix(0.0 , 1.0 , clusterIndex.y / (clusterSize.y - 1.0));
				clusterColor.z = mix(0.0 , 1.0 , clusterIndex.z / (clusterSize.z - 1.0));
			
			// show depth layer
			#elif 0 
				float layerPerColor = 0.25 * clusterSize.z;
				if (clusterIndex.z < layerPerColor)
					clusterColor = mix(vec4(0), vec4(1,0,0,1), clusterIndex.z / layerPerColor);
				else if (clusterIndex.z < 2 * layerPerColor)
					clusterColor = mix(vec4(1,0,0,1), vec4(0,1,0,1), (clusterIndex.z - layerPerColor) / layerPerColor);
				else if (clusterIndex.z < 3 * layerPerColor)
					clusterColor = mix(vec4(0,1,0,1), vec4(0,0,1,1), (clusterIndex.z - 2 * layerPerColor) / layerPerColor);
				else
					clusterColor = mix(vec4(0,0,1,1), vec4(1,1,1,1), (clusterIndex.z - 3 * layerPerColor) / layerPerColor);
					
			// show unic clusters
			#elif 0 
				int hash = clusterIndex.x + clusterIndex.y;
				bool odd = (hash & 0x1) != 0;
				bool oddz = (clusterIndex.z & 0x1) != 0;
				clusterColor.x = (odd ? 0.0 : 1.0);
				clusterColor.y = oddz ?(odd ? 1.0 : 0.0) : (odd ? 0.0 : 1.0);
				clusterColor.z = (!oddz && odd) ? 1.0 : 0.0;
			#endif
			
			return clusterIndex;
			
		}
		void ProcessLight(int lightIndex, vec4 albedo, float metalic, vec4 viewDir)
		{
			//if (lightIndex >= lightCount) return; // useless if clustering is well done
		
			// point light computation
			vec4 lightRay = fragmentPosition - lights[lightIndex].m_position;
			float d = length(lightRay);
			float u = d / lights[lightIndex].m_range;
			if (u > 1.0) return; // not that usefull if clustering is well done
			
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
				float edgeMultiplier = 0.4; // lower for thiner edges;
				vec3 a3 = smoothstep(vec3(0.0), fwidth(barycentricCoord) * edgeMultiplier , barycentricCoord);
				float edgeFactor = min(min(a3.x, a3.y), a3.z);
				if(edgeFactor >= 1.0)
					discard;
			#endif
			
			// compute base color depending on environement light (directional)
			vec4 albedoColor = texture(albedo, vec2(fragmentUv.x, fragmentUv.y));
			vec4 emmisiveColor = texture(emmisive, fragmentUv.xy);
			vec4 diffuse = clamp(dot(normalize(fragmentNormal), normalize(-m_directionalLightDirection)), 0 , 1 ) * m_directionalLightColor;
			vec4 metalicParam = texture(metalic, vec2(fragmentUv.x, fragmentUv.y));
			
			vec4 viewDir = normalize(cameraPosition - fragmentPosition);
			vec4 reflectDir = reflect(normalize(m_directionalLightDirection), fragmentNormal);  
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
			vec4 specular = metalicParam.x * spec * m_directionalLightColor;  
			fragmentColor = (diffuse + m_ambientColor + specular) * albedoColor + emmisiveColor;
			
			// process cluster data
			if (lightCount > 0)
			{
				ivec3 clusterIndex = ComputeClusterIndex();
				uvec4 clusterData = imageLoad(lightClusters, clusterIndex);
				for (int i = 0; i < 4; i++)
				{
					if (clusterData[i] == 0)
						continue;
				
					for (int j = 0; j < 32; j++)
					{
						if ((clusterData[i] & (1 << j)) != 0)
							ProcessLight(32 * i + j, albedoColor, metalicParam.x, viewDir);
					}
				}
			}
			
			//fragColor = 0.5 * fragmentColor + 0.5 * clusterColor;
			fragColor = fragmentColor;
		}
	};
} 