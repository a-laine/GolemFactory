DefaultTextured
{	
	renderQueue : 1000;//opaque
	
	uniform :
	{
		model : "mat4";
		normalMatrix : "mat4";
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

		uniform mat4 model; 	// model matrix (has to be present at this location)
		uniform mat4 normalMatrix;

		// output
		out vec4 fragmentPosition;
		out vec4 fragmentNormal;
		out vec4 fragmentUv;

		// program
		void main()
		{
			fragmentPosition = model * position;
			gl_Position = projection * view * fragmentPosition;
			fragmentNormal = normalize(normalMatrix * normal);
			fragmentUv = uv;
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

		// output
		layout (location = 0) out vec4 fragColor;

		// program
		void main()
		{
			vec4 albedoColor = texture(albedo, vec2(fragmentUv.x, fragmentUv.y));
			vec4 emmisiveColor = texture(emmisive, fragmentUv.xy);
			vec4 diffuse = clamp(dot(normalize(fragmentNormal), normalize(-m_directionalLightDirection)), 0 , 1 ) * m_directionalLightColor;
			vec4 metalicParam = texture(metalic, vec2(fragmentUv.x, fragmentUv.y));
			
			vec4 viewDir = normalize(cameraPosition - fragmentPosition);
			vec4 reflectDir = reflect(normalize(m_directionalLightDirection), fragmentNormal);  
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
			vec4 specular = metalicParam.x * spec * m_directionalLightColor;  
			
			fragColor = (diffuse + m_ambientColor + specular) * albedoColor + emmisiveColor;
			
			if (lightCount > 0)
			{
				for (int i = 0; i < lightCount; i++)
				{
					vec4 lightColor = lights[i].m_color;
					vec4 lightRay = fragmentPosition - lights[i].m_position;
					float d = length(lightRay);
					lightRay /= d;
					diffuse = clamp(dot(normalize(fragmentNormal), normalize(-lightRay)), 0 , 1 ) * lights[i].m_color;
					spec = pow(max(dot(viewDir, lightRay), 0.0), 32);
					specular = metalicParam.x * spec * lights[i].m_color;
					
					float u = d / lights[i].m_range;
					//float attenuation = lights[i].m_intensity * max(0.0 , 1.0 - pow(d / lights[i].m_range, 0.5));//intensity / (1.0 + lights[i].m_linear * d + lights[i].m_quadratic * (d * d));
					float attenuation = lights[i].m_intensity / (1.0 + 25 * u * u)* clamp((1 - u) * 5.0 , 0 , 1);
					fragColor += (diffuse + specular) * attenuation * albedoColor;
				}
			}
			
		}
	};
} 