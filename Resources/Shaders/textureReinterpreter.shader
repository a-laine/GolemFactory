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
		layout(binding = 3) uniform samplerCube cubemap;
		
		layout(rgba16ui) readonly uniform uimage2D terrainData;			// image unit 0
		
		uniform float layer;
		uniform float type;
		
		// input
		in vec2 fragmentUv;
		
		// output
		layout (location = 0) out vec4 fragColor;
		
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
		VertexData GetVertexData(ivec2 vertexCoord)
		{
			uvec4 data = imageLoad(terrainData, vertexCoord);
			
			VertexData vdata;
			vdata.height = data.x / 65536.0;
			vdata.water = data.y / 65536.0;
			vdata.normalTerrain = octahedralUnpack(data.z, 7);
			vdata.normalWater = octahedralUnpack(data.w, 5);
			vdata.material = (data.w >> 10) & 0xFF;
			vdata.holeTerrain = (data.z & (1 << 14)) != 0;
			vdata.holeWater = (data.z & (1 << 15)) != 0;
			return vdata;
		}
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
				VertexData data = GetVertexData(uv);
				
				if (layer == 0)
				{
					float h = data.height;
					fragColor = vec4(h, h, h, 1);
				}
				else if (layer == 1)
				{
					float h = data.water;
					fragColor = vec4(h, h, h, 1);
				}
				else if (layer == 2)
				{
					fragColor = vec4(0.5 * data.normalTerrain.xyz + vec3(0.5), 1);
				}
				else if (layer == 3)
				{
					fragColor = vec4(0.5 * data.normalWater.xyz + vec3(0.5), 1);
				}
				else if (layer == 4)
				{
					float r = 0.25 * (data.material & 0x03);
					float g = 0.25 * ((data.material & 0x0C) >> 2);
					float b = 0.25 * ((data.material & 0x30) >> 4);
					fragColor = vec4(r, g, b, 1);
				}
				else if (layer == 5)
				{
					fragColor = vec4(data.holeTerrain ? 1 : 0, 0, data.holeWater ? 1 : 0, 1);
				}
				else
					fragColor = vec4(0, 0, 0, 1);
			}
			else if (type == 4) // simple texture 2d array
			{
				fragColor = texelFetch(texArray, ivec3(fragmentUv * textureSize(texArray, 0).xy, layer), 0);
				fragColor.w = 1.0;
			}
			else if (type == 5) // cubemap
			{
				vec3 fakeDir;
				switch(int(layer))
				{
					case 0: fakeDir = vec3( 1.0, -fragmentUv.x, fragmentUv.y); break; // +X
					case 1: fakeDir = vec3(-1.0,  fragmentUv.x, fragmentUv.y); break; // -X
					case 2: fakeDir = vec3( fragmentUv.x,  1.0, fragmentUv.y); break; // +Y
					case 3: fakeDir = vec3(-fragmentUv.x, -1.0, fragmentUv.y); break; // -Y
					case 4: fakeDir = vec3(fragmentUv.x, -fragmentUv.y,  1.0); break; // +Z
					default: fakeDir = vec3(fragmentUv.x,  fragmentUv.y, -1.0); break; // -Z
				}
				
				fragColor = texture(cubemap, normalize(fakeDir));
				fragColor.w = 1.0;
			}
		}
	};
}