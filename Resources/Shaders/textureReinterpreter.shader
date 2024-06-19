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
		layout(binding = 0) uniform sampler2DArray texArray;			//sampler unit 0
		layout(binding = 1) uniform samplerCubeArray omniShadowArray;	//sampler unit 1
		layout(binding = 2) uniform sampler2D depthTexture;				//sampler unit 2
		
		layout(rgba16ui) readonly uniform uimage2D terrainData;			// image unit 0
		
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
				float depth = texelFetch(texArray, ivec3(fragmentUv * textureSize(texArray, 0).xy, layer), 0).r;
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
			else if (type == 2) // occlusion 
			{
				float f = texture(depthTexture, vec2(fragmentUv.x, 1.0 - fragmentUv.y)).r;
				
				float depth = 2 * (1.0 - f);
				fragColor = vec4(depth, depth, depth, 1.0);
			}
			else if (type == 3) // terrain heightmap
			{
				ivec2 terrainDataSize = imageSize(terrainData);
				ivec2 uv = ivec2(fragmentUv.x * terrainDataSize.x, (1.0 - fragmentUv.y) * terrainDataSize.y);
				uvec4 data = imageLoad(terrainData, uv);
				
				uint height = data.r;
				uint water = data.g;
				uint normalx = (data.b & 0x7FF);
				uint normalz = (data.b >> 11) | ((data.a & 0x3F) << 5);
				uint material = (data.a >> 6) & 0xFF;
				uint hole = (data.a >> 14) & 0x03;
				
				if (layer == 0)
				{
					float h = height / 65536.0;
					fragColor = vec4(h, h, h, 1);
				}
				else if (layer == 1)
				{
					float h = water / 6553.60;
					fragColor = vec4(h, h, h, 1);
				}
				else if (layer == 2)
				{
					float x = (normalx / 1024.0 - 1.0);
					float z = (normalz / 1024.0 - 1.0);
					float y = sqrt(1.0 - min(x * x + z * z, 1.0));
					fragColor = vec4(x, y, z, 1);
				}
				else if (layer == 3)
				{
					float r = 0.125 * (material & 0x03);
					float g = 0.125 * ((material & 0x38) >> 3);
					float b = 0.25 * ((material & 0xC0) >> 6);
					fragColor = vec4(r, g, b, 1);
				}
				else if (layer == 4)
				{
					float h = 0.33 * hole;
					fragColor = vec4(h, h, h, 1);
				}
				else
					fragColor = vec4(0, 0, 0, 1);
			}
			else if (type == 4) // simple texture 2d array
			{
				fragColor = texelFetch(texArray, ivec3(fragmentUv * textureSize(texArray, 0).xy, layer), 0);
				fragColor.w = 1.0;
			}
		}
	};
}