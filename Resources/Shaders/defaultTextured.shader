DefaultTextured
{	
	renderQueue : 1000;//opaque
	
	uniform :
	{
		model : "mat4";
		normalMatrix : "mat4";
		models : "mat4 array32";
		normalMatrices : "mat4 array32";
		lightCount : "int"
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
		#version 420
		
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
			uniform mat4 models[MAX_INSTANCE];
			uniform mat4 normalMatrices[MAX_INSTANCE];	
		#else
			uniform mat4 model; 	// model matrix (has to be present at this location)
			uniform mat4 normalMatrix;		
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
			#ifdef INSTANCING
				mat4 model = models[gl_InstanceID];
				mat4 normalMatrix = normalMatrices[gl_InstanceID];
			#endif
		
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
		
		uniform int lightCount;
		
		// input
		in vec4 fragmentPosition;
		in vec4 fragmentNormal;
		in vec4 fragmentUv;
			
		#ifdef WIRED_MODE
			in vec3 barycentricCoord;
		#endif

		// output
		layout (location = 0) out vec4 fragColor;
		
		float map(float value, float min1, float max1, float min2, float max2) {
		  return min2 + (value - min1) * (max2 - min2) / max(max1 - min1, 0.0001);
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
			
			vec4 fragmentColor = (diffuse + m_ambientColor + specular) * albedoColor + emmisiveColor;
			
			// process other lights
			if (lightCount > 0)
			{
				for (int i = 0; i < lightCount; i++)
				{
					// point light computation
					vec4 lightColor = lights[i].m_color;
					vec4 lightRay = fragmentPosition - lights[i].m_position;
					float d = length(lightRay);
					lightRay /= d;
					diffuse = clamp(dot(normalize(fragmentNormal), normalize(-lightRay)), 0 , 1 ) * lights[i].m_color;
					spec = pow(max(dot(viewDir, lightRay), 0.0), 32);
					specular = metalicParam.x * spec * lights[i].m_color;
					
					float u = d / lights[i].m_range;
					float attenuation = lights[i].m_intensity / (1.0 + 25 * u * u)* clamp((1 - u) * 5.0 , 0 , 1);
					
					// spot attenuation
					float isSpotMultiplier = 1 - length(lights[i].m_direction);
					float cutoffRange = lights[i].m_outCutOff - lights[i].m_inCutOff;
					float cutoffClamped = clamp(dot(lightRay, lights[i].m_direction), lights[i].m_outCutOff, lights[i].m_inCutOff);
					float cutoff01 = map(cutoffClamped, lights[i].m_outCutOff, lights[i].m_inCutOff, 0 , 1);
					float spotAttenuation = max(isSpotMultiplier, cutoff01);
					
					// additive blending
					fragmentColor += (spotAttenuation * attenuation) * ((diffuse + specular) * albedoColor);
				}
			}
			
			
			fragColor = fragmentColor;
		}
	};
} 