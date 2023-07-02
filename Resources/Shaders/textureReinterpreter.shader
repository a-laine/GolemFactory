TextureReinterpreter
{
	uniform :
	{
		layer : "float";
		type : "int";
	};
	
	vertex :
	{
		#version 430
		out vec2 fragmentUv;
		
		void main()
		{
			fragmentUv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
			gl_Position = vec4(fragmentUv * vec2(2 , -2) + vec2(-1 , 1), 0 , 1);
		}
	};
	fragment :
	{
		#version 430
		
		// textures & uniforms
		layout(binding = 0) uniform sampler2DArray shadowArray;
		layout(binding = 1) uniform samplerCubeArray omniShadowArray;
		layout(binding = 2) uniform sampler2D depthTexture;
		uniform float layer;
		uniform float type;
		
		// input
		in vec2 fragmentUv;
		
		// output
		layout (location = 0) out vec4 fragColor;
		
		void main()
		{
			if (type == 0) // shadow cascade
			{
				float depth = texelFetch(shadowArray, ivec3(fragmentUv * textureSize(shadowArray, 0).xy, layer), 0).r;
				fragColor = vec4(depth, depth, depth, 1.0);
			}
			else if (type == 1) // omni shadow array
			{
				if (fragmentUv.y < 0.25)
					fragColor = vec4(0, 0, 0, 1);
				else if (fragmentUv.y < 0.5)
				{
					if (fragmentUv.x > 0.25 && fragmentUv.x < 0.5)
					{
						vec2 uv = 8 * vec2(fragmentUv.x - 0.375 , fragmentUv.y - 0.375);
						vec4 direction = vec4(uv.x, -1, uv.y, layer);
						float depth = texture(omniShadowArray, direction).r;
						fragColor = vec4(depth, depth, depth, 1.0);
					}
					else
						fragColor = vec4(0, 0, 0, 1);
				}
				else if (fragmentUv.y > 0.75)
				{
					if (fragmentUv.x > 0.25 && fragmentUv.x < 0.5)
					{
						vec2 uv = 8 * vec2(fragmentUv.x - 0.375 , fragmentUv.y - 0.875);
						vec4 direction = vec4(uv.x, 1, -uv.y, layer);
						float depth = texture(omniShadowArray, direction).r;
						fragColor = vec4(depth, depth, depth, 1.0);
					}
					else
						fragColor = vec4(0, 0, 0, 1);
				}
				else
				{
					vec4 direction;
					if (fragmentUv.x < 0.25)
					{
						vec2 uv = 8 * vec2(fragmentUv.x - 0.125 , fragmentUv.y - 0.625);
						direction = vec4(-1, uv.y, uv.x, layer);
					}
					else if (fragmentUv.x < 0.5)
					{
						vec2 uv = 8 * vec2(fragmentUv.x - 0.375 , fragmentUv.y - 0.625);
						direction = vec4(uv.x, uv.y, 1, layer);
					}
					else if (fragmentUv.x < 0.75)
					{
						vec2 uv = 8 * vec2(fragmentUv.x - 0.625 , fragmentUv.y - 0.625);
						direction = vec4(1, uv.y, -uv.x, layer);
					}
					else 
					{
						vec2 uv = 8 * vec2(fragmentUv.x - 0.875 , fragmentUv.y - 0.625);
						direction = vec4(-uv.x, uv.y, -1, layer);
					}
					
					float depth = texture(omniShadowArray, direction).r;
					fragColor = vec4(depth, depth, depth, 1.0);
				}
			}
			else if (type == 2)
			{
				float f = texture(depthTexture, vec2(fragmentUv.x, 1.0 - fragmentUv.y)).r;
				
				float depth = 2 * (1.0 - f);
				fragColor = vec4(depth, depth, depth, 1.0);
			}
		}
	};
}