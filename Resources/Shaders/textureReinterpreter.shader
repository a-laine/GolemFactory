TextureReinterpreter
{
	uniform :
	{
		layer : "float";
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
		layout(binding = 0) uniform sampler2DArray shadowArray;	// 0
		uniform float layer;
		
		// input
		in vec2 fragmentUv;
		
		// output
		layout (location = 0) out vec4 fragColor;
		
		void main()
		{
			float depth = texelFetch(shadowArray, ivec3(fragmentUv * textureSize(shadowArray, 0).xy, layer), 0).r;
			fragColor = vec4(depth, depth, depth, 1.0);
		}
	};
}